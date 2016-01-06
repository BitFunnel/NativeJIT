#pragma once

#include <cstdint>

#include "NativeJIT/CodeGen/X64CodeGenerator.h" // Emit<OP> referenced by template definition.
#include "NativeJIT/ExpressionTree.h"           // ExpressionTree::Storage<T> parameter.
#include "Temporary/Assert.h"                   // Assert() referenced by template definition.


namespace NativeJIT
{
    namespace CodeGenHelpers
    {
        // Enums specifying whether the Emitter's helper method is operating
        // on register and storage whose register types are of the same size
        // and type or not and whether the immediate storage is allowed for input.
        enum class RegTypes { ExactlySame, Different };
        enum class ImmediateType { Allowed, NotAllowed };

        // A helper class for the Emit() method to be specialized for different
        // types of inputs.
        template <RegTypes REGTYPES, ImmediateType IMMEDIATETYPE>
        struct Emitter
        {
            // Emit from storage to register.
            template <OpCode OP, typename DESTREGTYPE, typename SRC>
            static void Emit(X64CodeGenerator& code,
                             DESTREGTYPE dest,
                             const ExpressionTree::Storage<SRC>& src);

            // Emit from register to non-immediate storage.
            template <OpCode OP, typename DEST, typename SRCREGTYPE>
            static void Emit(X64CodeGenerator& code,
                             const ExpressionTree::Storage<DEST>& dest,
                             SRCREGTYPE src);
        };


        //
        // Classes and methods that take Storage as the source.
        //

        // The Emit() method is used to Emit an opcode with the specified target
        // register and source storage. Depending on the type of the storage,
        // the correct flavor of X64CodeGenerator's Emit() method will be called
        // (i.e. registers of same or mixed type/size, immediate flavor allowed
        // or not). If the immediate flavor of the storage is not allowed and
        // the client passes a storage of an immediate type, an assert is triggered.
        template <OpCode OP, typename DESTREGTYPE, typename SRC>
        void Emit(X64CodeGenerator& code, DESTREGTYPE dest, const ExpressionTree::Storage<SRC>& src)
        {
            typedef ExpressionTree::Storage<SRC>::DirectRegister SrcRegType;

            // Pick the right flavor of the helper class specialization and call it.
            const RegTypes regTypes = std::is_same<SrcRegType, DESTREGTYPE>::value
                                      ? RegTypes::ExactlySame
                                      : RegTypes::Different;
            const ImmediateType immediateType = CanBeInImmediateStorage<SRC>::value
                                                ? ImmediateType::Allowed
                                                : ImmediateType::NotAllowed;

            Emitter<regTypes, immediateType>::Emit<OP>(code, dest, src);
        }


