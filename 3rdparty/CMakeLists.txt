add_subdirectory(log)

####################################################################
# MemFabric_Hybrid
####################################################################
set(HYBRID_SOURCE_DIR ${PROJECT_3RDPARTY_SRC_DIR}/memfabric_hybrid)
set(HYBRID_BUILD_DIR ${PROJECT_BUILD_PATH})
set(HYBRID_INSTALL_DIR ${PROJECT_OUTPUT_PATH})
set(HYBRID_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(HYBRID_OPEN_ABI ${BUILD_OPEN_ABI})
set(HYBRID_BUILD_PYTHON ${BUILD_PYTHON})
set(HYBRID_ENABLE_PTRACER ${ENABLE_PTRACER})
set(HYBRID_CHECK_FILE_LINKS ${HYBRID_INSTALL_DIR}/smem/lib64/libmf_smem.so)
# build the lib if not built yet
if(EXISTS ${HYBRID_CHECK_FILE_LINKS})
    message(STATUS "memfabric_hybrid: has been built, ignored")
else()
    # create dir
    exec_program(mkdir ${HYBRID_BUILD_DIR} ARGS -p ${HYBRID_BUILD_DIR})
    exec_program(mkdir ${HYBRID_BUILD_DIR} ARGS -p ${HYBRID_INSTALL_DIR})

    # configure
    exec_program(
        cmake
        ARGS -B ${HYBRID_BUILD_DIR}
        -DCMAKE_BUILD_TYPE=${HYBRID_BUILD_TYPE}
        -DBUILD_OPEN_ABI=${HYBRID_OPEN_ABI}
        -DBUILD_PYTHON=${HYBRID_BUILD_PYTHON}
        -DENABLE_PTRACER=${HYBRID_ENABLE_PTRACER}
        -DCMAKE_INSTALL_PREFIX=${HYBRID_INSTALL_DIR}
        -S ${HYBRID_SOURCE_DIR}
    )

    # make and make install
    exec_program(make ${HYBRID_BUILD_DIR} ARGS install -j32)
    message("-- memfabric_hybrid: has been success installed")
endif()