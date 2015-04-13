#pragma once

#include <algorithm>    // For std::max
#include <type_traits>

#include "ExpressionNodeFactory.h"
#include "Node.h"


namespace NativeJIT
{
    class ExpressionNodeFactory;

    // The CastNode implements a cast node from FROM to TO, most closely
    // resembling the C++ static_cast/reinterpret_cast. Requires that FROM
    // is convertible to TO using static_cast or that the sizes of FROM and TO
    // are the same so that reinterpret_cast can semi-safely be used.
    // If register size and type of FROM/TO are the same, the cast is a no-op.
    // Otherwise, the appropriate instructions are emitted to perform the
    // conversion.

    namespace Casting
    {
        // All the possible types of casts. Note that integer applies to register
        // type, i.e. everything that is not a floating point.
        enum class Cast { NoOp, Int, Float, IntToFloat, FloatToInt };

        // A class used to specialize code generation for casts that can be
        // implemented directly using the X64 conversion instructions.
        template <Cast TYPE>
        struct DirectCastGenerator
        {
            template <typename TO, typename FROM>
            static ExpressionTree::Storage<TO>
            Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source);
        };


        // A class used to build a composite node needed to perform a cast
        // for casts that cannot be implemented directly using the X64 conversion
        // instructions.
        template <Cast TYPE>
        struct CompositeCastNodeBuilder
        {
            template <typename TO, typename FROM>
            static Node<TO>& Build(ExpressionNodeFactory& tree, Node<FROM>& from);
        };


        // Properties for a cast.
        template <typename TO, typename FROM>
        struct Traits
        {
            // Source and target register types and properties.
            typedef typename ExpressionTree::Storage<FROM>::DirectRegister FromRegister;
            typedef typename ExpressionTree::Storage<TO>::DirectRegister ToRegister;

            static const bool c_isSameRegisterType = std::is_same<FromRegister, ToRegister>::value;
            static const bool c_isSameFloatValue = FromRegister::c_isFloat == ToRegister::c_isFloat;

            // Cast is a no-op if it's from and to the same register size and
            // type (see further for signed/unsigned consideration). Additionally,
            // integer cast from larger to smaller size is a no-op and is achieved
            // by observing the larger size as the smaller size.
            //
            // For integer casts, if the signedness of both sides is same, the
            // expected outcome is clearly achieved by observing just the smaller
            // portion of the register. The same applies for larger signed FROM
            // and smaller unsigned TO since that's how the C++ standard defines
            // such cast when two's complement system is used. Finally, for larger
            // unsigned FROM and smaller signed TO, the behavior is implementation
            // defined and here it's chosen to be bit-preserving, the same as in
            // VC.
            //
            // WARNING: assumption that the int cast from larger to smaller size
            // is a no-op is non-portable for indirect storage since it assumes
            // little endian architecture. If portability is required, the
            // assumption could be removed by converting the indirect storage to
            // direct.
            static const bool c_isNoOp
                = c_isSameRegisterType
                  || (c_isSameFloatValue
                      && !FromRegister::c_isFloat
                      && FromRegister::c_size > ToRegister::c_size);

            // Determine which type of cast should be performed.
            static const Cast c_castType
                = c_isNoOp
                  ? Cast::NoOp
                  : (c_isSameFloatValue
                     ? (FromRegister::c_isFloat ? Cast::Float : Cast::Int)
                     : (FromRegister::c_isFloat ? Cast::FloatToInt : Cast::IntToFloat));

            // Check whether an int to float cast can be done directly.
            static const bool c_isDirectIntToFloatCast
                = c_castType == Cast::IntToFloat
                  && (std::is_same<FROM, __int32>::value
                      || std::is_same<FROM, __int64>::value);

            // Check whether a float to int cast can be done directly.
            static const bool c_isDirectFloatToIntCast
                = c_castType == Cast::FloatToInt
                  && (std::is_same<TO, __int32>::value
                      || std::is_same<TO, __int64>::value);

            static const bool c_isDirectCast
                = c_castType == Cast::NoOp
                  || c_castType == Cast::Int
                  || c_castType == Cast::Float
                  || c_isDirectIntToFloatCast
                  || c_isDirectFloatToIntCast;
        };


        // Cast using a static_cast for convertible types.
        template <typename TO, typename FROM>
        TO ForcedCast(FROM from,
                      typename std::enable_if<std::is_convertible<FROM, TO>::value>::type* = nullptr)
        {
            return static_cast<TO>(from);
        }


