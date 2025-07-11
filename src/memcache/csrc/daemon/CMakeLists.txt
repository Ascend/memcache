file(GLOB_RECURSE DAEMON_SRCS *.cc *.cpp *.h)

add_executable(mmc_meta_service ${DAEMON_SRCS})

target_link_libraries(mmc_meta_service PUBLIC mmc_static smem_static pthread libacc_tcp_net.a)

install(TARGETS mmc_meta_service
        RUNTIME DESTINATION ${TARGET_INSTALL_DIR}/memcache/bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE)