# NativeJIT/Examples/Functional/Functional.cpp

set(CPPFILES
  Functional.cpp
  )

set(PRIVATE_HFILES
  )

# This include_directories is redundant because the root CMakeLists.txt
# for NativeJIT sets it correctly. If you build this example outside of
# the NativeJIT project, be sure to update the include_directories to
# point to the inc subdirectory of NativeJIT.
include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(Functional ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (Functional CodeGen NativeJIT)

# This line makes Functional appear in the correct VS solution
# folder in the NativeJIT project. Delete this line if building
# outside of NativeJIT.
set_property(TARGET Functional PROPERTY FOLDER "${NATIVEJIT_PREFIX}Examples")
