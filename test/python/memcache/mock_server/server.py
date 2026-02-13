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

import concurrent.futures
import dataclasses
import inspect
import json
import logging
import socket
import sys
import threading
import time
import traceback
from functools import wraps
from typing import Callable, Dict, List, Tuple
from enum import Enum

import torch

from memcache_hybrid import DistributedObjectStore, ReplicateConfig


logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


class MmcDirect(Enum):
    COPY_L2G = 0
    COPY_G2L = 1
    COPY_G2H = 2
    COPY_H2G = 3


@dataclasses.dataclass
class CliCommand:
    cmd_name: str
    cmd_desc: str
    func: Callable
    required_args_num: int


class TestServer:
    def __init__(self, ip, port):
        self._ip_port = (ip, int(port))
        self._server_socket = None
        self._commands: Dict[str:CliCommand] = {}
        self._thread_local = threading.local()
        self._thread_local.client_socket = None
        self._register_inner_command()

    def __del__(self):
        self._server_socket.close()

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
    def _convert_argument(arg, param_type):
        try:
            if arg == "__NONE__":
                return None
            elif param_type == int:
                return int(arg)
            elif param_type == float:
                return float(arg)
            elif param_type == str:
                return str(arg)
            elif param_type == bytes:
                return bytes(arg, 'utf-8')
            elif param_type == List[bytes]:
                return [val.encode('utf-8') for val in arg]
            else:
                return arg
        except (ValueError, SyntaxError) as e:
            logging.error(f"failed to convert to {param_type}, error: {e}")
            return arg

    @staticmethod
    # 解析参数并根据目标函数的参数类型进行转换
    def _parse_arguments(func, args):
        signature = inspect.signature(func)
        parsed_args = []
        for param, arg in zip(signature.parameters.values(), args):
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
        if len(args) < command.required_args_num:
            self.cli_print(f"Invalid input args num: {len(args)}, required args num: {command.required_args_num}")
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

    def _handle_client(self, client_socket: socket.socket):
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
                logging.debug("received request: %s", request)

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
            data = str(obj).encode('utf-8')
        self._thread_local.client_socket.send(data)

    def _cli_end_line(self):
        logging.debug("send command result")
        self._thread_local.client_socket.send("\0".encode('utf-8'))

    def _help(self):
        """显示帮助信息。"""
        col_widths = max(len(item) for item in self._commands.keys()) + 1
        self.cli_print("Available commands:")
        for cmd in self._commands.values():
            self.cli_print(f":  {cmd.cmd_name: >{col_widths}}: {cmd.cmd_desc}")

    def _get_server_commands(self):
        msg = ",".join(self._commands.keys())
        self.cli_print(f"{msg}")


def result_handler(func):
    @wraps(func)
    def wrapper(self, *args, **kwargs):
        try:
            func(self, *args, **kwargs)
        except Exception as e:
            traceback.print_exc()
            self.cli_print(f"{func.__name__} raised exception: {e}")

    return wrapper


def tensor_sum(tensor, sizes: List[int] = None):
    if tensor is None:
        return 0
    if sizes is None:
        return tensor.sum().item()

    return sum(layer[:size].sum().item() for layer, size in zip(tensor, sizes))


