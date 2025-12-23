# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
# MemCache_Hybrid is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/googletest/googletest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/googletest/googlemock/include)

include_directories(
        ${PROJECT_SMEM_SRC_BASE}/include/host
        ${PROJECT_HYBM_SRC_BASE}/include
        ${PROJECT_ACCLINKS_SRC_BASE}/include
        ${PROJECT_3RDPARTY_SRC_DIR}/cpp-httplib
        ${PROJECT_3RDPARTY_SRC_DIR}/nlohmann/json/single_include/
        ${PROJECT_3RDPARTY_SRC_DIR}/prometheus-cpp-lite/simpleapi/include
        ${PROJECT_3RDPARTY_SRC_DIR}/prometheus-cpp-lite/core/include
)
add_subdirectory(3rdparty)
link_directories(${PROJECT_3RDPARTY_BIN_DIR}/googletest/lib)

set(ENABLE_FUZZ $ENV{ENABLE_FUZZ})
if (ENABLE_FUZZ STREQUAL "ON")
    message(STATUS "ENABLE_FUZZ is ${ENABLE_FUZZ}")
    add_subdirectory(fuzz)
    add_subdirectory(ut/mock)
else ()
    add_subdirectory(ut)
    add_subdirectory(ut/mock)
endif()