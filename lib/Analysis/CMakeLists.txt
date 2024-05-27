#===------------------------------------------------------------------------===#
#
#                     The KLEE Symbolic Virtual Machine
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===------------------------------------------------------------------------===#

add_library(kleeAnalysis
    ReachabilityAnalysis.cpp
    Inliner.cpp
    AAPass.cpp
    ModRefAnalysis.cpp
    Annotator.cpp
    Cloner.cpp
    Slicer.cpp
    SVFPointerAnalysis.cpp
    SliceGenerator.cpp
)

llvm_config(kleeAnalysis "${USE_LLVM_SHARED}" core executionengine mcjit native support)


find_library(SVF_CORE_LIB SvfCore HINTS ${SVF_ROOT_DIR}/build/lib)
find_library(SVF_LLVM_LIB SvfLLVM HINTS ${SVF_ROOT_DIR}/build/lib)

# find_library(DG_LLVMSDG_LIB dgllvmsdg HINTS ${DG_ROOT_DIR}/build/lib)
# find_library(DG_SDG_LIB dgsdg HINTS ${DG_ROOT_DIR}/build/lib)
find_library(DG_LLVM_DG_LIB dgllvmdg HINTS ${DG_ROOT_DIR}/build/lib)

target_link_libraries(kleeAnalysis PUBLIC
  ${SVF_LLVM_LIB}
  ${SVF_CORE_LIB}
  # ${DG_LLVMSDG_LIB}
  # ${DG_SDG_LIB}
  ${DG_LLVM_DG_LIB}
  ${LLVM_LIBS}
)

target_include_directories(kleeAnalysis SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})

target_compile_options(kleeAnalysis PRIVATE ${KLEE_COMPONENT_CXX_FLAGS})
target_compile_definitions(kleeAnalysis PRIVATE ${KLEE_COMPONENT_CXX_DEFINES})

target_include_directories(kleeAnalysis SYSTEM PUBLIC
    ${SVF_ROOT_DIR}/svf/include
    ${SVF_ROOT_DIR}/svf-llvm/include
    ${SVF_ROOT_DIR}/build/include
    ${DG_ROOT_DIR}/include
    ${DG_ROOT_DIR}/tools/include
)

