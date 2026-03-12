# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
# MemCache_Hybrid is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

add_executable(test_ubs_io_proxy test_ubs_io_proxy.cpp)
target_include_directories(test_ubs_io_proxy PRIVATE
    ${CMAKE_SOURCE_DIR}/src/memcache/csrc
    ${CMAKE_SOURCE_DIR}/src/memcache/csrc/under_api/ubs_io
    ${CMAKE_SOURCE_DIR}/test/ut/testcase/memcache/include
)
target_link_libraries(test_ubs_io_proxy PRIVATE
    memcache
    gtest
    gtest_main
    pthread
)

add_test(NAME test_ubs_io_proxy COMMAND test_ubs_io_proxy)
