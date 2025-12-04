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

import time
import unittest
from memcache_hybrid import DistributedObjectStore
import acl

acl.init()
count, ret = acl.rt.get_device_count()
print("设备数量:", acl.rt.get_device_count())
ret = acl.rt.set_device(count - 1)
print("set_device returned: {}".format(ret))


class TestExample(unittest.TestCase):
    key_1 = "key_1"
    original_data = b"some data!"

    def setUp(self):
        self._distributed_object_store = DistributedObjectStore()
        res = self._distributed_object_store.init(count - 1)
        self.assertEqual(res, 0)

    def test_1(self):
        # check existence before put
        exist_res = self._distributed_object_store.is_exist(self.key_1)
        self.assertEqual(exist_res, 0)

        # test put
        put_res = self._distributed_object_store.put(self.key_1, self.original_data)
        self.assertEqual(put_res, 0)

        # check existence after put
        exist_res = self._distributed_object_store.is_exist(self.key_1)
        self.assertEqual(exist_res, 1)

        key_info = self._distributed_object_store.get_key_info(self.key_1)
        print(key_info)

        retrieved_data = self._distributed_object_store.get(self.key_1)
        print(retrieved_data)
        self.assertEqual(retrieved_data, self.original_data)

        time.sleep(3) # wait for the lease to expire

        # test remove
        rm_res = self._distributed_object_store.remove(self.key_1)
        self.assertEqual(rm_res, 0)

        # check existence after remove
        exist_res = self._distributed_object_store.is_exist(self.key_1)
        self.assertEqual(exist_res, 0)

    def tearDown(self):
        self._distributed_object_store.close()
        print("object store destroyed")


if __name__ == '__main__':
    unittest.main()