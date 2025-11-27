# 构建

## 环境准备

#### 编译工具建议版本

- OS: Ubuntu 22.04 LTS+
- cmake: 3.20.x
- gcc: 11.4+
- python 3.11.10
- pybind11 2.10.3

#### (编译选择CANN依赖时)需要NPU固件驱动和CANN包

run包只能安装到npu环境上，且依赖于NPU固件驱动和CANN包，具体版本依赖详见下面的软件硬件配套说明

请在环境上提前安装NPU固件驱动和CANN包([环境安装参考链接](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0000.html))

安装完成后需要配置CANN环境变量([参考安装Toolkit开发套件包的第三步配置环境变量](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0008.html))

## 编译

memcache_hybrid编译不依赖CANN和HDK.

1. 下载代码

```
git clone https://gitee.com/ascend/memfabric_hybrid.git
cd memfabric_hybrid
git checkout br_opensource_memcache
git clean -xdf
git reset --hard
```

2. 拉取第三方库

```
git submodule update --recursive --init
```

3. 编译

```
bash script/build_and_pack_run.sh --build_mode RELEASE --build_python ON --use_cann ON

```

- build_and_pack_run.sh支持3个参数,按顺序分别是<build_mode> <build_python> <use_cann>
- build_mode:编译类型,可填RELEASE或DEBUG
- build_python:是否编译python的whl包,可填ON或OFF
- use_cann: 编译需要CANN dependency。
- 不填入参数情况下,默认执行build_and_pack_run.sh RELEASE ON ON

4. ut运行

支持直接执行如下脚本编译并运行ut

```
bash script/run_ut.sh
```

## 安装使用

memcache_hybrid将所有特性集成到run包中供用户使用，run包格式为 ```memcache_hybrid-${version}_${os}_${arch}.run```

其中，versin表示memcache_hybrid的版本；os表示操作系统,如linux；arch表示架构，如x86或aarch64

### run包安装

run包的默认安装根路径为 /usr/local/

安装完成后需要source安装路径下的memfabric_hybrid/set_env.sh

参考安装命令如下

```bash
bash memcache_hybrid-1.0.0_linux_aarch64.run
source /usr/local/memfabric_hybrid/set_env.sh
```

如果想要自定义安装路径，可以添加--install-path参数

```bash
bash memcache_hybrid-1.0.0_linux_aarch64.run --install-path=${your path}
```

默认安装完成后目录结构如下

```

/usr/local/memfabric_hybrid
├── 1.0.0
│   ├── aarch64-linux
│   │   ├── bin
│   │   │   └── mmc_meta_service
│   │   ├── include
│   │   │   ├── driver
│   │   │   │   └── hybm_gvm_user.h
│   │   │   ├── hybm
│   │   │   │   ├── hybm.h
│   │   │   │   ├── hybm_big_mem.h
│   │   │   │   ├── hybm_data_op.h
│   │   │   │   └── hybm_def.h
│   │   │   ├── memcache
│   │   │   │   ├── cpp
│   │   │   │   │   └── mmcache.h
│   │   │   │   ├── mmc.h
│   │   │   │   ├── mmc_client.h
│   │   │   │   ├── mmc_def.h
│   │   │   │   └── mmc_service.h
│   │   │   └── smem
│   │   │       ├── device
│   │   │       │   ├── smem_shm_aicore_base_api.h
│   │   │       │   ├── smem_shm_aicore_base_copy.h
│   │   │       │   ├── smem_shm_aicore_base_define.h
│   │   │       │   ├── smem_shm_aicore_base_meta.h
│   │   │       │   └── smem_shm_aicore_base_rdma.h
│   │   │       └── host
│   │   │           ├── smem.h
│   │   │           ├── smem_bm.h
│   │   │           ├── smem_bm_def.h
│   │   │           ├── smem_shm.h
│   │   │           ├── smem_shm_def.h
│   │   │           ├── smem_trans.h
│   │   │           └── smem_trans_def.h
│   │   ├── lib64
│   │   │   ├── _pymmc.cpython-311-aarch64-linux-gnu.so
│   │   │   ├── libhybm_gvm.a
│   │   │   ├── libhybm_gvm.so
│   │   │   ├── libmf_hybm_core.so
│   │   │   ├── libmf_memcache.so
│   │   │   └── libmf_smem.so
│   │   ├── logs
│   │   │   ├── mmc-meta-audit.log
│   │   │   └── mmc-meta.log
│   │   ├── script
│   │   │   ├── certs
│   │   │   │   ├── generate_client_cert.py
│   │   │   │   ├── generate_crl.py
│   │   │   │   ├── generate_root_cert.py
│   │   │   │   └── generate_server_cert.py
│   │   │   ├── ha
│   │   │   │   └── test-mmc-meta-ha.py
│   │   │   ├── k8s_deploy
│   │   │   │   ├── account-role-demo.yaml
│   │   │   │   ├── local-pods-demo.yaml
│   │   │   │   ├── meta-cluster-ip-demo.yaml
│   │   │   │   ├── meta-lease-lock-demo.yaml
│   │   │   │   └── meta-pods-demo.yaml
│   │   │   └── mock_server
│   │   │       ├── server.py
│   │   │       └── smem_bm_server.py
│   │   └── wheel
│   │       ├── memcache-1.0.0-cp311-cp311-linux_aarch64.whl
│   │       ├── mf_adapter-1.0.0-cp311-cp311-linux_aarch64.whl
│   │       └── mf_smem-1.0.0-cp311-cp311-linux_aarch64.whl
│   ├── config
│   │   ├── mmc-local.conf
│   │   └── mmc-meta.conf
│   ├── uninstall.sh
│   └── version.info
├── latest -> 1.0.0
└── set_env.sh


```

