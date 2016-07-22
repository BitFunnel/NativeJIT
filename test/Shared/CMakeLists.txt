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

add_library(TestShared ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET TestShared PROPERTY FOLDER "test")
