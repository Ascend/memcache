include_directories(
    ${PROJECT_SMEM_SRC_BASE}/csrc/common
    ${PROJECT_3RDPARTY_BIN_DIR}/acc_links/include/
    ${PROJECT_SOURCE_DIR}/3rdparty/net/acc_links/src/net/include
    ${PROJECT_UTIL_SRC_BASE}
)
aux_source_directory(. CONFIG_STORE_SRCS)
add_library(config_store_object OBJECT ${CONFIG_STORE_SRCS})
target_compile_options(config_store_object PRIVATE -fPIC)