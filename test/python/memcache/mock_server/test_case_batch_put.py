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

    count = 1280
    keys = []
    put_sizes = []
    get_sizes = []
    for i in range(count):
        keys.append('test' + str(i))
        put_sizes.append(1024)
        get_sizes.append(1024)

    print("============ test cpu =========== ")
    media = 0
    client.init_mmc()
    client.batch_put_from(keys, put_sizes, media)
    client.batch_is_exit(keys)
