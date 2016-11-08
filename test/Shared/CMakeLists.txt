# NativeJIT/test/Shared

set(CPPFILES
  TestSetup.cpp
  TestSetup.h
)

set(PRIVATE_HFILES
)

set(PUBLIC_HFILES
  TestSetup.h
)

include_directories(${PROJECT_SOURCE_DIR}/src/NativeJIT)

add_library(NativeJITTestShared ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET NativeJITTestShared PROPERTY FOLDER "${NATIVEJIT_PREFIX}test")
