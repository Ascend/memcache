# 构建

## 环境准备

#### 编译工具建议版本

- OS: Ubuntu 22.04 LTS+
- cmake: 3.20.x
- gcc: 11.4+
- python 3.11.10
- pybind11 2.10.3

## 编译

1. 下载代码

```
git clone https://gitcode.com/Ascend/memcache
cd memcache
git clean -xdf
git reset --hard
```

2. 拉取第三方库

```
git submodule update --recursive --init
git submodule update --remote 3rdparty/memfabric_hybrid
```

3. 编译

```
bash script/build_and_pack_run.sh --build_mode RELEASE --build_python ON

```

- build_and_pack_run.sh支持3个参数，分别是<build_mode> <build_python> <build_test>
- build_mode: 编译类型，可填RELEASE或DEBUG，默认RELEASE
- build_python: 是否编译python的whl包，可填ON或OFF，默认ON
- build_test: 是否打包测试工具，可填ON或OFF，默认OFF

4. ut运行

支持直接执行如下脚本编译并运行ut

```
bash script/run_ut.sh
```

## 安装使用

MemCache将所有特性集成到run包中供用户使用，run包格式为 ```memcache_hybrid-${version}_${os}_${arch}.run```

其中，versin表示MemCache的版本；os表示操作系统，如linux；arch表示架构，如x86或aarch64

注意：MemCache运行时依赖MemFabric，需要先参考[MemFabric使用指导](https://gitcode.com/Ascend/memfabric_hybrid/blob/master/doc/build.md)完成MemFabric的安装。

### run包安装

run包的默认安装根路径为 /usr/local/

安装完成后需要source安装路径下的memcache_hybrid/set_env.sh

参考安装命令如下

```bash
bash memcache_hybrid-1.0.0_linux_aarch64.run
source /usr/local/memcache_hybrid/set_env.sh
```

如果想要自定义安装路径，可以添加--install-path参数

```bash
bash memcache_hybrid-1.0.0_linux_aarch64.run --install-path=${your path}
```

安装的run包可以通过如下命令查看版本（此处以默认安装路径为例）

```
root@localhost:/# cat /usr/local/memcache_hybrid/latest/version.info
Version:1.0.0
Platform:aarch64
Kernel:linux
CommitId:91b4ed50d42874dc21fa71917ab84e6824b2f7b7
```

安装的python包可以通过如下命令查看版本

```text
root@localhost:# pip show memcache_hybrid
Name: memcache_hybrid
Version: 1.0.0
Summary: python api for memcache
Home-page: https://gitcode.com/Ascend/memcache
Author:
Author-email:
License: Apache License Version 2.0
Location: /usr/local/lib/python3.11/site-packages
Requires:
Required-by:
```

在安装过程中，会默认尝试安装适配当前环境的MemCache的whl包，如果未安装，则在使用python接口前需要用户手动安装(安装包路径参考上面目录结构中的whl包路径)

## 运行服务
### MetaService

 👆 NOTE：运行前需要根据 [MetaService配置项](./memcache_config.md) 对配置文件 /usr/local/memcache_hybrid/latest/config/mmc-meta.conf 进行相关配置，该文件可以放置到任意可访问的路径

* **python形式**：*以下均以python311版本whl包（memcache_hybrid-1.0.0-cp311-cp311-linux_aarch64.whl）为例*
```
1、安装whl包（如在安装run包过程中，已安装whl包，此步骤可省略）
pip install memcache_hybrid-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf

3、进入python控制台或者编写python脚本如下即可拉起进程：
from memcache_hybrid import MetaService
MetaService.main()
```
* **bin形式**：
```
安装run包即完成了相应二进制的部署
1、设置环境变量
source /usr/local/memcache_hybrid/set_env.sh
source /usr/local/memfabric_hybrid/set_env.sh
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf

2、拉起二进制
/usr/local/memcache_hybrid/latest/aarch64-linux/bin/mmc_meta_service
```

### LocalService
 👆 NOTE：运行前需要根据 [LocalService配置项](./memcache_config.md) 对配置文件 /usr/local/memcache_hybrid/latest/config/mmc-local.conf 进行相关配置，该文件可以放置到任意可访问的路径
* **whl（python）**：
```
1、安装whl包（如在安装run包过程中，已安装whl包，此步骤可省略）
pip install memcache_hybrid-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf

3、通过MemCache提供的接口初始化客户端并拉起localservice，执行数据写入、查询、获取、删除等，下面的脚本时一个示例：
python3 test_mmc_demo.py
```

* **so（C++）**：
```
1、导入头文件
#include "mmcache.h"

2、将 libmf_memcache.so 路径加入到 LD_LIBRARY_PATH 环境变量
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${libmf_memcache.so path}

注：如果其所在路径已经在 PATH 或 LD_LIBRARY_PATH，则无需此步骤

3、设置配置文件环境变量
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf

4、调用相关API

```