安装的python包如下

```text

root@localhost:/# pip show mf_smem
Name: mf_smem
Version: 1.0.0
Summary: python api for smem
Home-page: https://gitee.com/ascend/memfabric_hybrid
Author:
Author-email:
License: Apache License Version 2.0
Location: /usr/local/lib/python3.11/site-packages
Requires:
Required-by:

root@localhost:/# pip show mf_adapter
Name: mf_adapter
Version: 1.0.0
Summary: python api for mf_adapter
Home-page: https://gitee.com/ascend/memfabric_hybrid
Author:
Author-email:
License: Apache License Version 2.0
Location: /usr/local/lib/python3.11/site-packages
Requires:
Required-by:

root@localhost:# pip show memcache
Name: memcache
Version: 1.0.0
Summary: python api for memcache
Home-page: https://gitee.com/ascend/memfabric_hybrid
Author:
Author-email:
License: Apache License Version 2.0
Location: /usr/local/lib/python3.11/site-packages
Requires:
Required-by:
```

python 包文件内容

```text

root@localhost:/# tree /usr/local/lib/python3.11/site-packages/mf_smem
/usr/local/lib/python3.11/site-packages/mf_smem
├── VERSION
├── __init__.py
├── __pycache__
│   └── __init__.cpython-311.pyc
├── _pymf_smem.cpython-311-aarch64-linux-gnu.so
└── lib
    ├── libmf_hybm_core.so
    └── libmf_smem.so

root@localhost:/# tree /usr/local/lib/python3.11/site-packages/mf_adapter
/usr/local/lib/python3.11/site-packages/mf_adapter
├── VERSION
├── __init__.py
├── __pycache__
│   └── __init__.cpython-311.pyc
├── _pymf_transfer.cpython-311-aarch64-linux-gnu.so
└── lib
    ├── libmf_hybm_core.so
    └── libmf_smem.so 
    
root@localhost:/# tree /usr/local/lib/python3.11/site-packages/memcache
/usr/local/lib/python3.11/site-packages/memcache
├── VERSION
├── __init__.py
├── __pycache__
│   ├── __init__.cpython-311.pyc
│   └── meta_service_leader_election.cpython-311.pyc
├── _pymmc.cpython-311-aarch64-linux-gnu.so
├── lib
│   ├── libhybm_gvm.so
│   ├── libmf_hybm_core.so
│   ├── libmf_memcache.so
│   └── libmf_smem.so
└── meta_service_leader_election.py
    
```

在安装过程中，会默认尝试安装适配当前环境的memcache_hybrid的whl包，如果未安装，则在使用python接口前需要用户手动安装(
安装包路径参考上面目录结构)

memcache_hybrid 默认开启tls通信加密。如果想关闭，需要主动调用`smem_set_conf_store_tls`接口关闭：

```c
int32_t ret = smem_set_conf_store_tls(false, nullptr, 0);
```

具体细节详见安全声明章节
## 安装部署
### MetaService
* **python形式**：
*以下均以python311版本whl包（memcache-1.0.0-cp311-cp311-linux_aarch64.whl）为例*
```
1、安装whl包
pip install memcache-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_META_CONFIG_PATH=/usr/local/memfabric_hybrid/latest/config/mmc-meta.conf

3、进入python控制台或者编写python脚本如下即可拉起进程：
from memcache import MetaService
MetaService.main()
```
* **bin形式**：
```
1、安装run包
run包格式为 memcache_hybrid-${version}_${os}_${arch}.run
默认安装根路径为 /usr/local/
参考安装命令如下：
bash memcache_hybrid-1.0.0_linux_aarch64.run

如果想要自定义安装路径，可以添加--install-path参数
bash memcache_hybrid-1.0.0_linux_aarch64.run --install-path=${your path}
如果自定义安装路径，下述 /usr/local 需替换为 ${your path}

2、设置环境变量
source /usr/local/memfabric_hybrid/set_env.sh
export MMC_META_CONFIG_PATH=/usr/local/memfabric_hybrid/latest/config/mmc-meta.conf

3、拉起二进制
/usr/local/memfabric_hybrid/latest/aarch64-linux/bin/mmc_meta_service
```

### LocalService
* **whl（python）**：
```
1、安装whl包
pip install memcache-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_LOCAL_CONFIG_PATH=/usr/local/memfabric_hybrid/latest/config/mmc-local.conf

3、通过memcache提供的接口初始化客户端并拉起localservice，执行数据写入、查询、获取、删除等，下面的脚本时一个示例：
python3 test_mmc_demo.py
```
  
* **so（C++）**：
```
1、导入头文件
#include "mmcache.h"

2、将 libmf_memcache.so 路径加入到 LD_LIBRARY_PATH 环境变量
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${libmf_memcache.so path}

注：如果其所在路径已经在 PATH 或 LD_LIBRARY_PATH，则无需此步骤

3、调用API

```