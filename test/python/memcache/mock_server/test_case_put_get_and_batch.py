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
import json
from test_case_common import TestClient

if __name__ == "__main__":
    client = TestClient('61.47.1.122', 5004)
    client.execute("help")
    client.execute("getServerCommands")

    count = 1280
    keys = []
    values = []
    for i in range(count):
        keys.append('test' + str(i))
        values.append(json.dumps({"shape": [512, 64], "dtype": "torch.bfloat16"}).decode("utf-8"))

    media = 0
    client.init_mmc()
    put_batch_res = client.put_batch(keys, values, media)
    assert put_batch_res == 0, "put_batch_res result not successful"

    batch_exist = client.batch_is_exit(keys)
    assert batch_exist == [1] * count, "batch_exist results not correct"

    get_batch_values = client.get_batch(keys)
    assert get_batch_values == values, "get_patch values not correct"
