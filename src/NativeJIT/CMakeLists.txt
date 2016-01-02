# NativeJIT/src/NativeJIT

set(CPPFILES
  CallNode.cpp
  Examples.cpp
  ExpressionNodeFactory.cpp
  ExpressionTree.cpp
  Node.cpp
  stdafx.cpp
)

set(PRIVATE_HFILES
  BinaryImmediateNode.h
  BinaryNode.h
  CallNode.h
  CastNode.h
  CodeGenHelpers.h
  ConditionalNode.h
  Examples.h
  ExecutionPreconditionTest.h
  ExpressionNodeFactory.h
  ExpressionTree.h
  FieldPointerNode.h
  Function.h
  ImmediateNode.h
  IndirectNode.h
  Node.h
  PackedMinMaxNode.h
  ParameterNode.h
  ReturnNode.h
  ShldNode.h
  StackVariableNode.h
  stdafx.h
  targetver.h
  TypePredicates.h
)

set(PUBLIC_HFILES
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Model.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Packed.h
)

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

add_library(NativeJIT ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET NativeJIT PROPERTY FOLDER "src")
