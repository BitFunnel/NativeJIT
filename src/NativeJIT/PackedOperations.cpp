#include "stdafx.h"

#include <iostream>                 // TODO: Removed this temporary header.

#include "NativeJIT/Packed.h"
#include "PackedMinMaxNode.h"


namespace NativeJIT
{
    void Foo2()
    {
//        auto a = Packed<3>(5).Push<4>(6).Push<5>(7);
        auto a = Packed<3>::Create(5).Push<4>(6).Push<5>(7);
        std::cout << "Field sizes: " << std::hex << a.c_fieldSizes << std::endl;
        std::cout << "Value: " << std::hex << a.GetBits() << std::endl;

        std::cout << std::endl;

        std::cout << std::hex << a.GetBits() << std::endl;
        std::cout << std::hex << a.Pop().GetBits() << std::endl;
        std::cout << std::hex << a.Pop().Pop().GetBits() << std::endl;

        std::cout << std::endl;

        std::cout << std::hex << a.Back() << std::endl;
        std::cout << std::hex << a.Pop().Back() << std::endl;
        std::cout << std::hex << a.Pop().Pop().Back() << std::endl;
    }


    void Foo()
    {
    }
}
