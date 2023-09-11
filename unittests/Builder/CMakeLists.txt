add_klee_unit_test(BuilderTest
                   Z3AdapterTest.cpp)
target_link_libraries(BuilderTest PRIVATE kleaverExpr kleeSupport kleaverSolver)
target_compile_options(BuilderTest PRIVATE ${KLEE_COMPONENT_CXX_FLAGS})
target_compile_definitions(BuilderTest PRIVATE ${KLEE_COMPONENT_CXX_DEFINES})

target_include_directories(BuilderTest PRIVATE ${KLEE_INCLUDE_DIRS})