        // Cast using a reinterpret_cast for non-convertible types of the same size.
        template <typename TO, typename FROM>
        TO ForcedCast(FROM from,
                      typename std::enable_if<!std::is_convertible<FROM, TO>::value
                                              && sizeof(FROM) == sizeof(TO)>::type* = nullptr)
        {
            return *reinterpret_cast<TO*>(&from);
        }
    }


    // The CastNode is partially specialized depending on whether or not a direct
    // cast can be used.
    template <typename TO,
              typename FROM,
              bool ISDIRECT = Casting::Traits<TO, FROM>::c_isDirectCast>
    class CastNode;


    // Direct cast version of the CastNode.
    template <typename TO, typename FROM>
    class CastNode<TO, FROM, true> : public Node<TO>
    {
    public:
        CastNode(ExpressionTree& tree, Node<FROM>& from);

        //
        // Overrides of Node methods.
        //

        virtual ExpressionTree::Storage<TO> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    protected:
        typedef Casting::Traits<TO, FROM> Traits;

        Node<FROM>& m_from;
    };


    // Composite cast version of the CastNode.
    template <typename TO, typename FROM>
    class CastNode<TO, FROM, false> : public Node<TO>
    {
    public:
        CastNode(ExpressionNodeFactory& tree, Node<FROM>& from);

        //
        // Overrides of Node methods.
        //

        virtual ExpressionTree::Storage<TO> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    protected:
        typedef Casting::Traits<TO, FROM> Traits;

        // The composite node used to perform the cast.
        Node<TO>& m_conversionNode;
    };


    //*************************************************************************
    //
    // Template definitions for direct CastNode.
    //
    //*************************************************************************

    template <typename TO, typename FROM>
    CastNode<TO, FROM, true>::CastNode(ExpressionTree& tree, Node<FROM>& from)
        : Node(tree),
          m_from(from)
    {
        m_from.IncrementParentCount();
    }


    template <typename TO, typename FROM>
    typename ExpressionTree::Storage<TO>
    CastNode<TO, FROM, true>::CodeGenValue(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();
        auto source = m_from.CodeGen(tree);

        using namespace Casting;
        ExpressionTree::Storage<TO> value;

        // Use the compiler to perform the cast if possible.
        if (Traits::c_castType != Cast::NoOp
            && source.GetStorageClass() == StorageClass::Immediate)
        {
            value = tree.Immediate(Casting::ForcedCast<TO, FROM>(source.GetImmediate()));
        }
        else
        {
            value = Casting::DirectCastGenerator<Traits::c_castType>::Generate<TO, FROM>(tree, source);
        }

        return value;
    }


    template <typename TO, typename FROM>
    unsigned CastNode<TO, FROM, true>::LabelSubtree(bool /* isLeftChild */)
    {
        // Need at least one register for storing the result.
        SetRegisterCount((std::max)(m_from.LabelSubtree(true), 1u));

        return GetRegisterCount();
    }


    template <typename TO, typename FROM>
    void CastNode<TO, FROM, true>::Print() const
    {
        std::cout << "CastNode (direct) id = " << GetId()
                  << ", parents = " << GetParentCount()
                  << ", from = " << m_from.GetId()
                  << ", ";

        PrintRegisterAndCacheInfo();
    }


    //*************************************************************************
    //
    // Template definitions for composite CastNode.
    //
    //*************************************************************************

    template <typename TO, typename FROM>
    CastNode<TO, FROM, false>::CastNode(ExpressionNodeFactory& tree, Node<FROM>& from)
        : Node(tree),
          m_conversionNode(Casting::CompositeCastNodeBuilder<Casting::Traits<TO, FROM>::c_castType>
                            ::Build<TO, FROM>(tree, from))
    {
        from.IncrementParentCount();
        m_conversionNode.IncrementParentCount();
    }


    template <typename TO, typename FROM>
    typename ExpressionTree::Storage<TO>
    CastNode<TO, FROM, false>::CodeGenValue(ExpressionTree& tree)
    {
        return m_conversionNode.CodeGen(tree);
    }


    template <typename TO, typename FROM>
    unsigned CastNode<TO, FROM, false>::LabelSubtree(bool isLeftChild)
    {
        SetRegisterCount(m_conversionNode.LabelSubtree(isLeftChild));

        return GetRegisterCount();
    }


    template <typename TO, typename FROM>
    void CastNode<TO, FROM, false>::Print() const
    {
        std::cout << "CastNode (indirect) id = " << GetId()
                  << ", parents = " << GetParentCount()
                  << ", conversionNode = " << m_conversionNode.GetId()
                  << ", ";

        PrintRegisterAndCacheInfo();
    }


    namespace Casting
    {
        //
        // Template definitions for direct DirectCastGenerator.
        //

        // No-op cast.
        template <>
        template <typename TO, typename FROM>
        ExpressionTree::Storage<TO>
        DirectCastGenerator<Cast::NoOp>::Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source)
        {
            return ExpressionTree::Storage<TO>(source);
        }


        // Cast between two float registers.
        template <>
        template <typename TO, typename FROM>
        ExpressionTree::Storage<TO>
        DirectCastGenerator<Cast::Float>::Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source)
        {
            static_assert(Traits<TO, FROM>::c_isDirectCast, "Invalid direct cast.");

            auto target = tree.Direct<TO>();

            CodeGenHelpers::Emit<OpCode::CvtFP2FP>(tree.GetCodeGenerator(),
                                                   target.GetDirectRegister(),
                                                   source);

            return target;
        }


        // Cast between two integer registers.
        template <>
        template <typename TO, typename FROM>
        ExpressionTree::Storage<TO>
        DirectCastGenerator<Cast::Int>::Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source)
        {
            typedef Traits<TO, FROM> Traits;

            static_assert(Traits::c_isDirectCast, "Invalid direct cast.");
            static_assert(Traits::FromRegister::c_size < Traits::ToRegister::c_size,
                          "Invalid direct cast.");

            typedef ExpressionTree::Storage<TO> ToStorage;

            // The cast needs to extend the source storage to a larger target.
            // Depending on the signedness of the source type, use sign extension
            // or zero extension.
            const OpCode opCode = std::is_signed<FROM>::value
                ? OpCode::MovSX
                : OpCode::MovZX;
            auto & code = tree.GetCodeGenerator();
            ToStorage result;

            if (source.GetStorageClass() == StorageClass::Indirect)
            {
                using namespace CodeGenHelpers;

                // Cannot reuse source, allocate a new register and move with
                // extension into it.
                result = tree.Direct<TO>();

                // Target the full register to prevent a partial register stall.
                ToStorage::FullRegister targetRegister(result.GetDirectRegister());
                Emitter<RegTypes::Different, ImmediateType::NotAllowed>
                    ::Emit<opCode>(tree.GetCodeGenerator(),
                                   targetRegister,
                                   source);
            }
            else
            {
                // May be able to reuse the register if it's fully owned by the storage.
                source.ConvertToValue(tree, true);
                result = ToStorage(source);

                // Always target the full register to prevent a partial register stall.
                auto sourceRegister = source.GetDirectRegister();
                ToStorage::FullRegister fullSourceRegister(sourceRegister);

                code.Emit<opCode>(fullSourceRegister, sourceRegister);
            }

            return result;
        }


        // Cast from integer to float register.
        template <>
        template <typename TO, typename FROM>
        ExpressionTree::Storage<TO>
        DirectCastGenerator<Cast::IntToFloat>::Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source)
        {
            static_assert(Traits<TO, FROM>::c_isDirectCast, "Invalid direct cast.");

            auto target = tree.Direct<TO>();

            // TODO: Implement xorps/xorpd and add a "xor target, target" call to
            // clear the target FP register before changing its lower 32/64 bits.
            // Doing that will prevent a partial register stall.
            using namespace CodeGenHelpers;
            Emitter<RegTypes::Different, ImmediateType::NotAllowed>
                ::Emit<OpCode::CvtSI2FP>(tree.GetCodeGenerator(),
                                         target.GetDirectRegister(),
                                         source);

            return target;
        }


        // Cast from float to integer register.
        template <>
        template <typename TO, typename FROM>
        ExpressionTree::Storage<TO>
        DirectCastGenerator<Cast::FloatToInt>::Generate(ExpressionTree& tree, ExpressionTree::Storage<FROM>& source)
        {
            static_assert(Traits<TO, FROM>::c_isDirectCast, "Invalid direct cast.");

            auto target = tree.Direct<TO>();

            CodeGenHelpers::Emit<OpCode::CvtFP2SI>(tree.GetCodeGenerator(),
                                                   target.GetDirectRegister(),
                                                   source);

            return target;
        }


        // A helper method to build two cast nodes which would convert from FROM
        // to TO by using direct casts through an INTERMEDIATE.
        template <typename TO, typename INTERMEDIATE, typename FROM>
        Node<TO>& TwoStepCast(ExpressionNodeFactory& tree, Node<FROM>& from)
        {
            static_assert(Traits<FROM, INTERMEDIATE>::c_isDirectCast, "Must be a direct cast.");
            static_assert(Traits<INTERMEDIATE, TO>::c_isDirectCast, "Must be a direct cast.");

            return tree.Cast<TO, INTERMEDIATE>(tree.Cast<INTERMEDIATE, FROM>(from));
        }


        //
        // Template definitions for direct CompositeCastNodeBuilder.
        //

        // Composite node for casting from integer to float.
        template <>
        template <typename TO, typename FROM>
        Node<TO>&
        CompositeCastNodeBuilder<Cast::IntToFloat>::Build(ExpressionNodeFactory& tree, Node<FROM>& from)
        {
            Node<TO>* result;

            if (sizeof(FROM) <= 2)
            {
                // Two direct casts: [unsigned] __int8/16 -> __int32 -> float.
                result = &TwoStepCast<TO, __int32, FROM>(tree, from);
            }
            else if (sizeof(FROM) == 4)
            {
                // Two direct casts: __int32 -> __int64 -> float.
                result = &TwoStepCast<TO, __int64, FROM>(tree, from);
            }
            else
            {
                // Cast unsigned __int64 to float as if it is a signed __int64
                // and add 2^64 if the signed __int64 was negative to adjust.

                // This test could be done with the group 3 "test" instruction
                // in case it was available.
                auto & isNegative = tree.Compare<JccType::JS>(from, tree.Immediate<FROM>(0));

                auto & floatNode = TwoStepCast<TO, __int64, FROM>(tree, from);
                auto & twoToThePowerOf64 = tree.Immediate<TO>(1.8446744073709551616e19f);

                result = &tree.Conditional(isNegative,
                                           tree.Add(floatNode, twoToThePowerOf64),
                                           floatNode);
            }

            return *result;
        }


        // Cast from float to integer register.
        template <>
        template <typename TO, typename FROM>
        Node<TO>&
        CompositeCastNodeBuilder<Cast::FloatToInt>::Build(ExpressionNodeFactory& tree, Node<FROM>& from)
        {
            Node<TO>* result;

            // Note: In C++, conversion from a floating point value that is
            // outside of the range of the target integer is undefined behavior
            // (the result can be anything and it doesn't even have to be
            // consistent). Any occurence of undefined behavior makes any further
            // operations also undefined. Thus, the caller is expected to make
            // sure that the input floating point values are inside the target
            // integer range. Because of this, the implementation doesn't take
            // any special steps if negative float is converted to unsigned
            // integer or if the float is too large.
            if (sizeof(TO) <= 2)
            {
                // Two direct casts: float -> __int32 -> [unsigned] __int8/16.
                result = &TwoStepCast<TO, __int32, FROM>(tree, from);
            }
            else if (sizeof(TO) == 4)
            {
                // Two direct casts: float -> __int64 -> unsigned __int32.
                result = &TwoStepCast<TO, __int64, FROM>(tree, from);
            }
            else
            {
                // When converting unsigned __int64 to float, there are two
                // branches: if the float is smaller than 2^63 and it will fit
                // into a signed __int64, use essentially a direct cast.
                // Otherwise, subtract 2^63 from the floating point, convert
                // to signed __int64 and add (integer) 2^63 to the result.
                unsigned __int64 twoToThePowerOf63 = 1ull << 63;
                auto & twoToThePowerOf63Float
                    = tree.Immediate(static_cast<FROM>(twoToThePowerOf63));

                // Check whether source is smaller than 2^63.
                auto & isSourceWithoutOverflow
                    = tree.Compare<JccType::JB>(from, twoToThePowerOf63Float);

                // The node with a reduced float and the int which gets increased
                // by the same value after the conversion.
                auto & reducedFloat = tree.Sub(from, twoToThePowerOf63Float);
                auto & adjustedConvertedInt
                    = tree.Add(TwoStepCast<TO, __int64, FROM>(tree, reducedFloat),
                               tree.Immediate(static_cast<TO>(twoToThePowerOf63)));

                result = &tree.Conditional(isSourceWithoutOverflow,
                                           TwoStepCast<TO, __int64, FROM>(tree, from),
                                           adjustedConvertedInt);
            }

            return *result;
        }
    }
}
