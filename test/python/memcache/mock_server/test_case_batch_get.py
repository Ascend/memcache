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
    client = TestClient('61.47.1.122', 5004)
    client.execute("help")
    client.execute("getServerCommands")
    client.init_mmc()
    count = 1280
    keys = []
    put_sizes = []
    get_sizes = []
    for i in range(count):
        key = 'test' + str(i)
        ret = client.is_exist(key)
        if ret == "1":
            res = client.get_into(key, 1024, 0)
            if res[1] != "0":
                print(f" {key} read failed:{res}, type:{type(res[1])}, {res[1]}")
        else:
            print(f" {key} not exist")
