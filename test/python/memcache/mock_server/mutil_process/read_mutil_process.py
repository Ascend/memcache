from multiprocessing import Process
import os

import torch
import torch_npu

from memcache import DistributedObjectStore

import mutil_process

if __name__ == "__main__":
    print(f"主进程 PID: {os.getpid()}")
    process = []
    # 创建两个子进程
    for index in range(mutil_process.process_count):
        p = Process(target=mutil_process.read_worker, args=(index,))
        p.start()
        process.append(p)

    for i in range(mutil_process.process_count):
        process[i].join()
    print("所有子进程结束。")

