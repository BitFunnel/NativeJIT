# NativeJIT/UnitTest/Shared

set(CPPFILES
  TestSetup.cpp
  TestSetup.h
)

set(PRIVATE_HFILES
  stdafx.h
  targetver.h
)

set(PUBLIC_HFILES
  TestSetup.h
)

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT)

add_library(UnitTestShared ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET UnitTestShared PROPERTY FOLDER "UnitTest")
