#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.

import os
import sys


sys.path.append(os.path.dirname(os.path.abspath(__file__)))


from _pymmc import SliceBuffer, DistributedObjectStore


__all__ = ['SliceBuffer', 'DistributedObjectStore']
