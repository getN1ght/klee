add_klee_unit_test(MemoryTest
MemoryTest.cpp)

target_link_libraries(MemoryTest PRIVATE kleeCore)
target_include_directories(MemoryTest BEFORE PRIVATE "${CMAKE_SOURCE_DIR}/lib")
target_compile_options(MemoryTest PRIVATE ${KLEE_COMPONENT_CXX_FLAGS})
target_compile_definitions(MemoryTest PRIVATE ${KLEE_COMPONENT_CXX_DEFINES})

target_include_directories(MemoryTest PRIVATE ${KLEE_INCLUDE_DIRS})