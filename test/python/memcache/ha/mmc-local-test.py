import time
import signal
import torch
import uuid
import acl
from enum import Enum
from pymmc import DistributedObjectStore

"""
This py is used for testing leader election in k8s cluster
"""


class MmcDirect(Enum):
    COPY_L2G = 0
    COPY_G2L = 1
    COPY_G2H = 2
    COPY_H2G = 3


def malloc_cpu(layer_num: int = 1, block_num: int = 1, min_block_size: int = 1024):
    if min_block_size <= 0:
        return None
    raw_blocks = torch.rand(
        size=(layer_num, block_num, min_block_size // 2),  # torch.float16占两个字节所以除以2
        dtype=torch.float16,
        device=torch.device('cpu')
    )
    return raw_blocks


def acl_set_device():
    acl.init()
    ret = acl.rt.set_device(0)
    if ret != 0:
        raise RuntimeError("acl set device failed")


class MemCacheStore:
    def __init__(self):
        acl_set_device()
        self.store = DistributedObjectStore()
        ret = self.store.init()
        print(f"init结果: {ret}")

        self.key = "key_loop_" + str(uuid.uuid4())
        self.key_t = "key_ha_rebuild_" + str(uuid.uuid4())
        self.tensor = malloc_cpu(min_block_size=100)

    def put(self):
        direct = int(MmcDirect.COPY_H2G.value)
        ret = self.store.put_from(self.key, self.tensor.data_ptr(), 100, direct)
        print(f"put 结果: {self.key=}, {ret=}")

    def put_t(self):
        """测试 HA rebuild"""
        direct = int(MmcDirect.COPY_H2G.value)
        ret = self.store.put_from(self.key_t, self.tensor.data_ptr(), 100, direct)
        print(f"put_t 结果: {self.key_t=}, {ret=}")

    def exist(self, prompt):
        ret = self.store.is_exist(self.key)
        print(f"{prompt}, is_exist 结果: {self.key=}, {ret}")

    def exist_t(self, prompt):
        """测试 HA rebuild"""
        ret = self.store.is_exist(self.key_t)
        print(f"{prompt}, exist_t 结果: {self.key_t=}, {ret}")

    def remove(self):
        ret = self.store.remove(self.key)
        print(f"remove 结果: {self.key=}, {ret}")

    def test_loop(self):
        loop = 0
        while True:
            self.exist_t(f"loop {loop} after put_t")
            time.sleep(2)
            self.put()
            time.sleep(2)
            self.exist(f"loop {loop} after put")
            time.sleep(2)
            self.remove()
            time.sleep(2)
            self.exist(f"loop {loop} after remove")
            loop += 1

            time.sleep(10)  # 休眠10秒，减少CPU占用

    def close(self):
        ret = self.store.close()
        print(f"close结果: {ret}")


meta_store = MemCacheStore()


# 定义信号处理函数
def handle_signal(signal_num, frame):
    """
    信号处理函数
    :param signal_num: 信号编号
    :param frame: 当前栈帧（通常用不到）
    """
    signal_names = {
        signal.SIGINT: "SIGINT (Ctrl+C)",
        signal.SIGTERM: "SIGTERM (终止信号)",
        signal.SIGUSR1: "SIGUSR1 (用户自定义信号1)"
    }
    signal_name = signal_names.get(signal_num, f"未知信号 ({signal_num})")
    print(f"\n收到信号: {signal_name}")

    # 示例：收到SIGINT时准备退出
    if signal_num == signal.SIGINT or signal_num == signal.SIGTERM:
        print("准备优雅退出...")
        # 这里可以添加清理操作（如关闭文件、释放资源等）

        meta_store.close()

        time.sleep(1)  # 模拟清理过程
        print("退出程序")
        exit(0)


if __name__ == "__main__":
    # 注册信号处理函数
    # 监听 SIGINT (Ctrl+C)
    signal.signal(signal.SIGINT, handle_signal)
    # 监听 SIGTERM（kill 命令默认发送的信号）
    signal.signal(signal.SIGTERM, handle_signal)
    # 监听用户自定义信号 SIGUSR1
    signal.signal(signal.SIGUSR1, handle_signal)

    # 无限循环保持程序运行
    try:
        meta_store.put_t()
        meta_store.test_loop()
    finally:
        # 捕获 Ctrl+C 中断信号，优雅退出
        print("程序被信号终止")
