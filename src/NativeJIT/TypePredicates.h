#pragma once

#include <type_traits>


namespace NativeJIT
{
    // A trait used to determine whether a type has any decay other than
    // const/volatile.
    template <typename T>
    struct HasMeaningfulDecay
        : std::integral_constant<
            bool,
            !std::is_same<typename std::remove_cv<T>::type,
                          typename std::decay<T>::type>
                        ::value>
    {
    };
}
