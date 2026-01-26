#!/bin/bash
rm -rf /var/log/memfabric_hybrid/*
rm -rf /root/ascend/log/debug/plog/*
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /usr/local/memcache_hybrid/set_env.sh
source /usr/local/memfabric_hybrid/set_env.sh
export MMC_LOCAL_CONFIG_PATH=$PWD/mmc-write.conf
#export ASCEND_RT_VISIBLE_DEVICES=0,1,2,3,4,5,6,7
# nohup python3 write_mutil_process.py &> write.log 2>&1 &
python3 write_mutil_process.py