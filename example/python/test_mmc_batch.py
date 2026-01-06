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
    kv = {
        "key_1": b"some data",
        "key_2": b"test data",
        "key_3": b"example",
    }

    @classmethod
    def setUpClass(self):
        self._distributed_object_store = DistributedObjectStore()
        self._distributed_object_store.init(count - 1)
        print("object store initialized")

    def test_batch_exist_batch_remove(self):
        # check existence before put
        exist_res = self._distributed_object_store.batch_is_exist(list(self.kv.keys()))
        self.assertEqual(len(exist_res), len(self.kv))
        self.assertTrue(all(res == 0 for res in exist_res))

        # put data
        for k, v in self.kv.items():
            put_res = self._distributed_object_store.put(k, v)
            self.assertEqual(put_res, 0)

        # check existence after put
        exist_res = self._distributed_object_store.batch_is_exist(list(self.kv.keys()))
        self.assertEqual(len(exist_res), len(self.kv))
        self.assertTrue(all(res == 1 for res in exist_res))

        for k, v in self.kv.items():
            retrieved_data = self._distributed_object_store.get(k)
            print(retrieved_data)
            self.assertEqual(retrieved_data, v)

        time.sleep(3)  # wait for the lease to expire

        # test batch remove
        rm_res = self._distributed_object_store.remove_batch(list(self.kv.keys()))
        self.assertTrue(all(res == 0 for res in rm_res))

        # check existence after remove
        exist_res = self._distributed_object_store.batch_is_exist(list(self.kv.keys()))
        self.assertEqual(len(exist_res), len(self.kv))
        self.assertTrue(all(res == 0 for res in exist_res))

    @classmethod
    def tearDownClass(self):
        self._distributed_object_store.close()
        print("object store destroyed")


if __name__ == '__main__':
    unittest.main()
