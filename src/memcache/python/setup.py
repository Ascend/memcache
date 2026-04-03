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

"""Python bindings for memcache_hybrid implemented with pybind11"""

import glob
import os
import platform
import subprocess

from setuptools import find_namespace_packages, setup
from setuptools.dist import Distribution
from wheel.bdist_wheel import bdist_wheel


def check_env_flag(name: str, default: str = "") -> bool:
    return os.getenv(name, default).upper() in ["ON", "1", "YES", "TRUE", "Y"]


# 消除whl压缩包的时间戳差异
os.environ["SOURCE_DATE_EPOCH"] = "0"
# 已经check
current_version = os.getenv("MEMCACHE_VERSION")
is_manylinux = check_env_flag("IS_MANYLINUX", "FALSE")


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""

    def has_ext_modules(self):
        return True


class BuildWheel(bdist_wheel):
    def run(self):
        bdist_wheel.run(self)

        if is_manylinux:
            file = glob.glob(os.path.join(self.dist_dir, "*-linux_*.whl"))[0]
            auditwheel_cmd = [
                "auditwheel",
                "-v",
                "repair",
                "--plat",
                f"manylinux_2_27_{platform.machine()}",
                "--plat",
                f"manylinux_2_28_{platform.machine()}",
                "-w",
                self.dist_dir,
                file,
            ]
            subprocess.check_call(auditwheel_cmd)
            os.remove(file)


with open("../../../README.md", "r", encoding="utf-8") as f:
    long_description = f.read()

setup(
    name="memcache_hybrid",
    version=current_version,
    author="",
    author_email="",
    description="Python bindings for memcache_hybrid implemented with pybind11",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_namespace_packages(exclude=("tests*",)),
    url="https://gitcode.com/Ascend/memcache",
    license="Mulan PSL v2",
    python_requires=">=3.8",
    install_requires=[
        "memfabric_hybrid>=1.1.0",
    ],
    zip_safe=False,
    package_data={
        "memcache_hybrid": ["_pymmc.cpython*.so", "lib/**", "config/**", "VERSION"]
    },
    cmdclass={
        "bdist_wheel": BuildWheel,
    },
    distclass=BinaryDistribution,
)
