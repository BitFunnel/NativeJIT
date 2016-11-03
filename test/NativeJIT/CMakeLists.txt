# NativeJIT/test/NativeJITTest

set(CPPFILES
  BitFunnelAcceptanceTest.cpp
  CastTest.cpp
  ConditionalTest.cpp
  ConditionalAutoGenTest.cpp
  ExpressionTreeTest.cpp
  FloatingPointTest.cpp
  FunctionTest.cpp
  PackedTest.cpp
  UnsignedTest.cpp
)

set(PRIVATE_HFILES
)

# CastTest uses a lot of templates which creates a lot of sections in debug mode which requires the /bigobj or equivalent flag.
if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
elseif(CMAKE_COMPILER_IS_GNUCXX AND MINGW)
  # NOTE: the "-Wa, -mbig-obj" flags are supported by mingw
  # but not gcc in Linux.
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT ${PROJECT_SOURCE_DIR}/test/Shared)

add_executable(NativeJITTest ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (NativeJITTest NativeJITTestShared NativeJIT CodeGen gtest gtest_main)

set_property(TARGET NativeJITTest PROPERTY FOLDER "test")
