#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
# MemCache_Hybrid is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

from test_case_common import TestClient

if __name__ == "__main__":
    client = TestClient('127.0.0.1', 10001)
    client.execute("help")
    client.execute("getServerCommands")

    test_key = "test"
    media = 0

    print("============ test cpu =========== ")
    client.init_mmc()
    client.is_exist(test_key)
    client.put_from(test_key, 1024, media)
    client.is_exist(test_key)
    client.get_into(test_key, 1024, media)
    client.remove(test_key)
    client.is_exist(test_key)

    print("============ test npu =========== ")
    media = 1
    client.put_from(test_key, 1024, media)
    client.is_exist(test_key)
    client.get_into(test_key, 1024, media)
    client.remove(test_key)
    client.is_exist(test_key)

    client.close_mmc()