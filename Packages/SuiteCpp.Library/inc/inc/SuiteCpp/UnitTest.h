#pragma once

#include "SuiteCpp\UnitTestImpl.h"

#define TestClass(className) \
    class className : public SuiteCpp::TestClassFactory<className>
#define TestCase(testName) \
    _declspec(dllexport) void _m_SuiteCpp_Test_##testName()

#define TestFail(...) \
    SuiteCpp::TestFailImpl(__FILE__, __LINE__, __VA_ARGS__)
#define TestAssert(condition, ...) \
    SuiteCpp::TestAssertImpl(__FILE__, __LINE__, condition, #condition, __VA_ARGS__)
#define TestEqual(expected, actual, ...) \
    SuiteCpp::TestEqualImpl(__FILE__, __LINE__, expected, actual, #expected, #actual, __VA_ARGS__)
#define TestNotEqual(notExpected, actual, ...) \
    SuiteCpp::TestNotEqualImpl(__FILE__, __LINE__, notExpected, actual, #notExpected, #actual, __VA_ARGS__)
#define TestNull(pointer, ...) \
    SuiteCpp::TestNullImpl(__FILE__, __LINE__, pointer, #pointer, __VA_ARGS__)
#define TestNotNull(pointer, ...) \
    SuiteCpp::TestNotNullImpl(__FILE__, __LINE__, pointer, #pointer, __VA_ARGS__)