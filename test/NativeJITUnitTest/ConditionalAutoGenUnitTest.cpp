// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Auto generated from CondTest.go. Do not modify directly.



#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace ConditionalUnitTest
    {
        TEST_FIXTURE_START(ConditionalAutoGen)
        TEST_FIXTURE_END_TEST_CASES_BEGIN

        TEST_F(ConditionalAutoGen, AutoGenJGSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JG>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJZUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JZ>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 == p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJZSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JZ>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 == p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJBUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JB>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 < p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNBEUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNBE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJGESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JGE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 >= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJLESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JLE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 <= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNGSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNG>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 <= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNLESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNLE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNZUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNZ>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 != p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNZSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNZ>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 != p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNAUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNA>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 <= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNGESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNGE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 < p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJBEUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JBE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 <= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJAUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JA>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJLSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JL>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 < p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNLSigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNL>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 >= p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNEUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 != p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJNESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JNE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 != p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJEUnsigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (uint64_t p1 = 0; p1 <= 1; ++p1) {
                  for (uint64_t p2 = 0; p2 <= 1; ++p2) {
                    auto expected = (p1 == p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }

        TEST_F(ConditionalAutoGen, AutoGenJESigned)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 1;
                uint64_t falseValue = 128;

                auto & a = expression.Compare<JccType::JE>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                for (int64_t p1 = -1; p1 <= 1; ++p1) {
                  for (int64_t p2 = -1; p2 <= 1; ++p2) {
                    auto expected = (p1 == p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    ASSERT_EQ(expected, observed);
                  }
                }
            }
        }



        TEST_CASES_END
    }
}
