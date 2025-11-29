#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 1.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

BUILD_MODE=${1:-RELEASE}
BUILD_TESTS=${2:-OFF}
BUILD_OPEN_ABI=${3:-ON}
BUILD_PYTHON=${4:-ON}
ENABLE_PTRACER=${5:-ON}
USE_CANN=${6:-ON}
BUILD_COMPILER=${7:-gcc}
BUILD_PACKAGE=${8:-ON}

readonly SCRIPT_FULL_PATH=$(dirname $(readlink -f "$0"))
readonly PROJECT_FULL_PATH=$(dirname "$SCRIPT_FULL_PATH")

if [ "${BUILD_TESTS}" == "ON" ]; then
  readonly MOCKCPP_PATH="$PROJECT_FULL_PATH/test/3rdparty/mockcpp"
  readonly TEST_3RD_PATCH_PATH="$PROJECT_FULL_PATH/test/3rdparty/patch"
  dos2unix "$MOCKCPP_PATH/include/mockcpp/JmpCode.h"
  dos2unix "$MOCKCPP_PATH/include/mockcpp/mockcpp.h"
  dos2unix "$MOCKCPP_PATH/src/JmpCode.cpp"
  dos2unix "$MOCKCPP_PATH/src/JmpCodeArch.h"
  dos2unix "$MOCKCPP_PATH/src/JmpCodeX64.h"
  dos2unix "$MOCKCPP_PATH/src/JmpCodeX86.h"
  dos2unix "$MOCKCPP_PATH/src/JmpOnlyApiHook.cpp"
  dos2unix "$MOCKCPP_PATH/src/UnixCodeModifier.cpp"
  dos2unix $TEST_3RD_PATCH_PATH/*.patch
fi

set -e
readonly ROOT_PATH=$(dirname $(readlink -f "$0"))
CURRENT_DIR=$(pwd)

cd ${ROOT_PATH}/..
PROJ_DIR=$(pwd)

bash script/gen_last_git_commit.sh

rm -rf ./build ./output

mkdir build/
cmake -DCMAKE_BUILD_TYPE="${BUILD_MODE}" -DBUILD_COMPILER="${BUILD_COMPILER}" -DBUILD_TESTS="${BUILD_TESTS}" -DBUILD_OPEN_ABI="${BUILD_OPEN_ABI}" -DBUILD_PYTHON="${BUILD_PYTHON}" -DENABLE_PTRACER="${ENABLE_PTRACER}" -DUSE_CANN="${USE_CANN}" -S . -B build/
make install -j32 -C build/

if [ "${BUILD_PYTHON}" != "ON" ]; then
    echo "========= skip build python ============"
        cd ${CURRENT_DIR}
        exit 0
fi

FABRIC_PROJ_DIR=${PROJ_DIR}/3rdparty/memfabric_hybrid

mkdir -p "${PROJ_DIR}/src/memcache/python/memcache_hybrid/lib"
\cp -v "${FABRIC_PROJ_DIR}/output/hybm/lib64/libmf_hybm_core.so" "${PROJ_DIR}/src/memcache/python/memcache_hybrid/lib"
\cp -v "${FABRIC_PROJ_DIR}/output/smem/lib64/libmf_smem.so" "${PROJ_DIR}/src/memcache/python/memcache_hybrid/lib"
\cp -v "${PROJ_DIR}/output/memcache/lib64/libmf_memcache.so" "${PROJ_DIR}/src/memcache/python/memcache_hybrid/lib"

GIT_COMMIT=`git rev-parse HEAD` || true
{
  echo "mf version info:"
  echo "mf version: 1.0.0"
  echo "git: ${GIT_COMMIT}"
} > VERSION

cp VERSION "${PROJ_DIR}/src/memcache/python/memcache_hybrid/"
rm -f VERSION

readonly BACK_PATH_EVN=$PATH

# 如果 PYTHON_HOME 不存在，则设置默认值
if [ -z "$PYTHON_HOME" ]; then
    # 定义要检查的目录路径
    CHECK_DIR="/usr/local/python3.11"
    # 判断目录是否存在
    if [ -d "$CHECK_DIR" ]; then
        export PYTHON_HOME="$CHECK_DIR"
    else
        export PYTHON_HOME="/usr/local/"
    fi
    echo "Not set PYTHON_HOME，and use $PYTHON_HOME"
fi

export LD_LIBRARY_PATH=$PYTHON_HOME/lib:$LD_LIBRARY_PATH
export PATH=$PYTHON_HOME/bin:$BACK_PATH_EVN
export CMAKE_PREFIX_PATH=$PYTHON_HOME

python_path_list=("/opt/buildtools/python-3.8.5" "/opt/buildtools/python-3.9.11" "/opt/buildtools/python-3.10.2" "/opt/buildtools/python-3.11.4")
for python_path in "${python_path_list[@]}"
do
    if [ -n "${multiple_python}" ]; then
        export PYTHON_HOME=${python_path}
        export CMAKE_PREFIX_PATH=$PYTHON_HOME
        export LD_LIBRARY_PATH=$PYTHON_HOME/lib
        export PATH=$PYTHON_HOME/bin:$BACK_PATH_EVN

        rm -rf build/
        mkdir -p build/
        cmake -DCMAKE_BUILD_TYPE="${BUILD_MODE}" -DBUILD_OPEN_ABI="${BUILD_OPEN_ABI}" -S . -B build/
        make -j5 -C build
    fi

    cd "${PROJ_DIR}"
    rm -f "${PROJ_DIR}"/src/memcache/python/memcache_hybrid/_pymmc.cpython*.so
    \cp -v "${PROJ_DIR}"/build/src/memcache/csrc/python_wrapper/_pymmc.cpython*.so "${PROJ_DIR}"/src/memcache/python/memcache_hybrid
    cd "${PROJ_DIR}/src/memcache/python"
    rm -rf build memcache_hybrid.egg-info
    python3 setup.py bdist_wheel

    if [ -z "${multiple_python}" ];then
        break
    fi
done

mkdir -p "${PROJ_DIR}/output/memcache/wheel"
cp "${PROJ_DIR}"/src/memcache/python/dist/*.whl "${PROJ_DIR}/output/memcache/wheel"
rm -rf "${PROJ_DIR}"/src/memcache/python/dist

if [ "${BUILD_PACKAGE}" == "ON" ]; then
    bash "${PROJ_DIR}"/script/run_pkg_maker/make_run.sh RELEASE ON
    rm -rf "${PROJ_DIR}"/package
    mkdir -p "${PROJ_DIR}"/package
    cp "${PROJ_DIR}"/output/memcache/wheel/*.whl "${PROJ_DIR}/package"
    cp "${PROJ_DIR}"/output/*.run "${PROJ_DIR}"/package
fi

cd ${CURRENT_DIR}