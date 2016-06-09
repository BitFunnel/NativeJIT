# NativeJIT/src/NativeJIT

set(CPPFILES
  CallNode.cpp
  ExpressionNodeFactory.cpp
  ExpressionTree.cpp
  Node.cpp
)

set(PRIVATE_HFILES
)

set(PUBLIC_HFILES
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/CodeGenHelpers.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExecutionPreconditionTest.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExpressionNodeFactory.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExpressionNodeFactoryDecls.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExpressionTree.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/ExpressionTreeDecls.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Function.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Model.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/BinaryImmediateNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/BinaryNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/CallNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/CastNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ConditionalNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/FieldPointerNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ImmediateNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ImmediateNodeDecls.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/IndirectNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/Node.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/PackedMinMaxNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ParameterNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ReturnNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/ShldNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Nodes/StackVariableNode.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/Packed.h
  ${CMAKE_SOURCE_DIR}/inc/NativeJIT/TypePredicates.h
)

source_group("inc/NativeJIT" FILES ${PUBLIC_HFILES})

add_library(NativeJIT ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})

set_property(TARGET NativeJIT PROPERTY FOLDER "src")
