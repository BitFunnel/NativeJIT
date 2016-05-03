# NativeJIT/Examples/Parser.cpp

set(CPPFILES
  Parser.cpp
  )

set(PRIVATE_HFILES
  )

# This include_directories is redundant because the root CMakeLists.txt
# for NativeJIT sets it correctly. If you build this example outside of
# the NativeJIT project, be sure to update the include_directories to
# point to the inc subdirectory of NativeJIT.
include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(Parser ${CPPFILES} ${PRIVATE_HFILES})
target_link_libraries (Parser CodeGen NativeJIT)

# This line makes AreaOfCircle appear in the correct VS solution
# folder in the NativeJIT project. Delete this line if building
# outside of NativeJIT.
set_property(TARGET Parser PROPERTY FOLDER "Examples")
