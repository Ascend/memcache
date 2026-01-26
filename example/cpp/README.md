## 代码实现介绍

本样例简单验证了MemCache相关接口

### 如果编译选择CANN依赖

本样例需要在npu环境下编译运行

首先,请在环境上提前安装NPU固件驱动和CANN包([环境安装参考链接](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0000.html))

当前HDK固件驱动需要使用特定版本([社区版HDK下载链接](https://www.hiascend.com/hardware/firmware-drivers/community))

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

## 编译样例程序

在当前目录(memcache/example/cpp)执行如下命令即可

  ```bash
  mkdir build
  cmake . -B build
  make -C build
  ```

## 启动元数据服务

```shell
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf;
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf;
mmc_meta_service &
```

## 运行

设置大页（注：仅device rdma/host rdma等protocol需要设置）

```shell
 cat /proc/meminfo
 echo 2048 | sudo tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
```

编译完成后,会在当前目录生成build/memcache_cpp_test可执行文件
执行方式如下,可交互式的执行put，get等操作命令。

```text
root@localhost:/home/memcache_hybrid/example/cpp# ./build/memcache_cpp_test
```

### 手动执行put，get，remove

```text
put  1
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