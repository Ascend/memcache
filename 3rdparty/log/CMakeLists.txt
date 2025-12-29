set(SOURCE_DIR ${PROJECT_3RDPARTY_SRC_DIR}/log/spdlog)
set(BUILD_DIR ${PROJECT_BUILD_PATH}/build/spdlog)
set(INSTALL_DIR ${PROJECT_3RDPARTY_BIN_DIR}/spdlog)
set(CHECK_FILE_LINKS ${INSTALL_DIR}/lib/libspdlog.a)
set(SPDLOG_BUILD_TYPE release)
IF (x${CMAKE_BUILD_TYPE} STREQUAL "xDEBUG")
    set(SPDLOG_BUILD_TYPE debug)
    set(CHECK_FILE_LINKS ${INSTALL_DIR}/lib/libspdlogd.a)
endif ()
message(STATUS "spdlog: check if need to build at ${CHECK_FILE_LINKS}")

# build the lib if not built yet
if(EXISTS ${CHECK_FILE_LINKS})
    message(STATUS "spdlog: ${CHECK_FILE_LINKS}")
    message(STATUS "spdlog: has been built, ignored")
else()
    # Create directories
    execute_process(
            COMMAND mkdir -p ${BUILD_DIR}
            RESULT_VARIABLE MKDIR1_RESULT
            OUTPUT_VARIABLE MKDIR1_OUT
            ERROR_VARIABLE MKDIR1_ERR
    )
    if(NOT MKDIR1_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create directory: ${BUILD_DIR}\nstdout: ${MKDIR1_OUT}\nstderr: ${MKDIR1_ERR}")
    endif()

    execute_process(
            COMMAND mkdir -p ${INSTALL_DIR}
            RESULT_VARIABLE MKDIR2_RESULT
            OUTPUT_VARIABLE MKDIR2_OUT
            ERROR_VARIABLE MKDIR2_ERR
    )
    if(NOT MKDIR2_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create directory: ${INSTALL_DIR}\nstdout: ${MKDIR2_OUT}\nstderr: ${MKDIR2_ERR}")
    endif()

    # Detect ninja
    find_program(NINJA_EXE NAMES ninja)
    set(GENERATOR "Unix Makefiles")
    if(NINJA_EXE)
        set(MAKE_CMD ninja)
        set(GENERATOR "Ninja")
    else()
        set(MAKE_CMD make)
    endif()

    # Configure
    execute_process(
            COMMAND cmake
            -G "${GENERATOR}"
            -DCMAKE_BUILD_TYPE=${SPDLOG_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
            -S ${SOURCE_DIR}
            -B ${BUILD_DIR}
            WORKING_DIRECTORY ${BUILD_DIR}
            RESULT_VARIABLE CMAKE_RESULT
            OUTPUT_VARIABLE CMAKE_OUT
            ERROR_VARIABLE CMAKE_ERR
    )
    if(NOT CMAKE_RESULT EQUAL 0)
        message(FATAL_ERROR "CMake configure failed!\nstdout: ${CMAKE_OUT}\nstderr: ${CMAKE_ERR}")
    endif()

    # Clean
    execute_process(
            COMMAND ${MAKE_CMD} clean
            WORKING_DIRECTORY ${BUILD_DIR}
            RESULT_VARIABLE CLEAN_RESULT
            OUTPUT_VARIABLE CLEAN_OUT
            ERROR_VARIABLE CLEAN_ERR
    )
    if(NOT CLEAN_RESULT EQUAL 0)
        message(FATAL_ERROR "${MAKE_CMD} clean failed!\nstdout: ${CLEAN_OUT}\nstderr: ${CLEAN_ERR}")
    endif()

    # Install
    execute_process(
            COMMAND ${MAKE_CMD} install -j8
            WORKING_DIRECTORY ${BUILD_DIR}
            RESULT_VARIABLE INSTALL_RESULT
            OUTPUT_VARIABLE INSTALL_OUT
            ERROR_VARIABLE INSTALL_ERR
    )
    if(NOT INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "${MAKE_CMD} install failed!\nstdout: ${INSTALL_OUT}\nstderr: ${INSTALL_ERR}")
    endif()

    message("-- spdlog: has been success installed by ${MAKE_CMD}")
endif()