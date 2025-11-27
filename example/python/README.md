## 代码实现介绍

本样例简单验证了`big memory`相关`python`接口

本样例需要在npu环境下编译运行

首先,请在环境上提前安装NPU固件驱动和CANN包([环境安装参考链接](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0000.html))

HDK固件驱动需要使用**25.0.RC1**
及以上版本([社区版HDK下载链接](https://www.hiascend.com/hardware/firmware-drivers/community))

安装完成后需要配置CANN环境变量
([参考安装Toolkit开发套件包的第三步配置环境变量](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0008.html))

运行样例前请先编译安装**memfabric_hybrid的run包**,默认安装路径为/usr/local/,然后source安装路径下的set_env.sh

memfabric_hybrid参考安装命令

```bash
bash memfabric_hybrid-1.0.0_linux_aarch64.run
source /usr/local/memfabric_hybrid/set_env.sh
```

## 启动元数据服务

```shell
sed -i 's/true/false/g' /usr/local/memfabric_hybrid/latest/config/mmc-meta.conf;
sed -i 's/true/false/g' /usr/local/memfabric_hybrid/latest/config/mmc-local.conf;
sed -i 's/5000/5123/g' /usr/local/memfabric_hybrid/latest/config/mmc-meta.conf;
sed -i 's/5000/5123/g' /usr/local/memfabric_hybrid/latest/config/mmc-local.conf;
sed -i 's/6000/6123/g' /usr/local/memfabric_hybrid/latest/config/mmc-meta.conf;
sed -i 's/6000/6123/g' /usr/local/memfabric_hybrid/latest/config/mmc-local.conf;
sed -i 's/7000/8000/g' /usr/local/memfabric_hybrid/latest/config/mmc-local.conf;
export MMC_META_CONFIG_PATH=/usr/local/memfabric_hybrid/latest/config/mmc-meta.conf;
export MMC_LOCAL_CONFIG_PATH=/usr/local/memfabric_hybrid/latest/config/mmc-local.conf;
# 修改/usr/local/memfabric_hybrid/latest/config/mmc-local.conf ock.mmc.local_service.hcom_url 
mmc_meta_service &
```

## 配置大页

```shell
 cat /proc/meminfo
 echo 2048 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
```

## 执行脚本

选择脚本，直接执行，比如

```shell
python python/test_mmc_demo.py
```