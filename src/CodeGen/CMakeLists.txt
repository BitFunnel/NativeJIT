# NativeJIT/src/CodeGen

set(CPPFILES
  Allocator.cpp
  BitOperations.cpp
  CodeBuffer.cpp
  ExecutionBuffer.cpp
  FunctionBuffer.cpp
  FunctionSpecification.cpp
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
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/AllocatorVector.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/BitOperations.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Register.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/CallingConvention.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/CodeBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/ExecutionBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/FunctionBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/FunctionSpecification.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/JumpTable.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/ValuePredicates.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGen/X64CodeGenerator.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/Allocator.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/AllocatorOperations.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/Assert.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/IAllocator.h
  ${CMAKE_SOURCE_DIR}/inc/Temporary/StlAllocator.h
)

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

add_library(CodeGen ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET CodeGen PROPERTY FOLDER "src")

