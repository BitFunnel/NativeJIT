# NativeJIT/src/CodeGen

set(CPPFILES
  CodeBuffer.cpp
  ExecutionBuffer.cpp
  FunctionBuffer.cpp
  JumpTable.cpp
  Register.cpp
  stdafx.cpp
  UnwindCode.cpp
  ValuePredicates.cpp
  X64CodeGenerator.cpp
  )

set(PRIVATE_HFILES
  stdafx.h
  targetver.h
  UnwindCode.h
  )

set(PUBLIC_HFILES
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExecutionBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/FunctionBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/JumpTable.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Register.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ValuePredicates.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/X64CodeGenerator.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/Allocator.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/Assert.h
)

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

add_library(CodeGen ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET CodeGen PROPERTY FOLDER "src")

