## 代码实现介绍

本样例简单验证了`memcache_hybrid`相关`python`接口

本样例需要在npu环境下编译运行

### 如果编译选择CANN依赖

首先,请在环境上提前安装NPU固件驱动和CANN包([环境安装参考链接](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0000.html))

HDK固件驱动需要使用**25.0.RC1**
及以上版本([社区版HDK下载链接](https://www.hiascend.com/hardware/firmware-drivers/community))

安装完成后需要配置CANN环境变量
([参考安装Toolkit开发套件包的第三步配置环境变量](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0008.html))

运行样例前请先编译安装 [**memfabric_hybrid的run包**](https://gitcode.com/Ascend/memfabric_hybrid/blob/master/doc/installation.md)，默认安装路径为/usr/local/,然后source安装路径下的set_env.sh

memfabric_hybrid参考安装命令

```bash
bash memfabric_hybrid-1.0.0_linux_aarch64.run # 修改为实际的安装包名
bash memcache_hybrid-1.0.0_linux_aarch64.run  # 修改为实际的安装包名
source /usr/local/memfabric_hybrid/set_env.sh
source /usr/local/memcache_hybrid/set_env.sh
```

## 启动元数据服务

```shell
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
python3
>>> from memcache_hybrid import MetaService
>>> MetaService.main()

```

## 配置大页
注：仅device rdma/host rdma等protocol需要设置
```shell
 cat /proc/meminfo
 echo 2048 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
```

## 执行脚本

选择脚本，直接执行，以test_mmc_start_meta_service_and_simple_test.py为例，会在一个进程里面启动meta服务和localService并且完成put，get等测试（不需要前面步骤单独的启动meta服务进程）

```shell
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf;
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf;

python python/test_mmc_start_meta_service_and_simple_test.py
```