        // Storage and destination with registers with the same type and size,
        // with support for immediates.
        template <>
        template <OpCode OP, typename DESTREGTYPE, typename SRC>
        void Emitter<RegTypes::ExactlySame, ImmediateType::Allowed>::Emit(
            X64CodeGenerator& code, DESTREGTYPE dest,
            const ExpressionTree::Storage<SRC>& src)
        {
            switch (src.GetStorageClass())
            {
            case StorageClass::Immediate:
                code.EmitImmediate<OP>(dest, src.GetImmediate());
                break;
            case StorageClass::Direct:
                code.Emit<OP>(dest, src.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP>(dest, src.GetBaseRegister(), src.GetOffset());
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }


        // Storage and destination with registers with the same type and size,
        // with no support for immediates.
        template <>
        template <OpCode OP, typename DESTREGTYPE, typename SRC>
        void Emitter<RegTypes::ExactlySame, ImmediateType::NotAllowed>::Emit(
            X64CodeGenerator& code, DESTREGTYPE dest,
            const ExpressionTree::Storage<SRC>& src)
        {
            switch (src.GetStorageClass())
            {
            case StorageClass::Direct:
                code.Emit<OP>(dest, src.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP>(dest, src.GetBaseRegister(), src.GetOffset());
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }


        // Storage and destination with registers with the mixed type and size,
        // with support for immediates.
        template <>
        template <OpCode OP, typename DESTREGTYPE, typename SRC>
        void Emitter<RegTypes::Different, ImmediateType::Allowed>::Emit(
            X64CodeGenerator& code, DESTREGTYPE dest,
            const ExpressionTree::Storage<SRC>& src)
        {
            typedef ExpressionTree::Storage<SRC>::DirectRegister SrcRegType;

            switch (src.GetStorageClass())
            {
            case StorageClass::Immediate:
                code.EmitImmediate<OP>(dest, src.GetImmediate());
                break;
            case StorageClass::Direct:
                code.Emit<OP>(dest, src.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP,
                          DESTREGTYPE::c_size, DESTREGTYPE::c_isFloat,
                          SrcRegType::c_size, SrcRegType::c_isFloat>(
                            dest, src.GetBaseRegister(), src.GetOffset());
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }


        // Storage and destination with registers with the mixed type and size,
        // with no support for immediates.
        template <>
        template <OpCode OP, typename DESTREGTYPE, typename SRC>
        void Emitter<RegTypes::Different, ImmediateType::NotAllowed>::Emit(
            X64CodeGenerator& code, DESTREGTYPE dest,
            const ExpressionTree::Storage<SRC>& src)
        {
            typedef ExpressionTree::Storage<SRC>::DirectRegister SrcRegType;

            switch (src.GetStorageClass())
            {
            case StorageClass::Direct:
                code.Emit<OP>(dest, src.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP,
                          DESTREGTYPE::c_size, DESTREGTYPE::c_isFloat,
                          SrcRegType::c_size, SrcRegType::c_isFloat>(
                            dest, src.GetBaseRegister(), src.GetOffset());
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }


        // Places the constant float value into the target floating point
        // register. Allocates and releases a temporary integer register in
        // the process. This should be used only by the code which cannot
        // use the ImmediateNode because it doesn't know the floating point
        // value until the code generation time.
        template <unsigned SIZE, typename T>
        void MovThroughTemporary(ExpressionTree& tree, Register<SIZE, true> dest, T value)
        {
            static_assert(std::is_floating_point<T>::value, "The immediate must be a float.");
            static_assert(sizeof(T) == SIZE, "The size of the immediate must match the register size.");
            typedef std::conditional<SIZE == 4, uint32_t, uint64_t>::type TemporaryType;

            auto & code = tree.GetCodeGenerator();
            auto temp = tree.Direct<TemporaryType>();
            auto r = temp.GetDirectRegister();

            code.EmitImmediate<OpCode::Mov>(r, *(reinterpret_cast<TemporaryType*>(&value)));
            code.Emit<OpCode::Mov>(dest, r);
        }


        //
        // Classes and methods that take Storage as the destination.
        //

        // The Emit() method is used to Emit an opcode with the specified target
        // storage and the source register. Depending on the type of the storage,
        // the correct flavor of X64CodeGenerator's Emit() method will be called
        // (i.e. registers of same or mixed type/size). Immediate storage is
        // never allowed and triggers an assert.
        template <OpCode OP, typename DEST, typename SRCREGTYPE>
        void Emit(X64CodeGenerator& code,
                  const ExpressionTree::Storage<DEST>& dest,
                  SRCREGTYPE src)
        {
            typedef ExpressionTree::Storage<DEST>::DirectRegister DESTREGTYPE;

            // Pick the right flavor of the helper class specialization and call it.
            const RegTypes regTypes = std::is_same<SRCREGTYPE, DESTREGTYPE>::value
                                      ? RegTypes::ExactlySame
                                      : RegTypes::Different;

            Emitter<regTypes, ImmediateType::NotAllowed>::Emit<OP>(code, dest, src);
        }


        // Storage and destination with registers with the same type and size.
        template <>
        template <OpCode OP, typename DEST, typename SRCREGTYPE>
        void Emitter<RegTypes::ExactlySame, ImmediateType::NotAllowed>::Emit(
            X64CodeGenerator& code,
            const ExpressionTree::Storage<DEST>& dest,
            SRCREGTYPE src)
        {
            switch (dest.GetStorageClass())
            {
            case StorageClass::Direct:
                code.Emit<OP>(dest.GetDirectRegister(), src);
                break;
            case StorageClass::Indirect:
                code.Emit<OP>(dest.GetBaseRegister(), dest.GetOffset(), src);
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }


        // Storage and destination with registers with the mixed type and size.
        template <>
        template <OpCode OP, typename DEST, typename SRCREGTYPE>
        void Emitter<RegTypes::Different, ImmediateType::NotAllowed>::Emit(
            X64CodeGenerator& code,
            const ExpressionTree::Storage<DEST>& dest,
            SRCREGTYPE src)
        {
            typedef ExpressionTree::Storage<DEST>::DirectRegister DESTREGTYPE;

            switch (dest.GetStorageClass())
            {
            case StorageClass::Direct:
                code.Emit<OP>(dest.GetDirectRegister(), src);
                break;
            case StorageClass::Indirect:
                code.Emit<OP,
                          DESTREGTYPE::c_size, DESTREGTYPE::c_isFloat,
                          SRCREGTYPE::c_size, SRCREGTYPE::c_isFloat>(
                            dest.GetBaseRegister(), dest.GetOffset(), src);
                break;
            default:
                Assert(false, "Invalid storage class.");
            }
        }
    }
}
