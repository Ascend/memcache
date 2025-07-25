#!/usr/bin/env python3.10
#  Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.

import ast
import concurrent.futures
import dataclasses
import inspect
import json
import socket
import sys
import threading
import traceback
from functools import wraps
from typing import Callable, Dict, List

from pymmc import DistributedObjectStore


@dataclasses.dataclass
class CliCommand:
    cmd_name: str
    cmd_description: str
    func: Callable
    args_num: int


class TestServer:
    def __init__(self, ip, port):
        self._ip_port = (ip, int(port))
        self._server_socket = None
        self._commands: Dict[str:CliCommand] = {}
        self._thread_local = threading.local()
        self._thread_local.client_socket = None
        self._register_inner_command()

    def _register_inner_command(self):
        self._commands = {
            "help": CliCommand("help", "show command list information", self._help, 0),
            "getServerCommands": CliCommand("getServerCommands", "getServerCommands, get the registered Commands",
                                            self._get_server_commands, 0),
        }

    def register_command(self, cmds: List[CliCommand]):
        for cmd in cmds:
            self._commands[cmd.cmd_name] = cmd

    @staticmethod
    def _convert_argument(arg_str: str, param_type):
        try:
            if param_type == int:
                return int(arg_str)
            elif param_type == float:
                return float(arg_str)
            elif param_type == str:
                return str(arg_str)
            elif param_type == bool:
                return arg_str.lower() in ['true', '1', 'yes']
            elif param_type == bytes:
                return bytes(arg_str, 'utf-8')
            else:
                # 如果是其他类型，可以使用 ast.literal_eval
                return ast.literal_eval(arg_str)
        except (ValueError, SyntaxError):
            # 如果无法转换，返回原始字符串
            return arg_str

    @staticmethod
    # 解析参数并根据目标函数的参数类型进行转换
    def _parse_arguments(func, arg_strs):
        signature = inspect.signature(func)
        parsed_args = []
        for param, arg in zip(signature.parameters.values(), arg_strs):
            parsed_args.append(TestServer._convert_argument(arg, param.annotation))
        return parsed_args

    def _execute(self, request):
        """执行命令。"""
        cmd_str = request.get("cmd")
        args = request.get("args")
        command = self._commands.get(cmd_str)
        if not command:
            self.cli_print(f"Unknown command: {cmd_str}")
            self._help()
            self._cli_end_line()
            return
        if len(args) != command.args_num:
            self.cli_print(f"Invalid input args num: {len(args)}.")
            self._help()
            self._cli_end_line()
            return
        parsed_params = self._parse_arguments(command.func, args)
        command.func(*parsed_params)
        self._cli_end_line()

    def start(self):
        self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server_socket.bind(self._ip_port)
        self._server_socket.listen(5)
        with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
            while True:
                client_socket, _ = self._server_socket.accept()
                executor.submit(self._handle_client, client_socket)

    def _handle_client(self, client_socket: socket):
        self._thread_local.client_socket = client_socket
        buffer_list = []
        try:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    self._thread_local.client_socket = None
                    break
                buffer_list.append(data)
                if not data.endswith(b"\0"):
                    continue
                msg = b''.join(buffer_list).decode('utf-8').replace("\0", "").strip()
                request = json.loads(msg)
                print(f"received request: {request}")

                try:
                    self._execute(request)
                except Exception:
                    traceback.print_exc()
                finally:
                    buffer_list.clear()
        finally:
            client_socket.close()

    def cli_print(self, msg: str):
        self._thread_local.client_socket.send(f"{msg}\n".encode('utf-8'))

    def cli_return(self, obj):
        obj_type = type(obj)
        if obj_type is int:
            data = str(obj).encode('utf-8')
        elif obj_type is bytes:
            data = obj
        else:
            raise TypeError(f"Unsupported return type: {obj_type}")
        self._thread_local.client_socket.send(data)

    def _cli_end_line(self):
        print("send command result")
        self._thread_local.client_socket.send("\0".encode('utf-8'))

    def _help(self):
        """显示帮助信息。"""
        col_widths = max(len(item) for item in self._commands.keys()) + 1
        self.cli_print("Available commands:")
        for cmd in self._commands.values():
            self.cli_print(f":  {cmd.cmd_name: >{col_widths}}: {cmd.cmd_description}")

    def _get_server_commands(self):
        msg = ",".join(self._commands.keys())
        self.cli_print(f"{msg}")


def result_handler(func):
    @wraps(func)
    def wrapper(self, *args, **kwargs):
        try:
            func(self, *args, **kwargs)
        except Exception as e:
            self.cli_print(f"{func.__name__} raised exception: {e}")

    return wrapper


class MmcTest(TestServer):
    def __init__(self, ip, port):
        super().__init__(ip, port)
        self._init_cmds()
        self.__distributed_store_object = None

    def _init_cmds(self):
        cmds = [
            CliCommand("init_mmc", "initialize memcache", self.init_mmc, 0),
            CliCommand("close_mmc", "destruct memcache", self.close_mmc, 0),
            CliCommand("put", "put data in bytes format: [key] [data]", self.put, 2),
            CliCommand("get", "get data in bytes format: [key]", self.get, 1),
            CliCommand("is_exist", "check if a key exist: [key]", self.is_exist, 1),
            CliCommand("remove", "remove data: [key]", self.remove, 1),
        ]
        self.register_command(cmds)

    @result_handler
    def print(self):
        self.cli_print("test print info")

    @result_handler
    def init_mmc(self):
        self.__distributed_store_object = DistributedObjectStore()
        res = self.__distributed_store_object.init()
        self.cli_return(res)

    @result_handler
    def close_mmc(self):
        res = self.__distributed_store_object.close()
        self.cli_return(res)

    @result_handler
    def put(self, key: str, data: bytes):
        res = self.__distributed_store_object.put(key, data)
        self.cli_return(res)

    @result_handler
    def get(self, key: str):
        res = self.__distributed_store_object.get(key)
        self.cli_return(res)

    @result_handler
    def is_exist(self, key: str):
        res = self.__distributed_store_object.is_exist(key)
        self.cli_return(res)

    @result_handler
    def remove(self, key: str):
        res = self.__distributed_store_object.remove(key)
        self.cli_return(res)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Please input ip and port when starting the process.")
        sys.exit(1)
    _, ip, port = sys.argv
    print(f"Start app_id: {ip}:{port}")
    server = MmcTest(ip, port)
    server.start()
