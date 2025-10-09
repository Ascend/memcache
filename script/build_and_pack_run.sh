#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.

set -e
readonly ROOT_PATH=$(dirname $(readlink -f "$0"))
CURRENT_DIR=$(pwd)

BUILD_MODE=$1
if [ -z "$BUILD_MODE" ]; then
    BUILD_MODE="RELEASE"
fi

BUILD_PYTHON=$2
if [ -z "$BUILD_PYTHON" ]; then
    BUILD_PYTHON="ON"
fi

cd ${ROOT_PATH}
SPDLOG_SRC=${ROOT_PATH}/../3rdparty/log/spdlog/src
if [ ! -d "${SPDLOG_SRC}" ]; then
    git submodule init
    git submodule update
fi

bash build.sh "${BUILD_MODE}" OFF OFF "${BUILD_PYTHON}"

bash run_pkg_maker/make_run.sh "${BUILD_PYTHON}"

cd ${CURRENT_DIR}