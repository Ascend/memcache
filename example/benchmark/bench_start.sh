#!/bin/bash

# rm -rf /root/ascend/log/debug/plog/*
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /usr/local/memcache_hybrid/set_env.sh
source /usr/local/memfabric_hybrid/set_env.sh
export HCOM_SET_LOG_LEVEL=3
# export MMC_LOCAL_CONFIG_PATH=$PWD/mmc-local.conf
# export ASCEND_RT_VISIBLE_DEVICES=0,1,2,3,4,5,6,7

# 解析入参
TEST_CASE="read"
PROCESS_COUNT=1
BATCH_SIZE=16
BLOCK_SIZE=1048576
CALL_COUNT=100
DATA_DIM=1
BACKEND="memcache"
LOCAL_TYPE="npu"

# 参数检查
while getopts 'p:b:s:n:d:e:t:l:' OPT; do
    case $OPT in
    t)
        TEST_CASE="$OPTARG"
        ;;
    p)
        PROCESS_COUNT="$OPTARG"
        ;;
    b)
        BATCH_SIZE="$OPTARG"
        ;;
    s)
        BLOCK_SIZE="$OPTARG"
        ;;
    n)
        CALL_COUNT="$OPTARG"
        ;;
    d)
        DATA_DIM="$OPTARG"
        ;;
    e)
        BACKEND="$OPTARG"
        ;;
    l)
        LOCAL_TYPE="$OPTARG"
        ;;
    ?)
        echo "params: -t <read|write> -p <process count> -b <batch size> -s <block size> -n <call count> -d <data dim, 1|2> -e <memcache|mooncake> -l <npu|cpu>"
        echo 'eg: bash read_start.sh -t read -p 8 -b 32 -s 1048576 -n 1000 -d 1'
        exit 1
        ;;
    esac
done

set -x
python3 run_mutil_process.py ${TEST_CASE} ${PROCESS_COUNT} ${BATCH_SIZE} ${BLOCK_SIZE} ${CALL_COUNT} ${DATA_DIM} ${BACKEND} ${LOCAL_TYPE}
