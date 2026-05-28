# Mock libmf_smem.so for unit tests that stub smem_bm C API via dlopen.
add_library(mock_mf_smem SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/../../testcase/memcache/csrc/meta_service/mock_smem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../testcase/memcache/csrc/meta_service/mock_smem_bm.cpp
)
target_include_directories(mock_mf_smem PRIVATE
        ${PROJECT_SMEM_SRC_BASE}/include/host
)
set_target_properties(mock_mf_smem PROPERTIES
        OUTPUT_NAME "mf_smem"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_PATH}/test/mock_mf_smem/lib64"
)
install(TARGETS mock_mf_smem
        LIBRARY DESTINATION ${PROJECT_OUTPUT_PATH}/test/mock_mf_smem/lib64
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)
