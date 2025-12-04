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

from multiprocessing import Process
import os

import torch
import torch_npu

#from mutil_process import process_count, write_worker
from mooncake_mutil_process import process_count, write_worker

if __name__ == "__main__":
    print(f"主进程 PID: {os.getpid()}")
    process = []
    # 创建两个子进程
    for index in range(process_count):
        p = Process(target=write_worker, args=(index,))
        p.start()
        process.append(p)

    for i in range(process_count):
        process[i].join()
    print("所有子进程结束。")