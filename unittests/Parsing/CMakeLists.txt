add_klee_unit_test(ParsingTest
  ParsingTest.cpp
  )

add_resources_dir(resources)

target_link_libraries(ParsingTest PRIVATE kleaverExpr kleeModule kleaverSolver)
target_compile_options(ParsingTest PRIVATE ${KLEE_COMPONENT_CXX_FLAGS})
target_compile_definitions(ParsingTest PRIVATE ${KLEE_COMPONENT_CXX_DEFINES})

target_include_directories(ParsingTest PRIVATE ${KLEE_INCLUDE_DIRS})

