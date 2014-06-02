#pragma once

#include <type_traits>

namespace NativeJIT
{
    template <typename T> struct IsFloatingPointType : std::false_type {};
    template <> struct IsFloatingPointType<float> : std::true_type {};
    template <> struct IsFloatingPointType<double> : std::true_type {};
}
