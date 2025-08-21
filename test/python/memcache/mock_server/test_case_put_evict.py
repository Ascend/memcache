#!/usr/bin/python3.11
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.

from test_case_common import TestClient

if __name__ == "__main__":
    client = TestClient('61.47.1.122', 5004)
    client.init_mmc()

    count = 15
    keys = []
    for i in range(count):
        key = 'test_evict_' + str(i)
        client.put_from(key, 10485760, 0)
        keys.append(key)

        if i == 6:
            client.get_into(keys[0], 10485760, 0) 
        

    client.batch_is_exit(keys)