class MmcTest(TestServer):
    def __init__(self, ip, port, device_id=0):
        super().__init__(ip, port)
        self._device_id = device_id
        self._init_cmds()
        self._store = None

    def _init_cmds(self):
        cmds = [
            CliCommand("init_mmc", "initialize memcache", self.init_mmc, 0),
            CliCommand("close_mmc", "destruct memcache", self.close_mmc, 0),
            CliCommand("get_local_service_id", "get local service id", self.get_local_service_id, 0),
            CliCommand("put", "put data in bytes format: [key] [data]", self.put, 2),
            CliCommand("put_batch", "put batch datas in bytes format: [keys] [values]", self.put_batch, 2),
            CliCommand("get", "get data in bytes format: [key]", self.get, 1),
            CliCommand("get_batch", "get batch datas in bytes format: [keys]", self.get_batch, 1),
            CliCommand("put_from", "put data from a buffer: [key] [size] [media(0:cpu 1:npu)]", self.put_from, 3),
            CliCommand("get_into", "get data into a buffer: [key] [size] [media(0:cpu 1:npu)]", self.get_into, 3),
            CliCommand("batch_get_into", "batch put data: [keys] [sizes] [media(0:cpu 1:npu)]", self.batch_get_into, 3),
            CliCommand("batch_put_from", "batch get data: [keys] [sizes] [media(0:cpu 1:npu)]", self.batch_put_from, 3),
            CliCommand("is_exist", "check if a key exist: [key]", self.is_exist, 1),
            CliCommand("batch_is_exist", "check if a batch of keys exist: [keys]", self.batch_is_exist, 1),
            CliCommand("remove", "remove data: [key]", self.remove, 1),
            CliCommand("remove_batch", "remove a batch of data: [keys]", self.remove_batch, 1),
            CliCommand("remove_all", "remove all keys", self.remove_all, 0),
            CliCommand("get_key_info", "get data info of: [key]", self.get_key_info, 1),
            CliCommand("batch_get_key_info", "batch get data info of: [keys]", self.batch_get_key_info, 1),
            CliCommand("put_from_layers", "put data from multiple buffers [key] [sizes] [media(0:cpu 1:npu)]",
                       self.put_from_layers, 3),
            CliCommand("get_into_layers", "get data into multiple buffers [key] [sizes] [media(0:cpu 1:npu)]",
                       self.get_into_layers, 3),
            CliCommand("batch_put_from_layers", func=self.batch_put_from_layers, required_args_num=3,
                cmd_desc="batch put data from multiple buffers [keys] [sizes] [media(0:cpu 1:npu)]"),
            CliCommand("batch_get_into_layers", func=self.batch_get_into_layers, required_args_num=3,
                cmd_desc="batch get data into multiple buffers [keys] [sizes] [media(0:cpu 1:npu)]"),
            CliCommand("perf_test_put_from", func=self.perf_test_put_from, required_args_num=2,
                cmd_desc="test put_from performance: [size] [iter_count] [medium] [register] [preferred_rank]"),
            CliCommand("perf_test_get_into", func=self.perf_test_get_into, required_args_num=2,
                cmd_desc="test get_into performance: [size] [iter_count] [medium] [register]"),
            CliCommand("perf_test_put_from_layers", func=self.perf_test_put_from_layers, required_args_num=2,
                cmd_desc="test put_from_layers performance: [sizes] [iter_count] [medium] [register] [preferred_rank]"),
            CliCommand("perf_test_get_into_layers", func=self.perf_test_get_into_layers, required_args_num=2,
                cmd_desc="test get_into_layers performance: [sizes] [iter_count] [medium] [register]"),
        ]
        self.register_command(cmds)

    @result_handler
    def print(self):
        self.cli_print("test print info")

    @result_handler
    def init_mmc(self):
        self.set_device()
        self._store = DistributedObjectStore()
        res = self._store.init(self._device_id)
        self.cli_return(res)

    @result_handler
    def close_mmc(self):
        if self._store:
            res = self._store.close()
            self.cli_return(res)
        else:
            self.cli_return(0)

    @result_handler
    def get_local_service_id(self):
        self.cli_return(self._store.get_local_service_id())

    @result_handler
    def put(self, key: str, data: bytes, replica_num: int | None = None, preferred_ranks: List[int] | None = None):
        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks
        res = self._store.put(key, data, rep_conf)
        self.cli_return(res)

    @result_handler
    def put_batch(self, keys: List[str], values: List[bytes], replica_num: int | None = None,
                  preferred_ranks: List[int] | None = None):
        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks
        res = self._store.put_batch(keys, values, rep_conf)
        self.cli_return(res)

    @result_handler
    def put_from(self, key: str, size: int, media: int, replica_num: int | None = None,
                 preferred_ranks: List[int] | None = None):
        if media == 0:
            direct = int(MmcDirect.COPY_H2G.value)
            tensor = self.malloc_tensor(mini_block_size=size, device='cpu')
        else:
            direct = int(MmcDirect.COPY_L2G.value)
            tensor = self.malloc_tensor(mini_block_size=size, device='npu')
        if tensor is not None:
            self._store.register_buffer(tensor.data_ptr(), size)
        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks

        if size <= 0:
            res = self._store.put_from(key, 0, 0, direct)
            value = 0
        else:
            res = self._store.put_from(key, tensor.data_ptr(), size, direct, rep_conf)
            value = tensor_sum(tensor)
        if tensor is not None:
            self._store.unregister_buffer(tensor.data_ptr(), size)
        self.cli_return(str([res, value]))

    @result_handler
    def get(self, key: str):
        res = self._store.get(key)
        self.cli_return(res)

    @result_handler
    def get_batch(self, keys: List[str]):
        values = self._store.get_batch(keys)
        self.cli_return(values)

    @result_handler
    def get_into(self, key: str, size: int, media: int):
        if media == 0:
            direct = int(MmcDirect.COPY_G2H.value)
            tensor = self.malloc_tensor(mini_block_size=size, device='cpu')
        else:
            direct = int(MmcDirect.COPY_G2L.value)
            tensor = self.malloc_tensor(mini_block_size=size, device='npu')
        if tensor is not None:
            self._store.register_buffer(tensor.data_ptr(), size)
        if size <= 0:
            res = self._store.get_into(key, 0, 0, direct)
            value = 0
        else:
            res = self._store.get_into(key, tensor[0].data_ptr(), size, direct)
            value = tensor_sum(tensor)
        if tensor is not None:
            self._store.unregister_buffer(tensor.data_ptr(), size)
        self.cli_return(str([res, value]))

    @result_handler
    def batch_get_into(self, keys: list, sizes: list, media: int):
        data_ptrs = []
        blocks = []
        if media == 0:
            direct = int(MmcDirect.COPY_G2H.value)
            device = 'cpu'
        else:
            direct = int(MmcDirect.COPY_G2L.value)
            device = 'npu'
        for size in sizes:
            tensor = self.malloc_tensor(mini_block_size=size, device=device)
            if tensor is not None:
                self._store.register_buffer(tensor.data_ptr(), size)
            blocks.append(tensor)
        for i in range(len(sizes)):
            if blocks[i] is None:
                data_ptrs.append(0)
            else:
                data_ptrs.append(blocks[i].data_ptr())
        res = self._store.batch_get_into(keys, data_ptrs, sizes, direct)
        values = []
        for i in range(len(sizes)):
            values.append(tensor_sum(blocks[i]))
        for tensor, size in zip(blocks, sizes):
            if tensor is not None:
                self._store.unregister_buffer(tensor.data_ptr(), size)
        self.cli_return(str([res, values]))

    @result_handler
    def batch_put_from(self, keys: list, sizes: list, media: int, replica_num: int | None = None,
                       preferred_ranks: List[int] | None = None):
        data_ptrs = []
        blocks = []
        if media == 0:
            direct = int(MmcDirect.COPY_H2G.value)
            device = 'cpu'
        else:
            direct = int(MmcDirect.COPY_L2G.value)
            device = 'npu'
        for size in sizes:
            tensor = self.malloc_tensor(mini_block_size=size, device=device)
            if tensor is not None:
                self._store.register_buffer(tensor.data_ptr(), size)
            blocks.append(tensor)
        for i in range(len(sizes)):
            if blocks[i] is None:
                data_ptrs.append(0)
            else:
                data_ptrs.append(blocks[i].data_ptr())

        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks

        res = self._store.batch_put_from(keys, data_ptrs, sizes, direct, rep_conf)
        values = []
        for i in range(len(sizes)):
            values.append(tensor_sum(blocks[i]))
        for tensor, size in zip(blocks, sizes):
            if tensor is not None:
                self._store.unregister_buffer(tensor.data_ptr(), size)
        self.cli_return(str([res, values]))

    @result_handler
    def is_exist(self, key: str):
        res = self._store.is_exist(key)
        self.cli_return(res)

    @result_handler
    def batch_is_exist(self, keys: List[str]):
        res = self._store.batch_is_exist(keys)
        self.cli_return(res)

    @result_handler
    def remove(self, key: str):
        res = self._store.remove(key)
        self.cli_return(res)

    @result_handler
    def remove_batch(self, keys: List[str]):
        res = self._store.remove_batch(keys)
        self.cli_return(res)

    @result_handler
    def remove_all(self):
        res = self._store.remove_all()
        self.cli_return(res)

    @result_handler
    def get_key_info(self, key: str):
        res = self._store.get_key_info(key)
        self.cli_return(res)

    @result_handler
    def batch_get_key_info(self, keys: List[str]):
        res = self._store.batch_get_key_info(keys)
        self.cli_return(res)

    @result_handler
    def put_from_layers(self, key: str, sizes: List[int], media: int, replica_num: int | None = None,
                        preferred_ranks: List[int] | None = None):
        layers_num = len(sizes)
        mini_block_size = max(sizes, default=0)
        if media == 0:
            direct = MmcDirect.COPY_H2G.value
            device = 'cpu'
        else:
            direct = MmcDirect.COPY_L2G.value
            device = 'npu'
        tensor = self.malloc_tensor(layer_num=layers_num, mini_block_size=mini_block_size, device=device)
        # tensor is None in negative cases whose sizes is 0
        if tensor is not None:
            self._store.register_buffer(tensor.data_ptr(), mini_block_size * layers_num)

        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks

        res = self._store.put_from_layers(key,
                                          [] if tensor is None else [layer.data_ptr() for layer in tensor],
                                          sizes,
                                          direct,
                                          rep_conf)
        value = tensor_sum(tensor, sizes)
        if tensor is not None:
            self._store.unregister_buffer(tensor.data_ptr(), mini_block_size * layers_num)
        self.cli_return(str([res, value]))

    @result_handler
    def get_into_layers(self, key: str, sizes: List[int], media: int):
        layers_num = len(sizes)
        mini_block_size = max(sizes, default=0)
        if media == 0:
            direct = MmcDirect.COPY_G2H.value
            device = 'cpu'
        else:
            direct = MmcDirect.COPY_G2L.value
            device = 'npu'
        tensor = self.malloc_tensor(layer_num=layers_num, mini_block_size=mini_block_size, device=device)
        # tensor is None in negative cases whose sizes is 0
        if tensor is not None:
            self._store.register_buffer(tensor.data_ptr(), mini_block_size * layers_num)
        res = self._store.get_into_layers(key,
                                          [] if tensor is None else [layer.data_ptr() for layer in tensor],
                                          sizes,
                                          direct)
        if device == 'npu':
            self.sync_stream()
        value = tensor_sum(tensor, sizes)
        if tensor is not None:
            self._store.unregister_buffer(tensor.data_ptr(), mini_block_size * layers_num)
        self.cli_return(str([res, value]))

    @result_handler
    def batch_put_from_layers(self, keys: List[str], sizes: List[List[int]], media: int, replica_num: int | None = None,
                              preferred_ranks: List[int] | None = None):
        if media == 0:
            direct = MmcDirect.COPY_H2G.value
            device = 'cpu'
        else:
            direct = MmcDirect.COPY_L2G.value
            device = 'npu'
        blocks = []
        for sizes_ in sizes:
            tensor = self.malloc_tensor(layer_num=len(sizes_), mini_block_size=max(sizes_, default=0), device=device)
            # tensor is None in negative cases whose sizes is 0
            if tensor is not None:
                self._store.register_buffer(tensor.data_ptr(), max(sizes_, default=0) * len(sizes_))
            blocks.append(tensor)

        rep_conf = ReplicateConfig()
        if replica_num is not None:
            rep_conf.replicaNum = replica_num
        if preferred_ranks is not None:
            rep_conf.preferredLocalServiceIDs = preferred_ranks

        results = self._store.batch_put_from_layers(
            keys,
            [[] if block is None
             else [layer.data_ptr() for layer in block]
             for block in blocks],
            sizes,
            direct,
            rep_conf
        )
        if device == 'npu':
            self.sync_stream()
        tensor_sums = [tensor_sum(block, sizes_) for block, sizes_ in zip(blocks, sizes)]
        for block, sizes_ in zip(blocks, sizes):
            if block is not None:
                self._store.unregister_buffer(block.data_ptr(), max(sizes_, default=0) * len(sizes_))
        self.cli_return(str([results, tensor_sums]))

    @result_handler
    def batch_get_into_layers(self, keys: List[str], sizes: List[List[int]], media: int):
        if media == 0:
            direct = MmcDirect.COPY_G2H.value
            device = 'cpu'
        else:
            direct = MmcDirect.COPY_G2L.value
            device = 'npu'
        blocks = []
        for sizes_ in sizes:
            tensor = self.malloc_tensor(layer_num=len(sizes_), mini_block_size=max(sizes_, default=0), device=device)
            # tensor is None in negative cases whose sizes is 0
            if tensor is not None:
                self._store.register_buffer(tensor.data_ptr(), max(sizes_, default=0) * len(sizes_))
            blocks.append(tensor)
        results = self._store.batch_get_into_layers(
            keys,
            [[] if block is None
             else [layer.data_ptr() for layer in block]
             for block in blocks],
            sizes,
            direct
        )
        if device == 'npu':
            self.sync_stream()
        tensor_sums = [tensor_sum(block, sizes_) for block, sizes_ in zip(blocks, sizes)]
        for block, sizes_ in zip(blocks, sizes):
            if block is not None:
                self._store.unregister_buffer(block.data_ptr(), max(sizes_, default=0) * len(sizes_))
        self.cli_return(str([results, tensor_sums]))

    @result_handler
    def perf_test_put_from(self, size: int, iter_count: int, medium: str = 'npu', register: bool = True,
                           preferred_rank: int | None = None):
        if medium not in ('cpu', 'npu'):
            raise RuntimeError(f"Invalid device: {medium}")

        tensor = self.malloc_tensor(mini_block_size=size, device=medium)
        direct = MmcDirect.COPY_L2G.value if medium == 'npu' else MmcDirect.COPY_H2G.value

        if register:
            self._store.register_buffer(tensor.data_ptr(), size)

        rep_conf = ReplicateConfig()
        if preferred_rank is not None:
            rep_conf.preferredLocalServiceIDs = [preferred_rank]

        res = 0
        start = time.time()
        for i in range(iter_count):
            key = str(i)
            res = self._store.put_from(key, tensor.data_ptr(), size, direct, rep_conf)
            if res != 0:
                logging.error("put_from failed: %s", res)
                break
        end = time.time()

        if register:
            self._store.unregister_buffer(tensor.data_ptr(), size)

        self.cli_return(str([res, end - start]))

    @result_handler
    def perf_test_get_into(self, size: int, iter_count: int, medium: str = 'npu', register: bool = True):
        if medium not in ('cpu', 'npu'):
            raise RuntimeError(f"Invalid device: {medium}")

        tensor = self.malloc_tensor(mini_block_size=size, device=medium)
        direct = MmcDirect.COPY_G2L.value if medium == 'npu' else MmcDirect.COPY_G2H.value

        if register:
            self._store.register_buffer(tensor.data_ptr(), size)

        res = 0
        start = time.time()
        for i in range(iter_count):
            key = str(i)
            res = self._store.get_into(key, tensor.data_ptr(), size, direct)
            if res != 0:
                logging.error("get_into failed: %s", res)
                break
        end = time.time()

        if register:
            self._store.unregister_buffer(tensor.data_ptr(), size)

        self.cli_return(str([res, end - start]))

    @result_handler
    def perf_test_put_from_layers(self, sizes: List[int], iter_count: int, medium: str = 'npu', register: bool = True,
                                  preferred_rank: int | None = None):
        if medium not in ('cpu', 'npu'):
            raise RuntimeError(f"Invalid device: {medium}")

        layers_num = len(sizes)
        mini_block_size = max(sizes, default=0)
        tensor = self.malloc_tensor(layer_num=layers_num, mini_block_size=mini_block_size, device=medium)
        direct = MmcDirect.COPY_L2G.value if medium == 'npu' else MmcDirect.COPY_H2G.value

        if register:
            self._store.register_buffer(tensor.data_ptr(), mini_block_size * layers_num)

        rep_conf = ReplicateConfig()
        if preferred_rank is not None:
            rep_conf.preferredLocalServiceIDs = [preferred_rank]

        res = 0
        start = time.time()
        for i in range(iter_count):
            key = str(i)
            res = self._store.put_from_layers(key, [layer.data_ptr() for layer in tensor], sizes, direct, rep_conf)
            if res != 0:
                logging.error("put_from_layers failed: %s", res)
                break
        end = time.time()

        if register:
            self._store.unregister_buffer(tensor.data_ptr(), mini_block_size * layers_num)

        self.cli_return(str([res, end - start]))

    @result_handler
    def perf_test_get_into_layers(self, sizes: List[int], iter_count: int, medium: str = 'npu', register: bool = True):
        if medium not in ('cpu', 'npu'):
            raise RuntimeError(f"Invalid device: {medium}")

        layers_num = len(sizes)
        mini_block_size = max(sizes, default=0)
        tensor = self.malloc_tensor(layer_num=layers_num, mini_block_size=mini_block_size, device=medium)
        direct = MmcDirect.COPY_G2L.value if medium == 'npu' else MmcDirect.COPY_G2H.value

        if register:
            self._store.register_buffer(tensor.data_ptr(), mini_block_size * layers_num)

        res = 0
        start = time.time()
        for i in range(iter_count):
            key = str(i)
            res = self._store.get_into_layers(key, [layer.data_ptr() for layer in tensor], sizes, direct)
            if res != 0:
                logging.error("get_into_layers failed: %s", res)
                break
        end = time.time()

        if register:
            self._store.unregister_buffer(tensor.data_ptr(), mini_block_size * layers_num)

        self.cli_return(str([res, end - start]))

    def set_device(self):
        try:
            import acl
        except Exception:
            # adjust k5 env
            return
        acl.init()
        ret = acl.rt.set_device(self._device_id)
        if ret != 0:
            raise RuntimeError(f"acl set device failed, ret={ret}")

    def sync_stream(self):
        import torch_npu
        torch_npu.npu.current_stream().synchronize()

    def malloc_tensor(self, layer_num: int = 1, mini_block_size: int = 1024, device='cpu'):
        if mini_block_size <= 0:
            return None

        if device == "npu":
            return self.malloc_npu_tensor(shape=(layer_num, mini_block_size))
        elif device == "cpu":
            return self.malloc_cpu_tensor(shape=(layer_num, mini_block_size))
        else:
            raise RuntimeError(f"Invalid device: {device}")

    def malloc_npu_tensor(self, shape: Tuple[int]):
        import torch_npu
        self.set_device()
        raw_blocks = torch.randint(
            low=0, high=256,
            size=shape,
            dtype=torch.uint8,
            device=torch.device('npu')
        )
        self.sync_stream()
        return raw_blocks

    def malloc_cpu_tensor(self, shape: Tuple[int], align: int = 4096):
        total_bytes = torch.Size(shape).numel()

        aligned_size = total_bytes + align

        buffer = torch.randint(
            low=0, high=256,
            size=(aligned_size, ),
            dtype=torch.uint8,
        )

        data_ptr = buffer.data_ptr()
        offset = (align - (data_ptr % align)) % align

        aligned_tensor = buffer[offset:offset + total_bytes].view(shape)
        return aligned_tensor


if __name__ == "__main__":
    if len(sys.argv) == 3:
        _, ip_str, port_str = sys.argv
        server = MmcTest(ip_str, port_str)
    elif len(sys.argv) == 4:
        _, ip_str, port_str, device_id_str = sys.argv
        server = MmcTest(ip_str, port_str, int(device_id_str))
    else:
        logging.error("Please input ip and port when starting the process.")
        sys.exit(1)
    logging.info(f"Start app_id: {ip_str}:{port_str}")
    server.start()
