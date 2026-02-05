import os
import time
from typing import Optional
from pathlib import Path


class StatusFileManager:

    def __init__(self, file_path: str):
        self.file_path = Path(file_path)

    def write_status(self, status: int) -> bool:
        try:
            with open(self.file_path, 'w') as f:
                f.write(str(status))
            return True
        except Exception as e:
            print(f"写入状态失败: {e}")
            return False

    def read_status(self) -> Optional[int]:
        try:
            if not self.file_path.exists():
                return None

            with open(self.file_path, 'r') as f:
                content = f.read().strip()
                return int(content) if content.isdigit() else None
        except Exception as e:
            print(f"读取状态失败: {e}")
            return None

    def check_other_file_status(self, other_file_path: str) -> Optional[int]:
        try:
            other_path = Path(other_file_path)
            if not other_path.exists():
                return None

            with open(other_path, 'r') as f:
                content = f.read().strip()
                return int(content) if content.isdigit() else None
        except Exception as e:
            print(f"检查其他文件状态失败: {e}")
            return None

    def is_ready(self) -> bool:
        status = self.read_status()
        return status == 1

    def wait_until_ready(self, timeout: int = 60, check_interval: float = 0.001, check_ready: bool = True) -> bool:
        start_time = time.time()

        while time.time() - start_time < timeout:
            if self.is_ready() == check_ready:
                return True
            #time.sleep(check_interval)

        return False

    def reset_to_preparing(self) -> bool:
        return self.write_status(0)

    def set_to_ready(self) -> bool:
        return self.write_status(1)