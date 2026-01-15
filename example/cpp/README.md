## 代码实现介绍

本样例简单验证了MemCache相关接口

### 如果编译选择CANN依赖

本样例需要在npu环境下编译运行

首先,请在环境上提前安装NPU固件驱动和CANN包([环境安装参考链接](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0000.html))

当前HDK固件驱动需要使用特定版本([社区版HDK下载链接](https://www.hiascend.com/hardware/firmware-drivers/community))

安装完成后需要配置CANN环境变量
([参考安装Toolkit开发套件包的第三步配置环境变量](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0008.html))

运行样例前请先编译安装**memfabric_hybrid的run包**,默认安装路径为/usr/local/,然后source安装路径下的set_env.sh

memfabric_hybrid参考安装命令
git

```bash
bash memfabric_hybrid-1.0.0_linux_aarch64.run # 修改为实际的安装包名
bash memcache_hybrid-1.0.0_linux_aarch64.run  # 修改为实际的安装包名
source /usr/local/memfabric_hybrid/set_env.sh
source /usr/local/memcache_hybrid/set_env.sh
```

## 编译

在当前目录执行如下命令即可

  ```bash
  mkdir build
  cmake . -B build
  make -C build
  ```

## 启动元数据服务

```shell
sed -i 's/true/false/g' /usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
sed -i 's/true/false/g' /usr/local/memcache_hybrid/latest/config/mmc-local.conf;
sed -i 's/5000/5123/g' /usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
sed -i 's/5000/5123/g' /usr/local/memcache_hybrid/latest/config/mmc-local.conf;
sed -i 's/6000/6123/g' /usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
sed -i 's/6000/6123/g' /usr/local/memcache_hybrid/latest/config/mmc-local.conf;
sed -i 's/7000/8100/g' /usr/local/memcache_hybrid/latest/config/mmc-local.conf;
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf;
# 修改/usr/local/memcache_hybrid/latest/config/mmc-local.conf ock.mmc.local_service.hcom_url 为当前环境正确的ip
mmc_meta_service &
```

## 运行

设置大页

```shell
 cat /proc/meminfo
 echo 2048 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
```

编译完成后,会在当前目录生成build/memcache_cpp_test可执行文件
执行方式如下,可交互式的执行put，get等操作命令。

```text
root@localhost:/home/memfabric_hybrid/example/cpp# ./build/memcache_cpp_test
| No running processes found in NPU 0                                                            |

| No running processes found in NPU 1                                                            |

| No running processes found in NPU 2                                                            |

| No running processes found in NPU 3                                                            |

| No running processes found in NPU 4                                                            |

| No running processes found in NPU 5                                                            |

| No running processes found in NPU 6                                                            |

| No running processes found in NPU 7                                                            |

change device id to 7
2025-11-27 15:35:47.415361 info 528 [MMC mmc_configuration.h:336] After alignment 2MB, DRAM size is 134217728
2025-11-27 15:35:47.415405 info 528 [MMC mmc_configuration.h:337] After alignment 2MB, HBM size is 0
2025-11-27 15:35:47.415434 info 528 [MMC mmc_local_service_default.cpp:17] Starting meta service local_service
2025-11-27 15:35:47.415498 info 528 pid[528] [SMEM smem.cpp:29] smem init successfully, library version: (1).0.0, build time: Nov 27 2025 11:40:57, commit: b9b2c0ddbacf4673b500b2f363e6ee44a33673a6
2025-11-27 15:35:47.417449 info 541 pid[528] [AccLink acc_tcp_link_delay_cleanup.h:114] AccDelay cleanup thread thread started
2025-11-27 15:35:47.417715 info 542 pid[528] [AccLink acc_tcp_worker.cpp:152] Worker [name AccWrk, index 0, cpu -1, thread-priority 0, poll-timeout-ms 500] progress thread started
2025-11-27 15:35:47.417952 info 543 pid[528] [AccLink acc_tcp_worker.cpp:152] Worker [name AccWrk, index 1, cpu -1, thread-priority 0, poll-timeout-ms 500] progress thread started
2025-11-27 15:35:47.419397 info 528 pid[528] [AccLink acc_tcp_server_default.cpp:489] Trying to connect to 127.0.0.1:6123
2025-11-27 15:35:47.420075 info 528 pid[528] [AccLink acc_tcp_server_default.cpp:671] Connect to 127.0.0.1:6123 successfully, with ssl disable
2025-11-27 15:35:47.420844 info 528 pid[528] [SMEM smem_bm_entry_manager.cpp:133] Success to auto ranking rankId: 0 localIp:  deviceId: 7
2025-11-27 15:35:47.420887 info 528 pid[528] [SMEM smem_bm_entry_manager.cpp:70] initialize store(tcp://127.0.0.1:6123) world size(16) device(7) OK.
2025-11-27 15:35:47.421173 info 528 pid[528] [HYBM hybm_gva_version.cpp:188] Driver version V4 found
2025-11-27 15:35:47.500721 info 528 pid[528] [HYBM dl_hccp_api.cpp:124] LoadLibrary for DlHccpApi success
2025-11-27 15:35:47.506916 info 528 pid[528] [HYBM hybm_functions.h:33] Not set rt visible env return deviceId: 7
2025-11-27 15:35:47.772704 info 528 pid[528] [HYBM devmm_svm_gva.cpp:485] gva alloc heap. (size=0x40000000)
2025-11-27 15:35:47.772861 info 528 pid[528] [HYBM hybm_entry.cpp:105] hybm init successfully, library version: (1).0.0, build time: Nov 27 2025 11:41:02, commit: b9b2c0ddbacf4673b500b2f363e6ee44a33673a6, deviceId: 7
2025-11-27 15:35:47.772889 info 528 pid[528] [SMEM smem_bm.cpp:93] smem_bm_init success.  config_ip: tcp://127.0.0.1:6123
2025-11-27 15:35:47.873354 info 528 pid[528] [HYBM hybm_functions.h:33] Not set rt visible env return deviceId: 7
2025-11-27 15:35:47.875976 info 528 pid[528] [HYBM host_hcom_transport_manager.cpp:500] hcom base port: 8000, hcom auto port with rank: 8000
2025-11-27 15:35:47.876625 INFO [HCOM service_periodic_manager.cpp:163] PeriodicManager for timeout [name: hybm_hcom_service, index: 0] working thread start
2025-11-27 15:35:47.877743 INFO [HCOM net_mem_pool_fixed.cpp:143] Fixed size memory pool ServiceContextTimer-hybm_hcom_service allocated 1MB memory from os, total block size 1048576 and split to 16384 min block with size 64 which took 1109us, current free min block is 16384
2025-11-27 15:35:47.880277 INFO [HCOM verbs_api_dl.cpp:95] Success to load ibverbs
2025-11-27 15:35:47.880329 INFO [HCOM hcom.cpp:390] hcom build commit: 43c8395eb31c0d6f95b273e7076581bfccd14d78
2025-11-27 15:35:47.880347 INFO [HCOM hcom.cpp:394] Hcom version :1.0.0
2025-11-27 15:35:47.880380 INFO [HCOM hcom.cpp:423] Limit of open files is 1048576, please check if it is big enough
2025-11-27 15:35:47.880466 INFO [HCOM net_rdma_driver.cpp:39] RDMA driver try to initialize with UBSHcomNetDriverOptions mode: 0, send/receive-mr-seg-count: 8192, send/receive-mr-seg-size: 8192, device-mask: 61.47.1.122/32, cq-size 2048, max-post-send: 64, pre-post-receive-count: 256, polling-batch-size: 4, qp-send-queue-size: 512, qp-receive-queue-size: 512, worker-groups: 1, worker-groups-cpu-set: na, start-workers: 0, tls-enabled: 0, oob-type: Tcp, lb-policy: RR
2025-11-27 15:35:47.906340 INFO [HCOM net_rdma_driver.cpp:173] RDMADeviceHelper device info, devices: count 8, [7,hns_3,0] [6,hns_2,1] [5,hns_1,0] [4,hns_0,0] [3,mlx5_3,0] [2,mlx5_2,0] [1,mlx5_1,0] [0,mlx5_0,1] , gidTable: count 8, [deviceName hns_0, [4,0,IB/RoCE v1] [4,1,RoCE v2] ] [deviceName hns_3, [7,0,IB/RoCE v1] [7,1,RoCE v2] ] [deviceName hns_2, [6,0,IB/RoCE v1] [6,1,RoCE v2] [6,2,IB/RoCE v1] [6,3,RoCE v2] ] [deviceName hns_1, [5,0,IB/RoCE v1] [5,1,RoCE v2] ] [deviceName mlx5_2, [2,0,IB/RoCE v1] [2,1,RoCE v2] ] [deviceName mlx5_1, [1,0,IB/RoCE v1] [1,1,RoCE v2] ] [deviceName mlx5_3, [3,0,IB/RoCE v1] [3,1,RoCE v2] ] [deviceName mlx5_0, [0,0,IB/RoCE v1] [0,1,RoCE v2] [0,2,IB/RoCE v1] [0,3,RoCE v2] ]
2025-11-27 15:35:47.909130 INFO [HCOM rdma_verbs_wrapper_ctx.cpp:62] Device info: fw_ver 0.276.027 ,max_qp 1048576 ,max_qp_wr 32768 ,max_sge 255 ,adapter max_cqe 16 ,max_cq 1048576 ,max_cqe 4194304
2025-11-27 15:35:47.986290 INFO [HCOM net_mem_pool_fixed.cpp:143] Fixed size memory pool hybm_hcom_service allocated 1MB memory from os, total block size 1048576 and split to 16384 min block with size 64 which took 433us, current free min block is 16384
2025-11-27 15:35:47.986705 INFO [HCOM net_mem_pool_fixed.cpp:143] Fixed size memory pool hybm_hcom_service allocated 1MB memory from os, total block size 1048576 and split to 2048 min block with size 512 which took 380us, current free min block is 2048
2025-11-27 15:35:47.987194 INFO [HCOM rdma_worker_core.cpp:390] RDMAWorker [name: hybm_hcom_service, index: 0-0-0], cpuId: -1, cq count: 2048, polling batch size: 4, more options type: sender&receiver, mode: busy_polling, cq size: 2048, max post send: 64, pre-post receive size: 256, poll batch size 4, cpu id: -1, qp send queue: 512, qp receive queue: 512, dontStartWorkers: 0] working thread started
2025-11-27 15:35:47.987412 INFO [HCOM net_execution_service.cpp:131] Thread is started for executor service <OOBTcpConnHdl0> cpuId -1
2025-11-27 15:35:47.987486 INFO [HCOM net_execution_service.cpp:131] Thread is started for executor service <OOBTcpConnHdl1> cpuId -1
2025-11-27 15:35:47.987576 INFO [HCOM net_oob.cpp:512] OOB server accept thread for 61.47.1.122:8000 started, load balancer name: hybm_hcom_service, policy: RR, choose-able-count: all, worker-groups: 1
2025-11-27 15:35:48.2659 INFO [HCOM net_heartbeat.cpp:101] Heartbeat thread for driver hybm_hcom_service, HCOMHb0 started, idle time 60
2025-11-27 15:35:48.2820 INFO [HCOM net_rdma_driver_oob.cpp:337] Rdma event monitor thread for driver hybm_hcom_service started
2025-11-27 15:35:48.038436 info 528 pid[528] [HYBM hybm_data_operator.h:58] update type 2 gva: 35184372088832, space:134217728, rankCnt:16
2025-11-27 15:35:48.071270 info 528 pid[528] [SMEM smem_bm_entry.cpp:132] do join func, local_rk: 0 receive_rk: 0, rank size is: 1
2025-11-27 15:35:48.072195 info 528 pid[528] [SMEM smem_net_group_engine.cpp:304] allGather successfully, key: SMEM_BM_(0)_D_1_0_GA, rank: 0, size: 1, timeCostUs: total(884) append(233) getStatus(218) getData(211)
2025-11-27 15:35:48.073096 info 528 pid[528] [SMEM smem_net_group_engine.cpp:304] allGather successfully, key: SMEM_BM_(0)_D_1_1_GA, rank: 0, size: 1, timeCostUs: total(882) append(210) getStatus(218) getData(242)
2025-11-27 15:35:48.073775 info 528 pid[528] [SMEM smem_net_group_engine.cpp:147] groupBarrier successfully, key: SMEM_BM_(0)_D_1_2_BW, size: 1, timeCostUs: total(639) add(216) getStatus(207)
2025-11-27 15:35:48.074622 info 528 pid[528] [SMEM smem_net_group_engine.cpp:304] allGather successfully, key: SMEM_BM_(0)_D_1_3_GA, rank: 0, size: 1, timeCostUs: total(831) append(209) getStatus(204) getData(207)
2025-11-27 15:35:48.075303 info 528 pid[528] [SMEM smem_net_group_engine.cpp:147] groupBarrier successfully, key: SMEM_BM_(0)_D_1_4_BW, size: 1, timeCostUs: total(659) add(237) getStatus(183)
2025-11-27 15:35:48.075697 info 528 [MMC mmc_bm_proxy.cpp:72] init bm success, rank:0, worldSize:16, hbm{0}, dram{134217728}
2025-11-27 15:35:48.075952 info 528 [MMC mmc_net_ctx_store.h:60] Initialized context store, flatten capacity 65536, versionAndSeqMask 4194303, seqNoMask 65535, seqNoAndVersionIndex 1
2025-11-27 15:35:48.076188 info 561 pid[528] [AccLink acc_tcp_link_delay_cleanup.h:114] AccDelay cleanup thread thread started
2025-11-27 15:35:48.076353 info 562 pid[528] [AccLink acc_tcp_worker.cpp:152] Worker [name AccWrk, index 0, cpu -1, thread-priority 0, poll-timeout-ms 500] progress thread started
2025-11-27 15:35:48.076504 info 563 pid[528] [AccLink acc_tcp_worker.cpp:152] Worker [name AccWrk, index 1, cpu -1, thread-priority 0, poll-timeout-ms 500] progress thread started
2025-11-27 15:35:48.077813 info 528 [MMC mmc_meta_net_client.cpp:59] initialize meta net server success [MetaClientCommon]
2025-11-27 15:35:48.078498 info 528 pid[528] [AccLink acc_tcp_server_default.cpp:489] Trying to connect to 127.0.0.1:5123
2025-11-27 15:35:48.079058 info 528 pid[528] [AccLink acc_tcp_server_default.cpp:671] Connect to 127.0.0.1:5123 successfully, with ssl disable
2025-11-27 15:35:48.079079 info 528 [MMC mmc_local_service_default.cpp:138] mmc local register capacity:134217728, type:1
2025-11-27 15:35:48.079617 info 528 [MMC mmc_local_service_default.cpp:164] bm register succeed, bmRankId=0, type num=1
2025-11-27 15:35:48.079628 info 528 [MMC mmc_local_service_default.cpp:66] Started LocalService (local_service) server tcp://127.0.0.1:5123
2025-11-27 15:35:48.079694 info 528 [MMC mmc_client_default.cpp:28] Starting client mmc_client
Initialized with put hash value: 18356023244072079545
Checking if key exists: test1
Initial PutFrom result: 0
2025-11-27 15:35:48.104746 error 528 [MMC mmcache_store.cpp:386] Input vector sizes mismatch: keys=4, buffers=1, sizes=1
BatchPutFrom result: -1
2025-11-27 15:35:48.105174 error 528 [MMC mmc_client.cpp:142] mmc_client remove key kBatchPutFrom failed!, error code -3102
Retrieved data hash for key test1: 18356023244072079545
Store operations ready. Type 'exit' to quit.
```

### 手动执行put，get，remove

```text
put  1^H
1
PutFrom result: 0, key: , replicas: 1
put k 1
PutFrom result: 0, key: k, replicas: 1
get k
Checking existence for key: k
GetInto result: 0
Retrieved data hash for key k: 18356023244072079545
remove k
Remove result: 0 for key: k
exit
Exiting program.

```