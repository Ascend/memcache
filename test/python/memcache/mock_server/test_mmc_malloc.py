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
import sys
import unittest
import multiprocessing
from memcache_hybrid import DistributedObjectStore
from memcache_hybrid import MetaService
import acl
import torch
import torch_npu


# 启动 MetaService 在后台线程中运行
def start_meta_service():
    try:
        MetaService.main()
    except Exception as e:
        print(f"MetaService 出错: {e}")


# 启动子进程执行阻塞函数 xxx
proc = multiprocessing.Process(target=start_meta_service)
proc.start()
print(f"子进程已启动，PID: {proc.pid}")
time.sleep(3)


class TestExample(unittest.TestCase):
    key_1 = "key_1"
    original_data = b"some data!"

    def setUp(self):
        print("开始执行测试...")
        acl.init()
        device_id = 3
        ret = acl.rt.set_device(device_id)
        print(f"set_device {device_id} returned: {ret}")
        self._distributed_object_store = DistributedObjectStore()
        res = self._distributed_object_store.init(device_id)
        self.assertEqual(res, 0)

    def test_1(self):
        print("------------------------------ start ------------------------------")
        keys = ["key1", "key2", "key3", "key4"]
        write_tensors = []
        read_tensors = []
        read2_tensors = []
        buffers = []
        read_buffers = []
        read2_buffers = []
        sizes = []
        shape = (61, 10, 1024)
        for i in range(4):
            ts = torch.randn(
                    size=shape,
                    dtype=torch.uint8,
                    device=torch.device('npu')
                )

            rts = torch.empty(
                    size=shape,
                    dtype=torch.uint8,
                    device=torch.device('npu')
                )
            rts2 = torch.empty(
                    size=shape,
                    dtype=torch.uint8,
                    device=torch.device('npu')
                )                
            read_tensors.append(rts)
            write_tensors.append(ts)
            read2_tensors.append(rts2)
            buffers.append(ts.data_ptr())
            read_buffers.append(rts.data_ptr())
            read2_buffers.append(rts2.data_ptr())
            sizes.append(ts.element_size() * ts.nelement())
            print(f"=========={ts.sum().item()=}, {rts.sum().item()=}, {rts2.sum().item()=}")

        
        print(f"=========={sizes=}")
        gvas = self._distributed_object_store.batch_alloc(keys, sizes)
        print(f"=========={gvas=}")

        time.sleep(5)
        ret = self._distributed_object_store.batch_copy(gvas, buffers, sizes, 0)
        self.assertEqual(ret, 0)

        time.sleep(5)
        ret = self._distributed_object_store.batch_copy(gvas, read_buffers, sizes, 1)
        self.assertEqual(ret, 0)

        for rd, wr in zip(read_tensors, write_tensors):
            print(f"=========={wr.sum().item()=}, {rd.sum().item()=}")
            self.assertEqual(rd.sum().item(), wr.sum().item())



        ret = self._distributed_object_store.batch_get_into(keys, read2_buffers, sizes, 1)
        self.assertEqual(ret, [0, 0, 0, 0])
        for rd, wr in zip(read2_tensors, write_tensors):
            print(f"=========={wr.sum().item()=}, {rd.sum().item()=}")
            self.assertEqual(rd.sum().item(), wr.sum().item())

        print("----------------------------- over -------------------------------")


    def tearDown(self):
        self._distributed_object_store.close()
        print("object store destroyed")
        print(f"测试完成，PID: {proc.pid}")
        # 强制终止子进程
        if proc.is_alive():
            print("正在终止子进程...")
            proc.terminate()
            proc.join(timeout=2)
            if proc.is_alive():
                print("子进程未响应，强制杀死...")
                proc.kill()
                proc.join()


if __name__ == '__main__':
    unittest.main()
