#!/bin/bash
source /usr/local/Ascend/ascend-toolkit/set_env.sh
source /usr/local/memfabric_hybrid/set_env.sh
source /usr/local/memcache_hybrid/set_env.sh
# export MMC_META_CONFIG_PATH=$PWD/mmc-meta.conf

python3 start_meta_service.py
