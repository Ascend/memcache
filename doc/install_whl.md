# whl包安装及使用

介绍whl包的安装及使用方法，适用于需要使用Python接口的用户

## 软件安装

whl包已发布到[pypi](https://pypi.org/project/memcache-hybrid/#files)，可以直接进行在线安装
```bash
# 安装最新版本
pip install memcache_hybrid
```

```bash
# 查看版本及安装路径
pip show memcache_hybrid
```

## 软件使用

### 安装依赖软件MemFabric

MemCache运行时需要依赖MemFabric

```bash
# 查看memfabric_hybrid
pip show memfabric_hybrid
```

如果安装的MemCache是1.1.x及以上版本，则在安装时会自动安装配套依赖的MemFabric，可以无需再安装

如果安装的MemCache是1.0.x版本，可通过[版本配套](https://gitcode.com/Ascend/memcache/releases)查看与MemCache匹配的MemFabric版本

```bash
# 此处以1.0.8版本为例
pip install memfabric_hybrid==1.0.8
```

MemFabric详细安装方法可参考[MemFabric使用指导](https://gitcode.com/Ascend/memfabric_hybrid/blob/master/doc/installation.md)

### 运行软件

我们提供了两种方式来启动软件，两种方式的区别在于设置配置项的方式不同

方式一：通过Python接口直接设置配置项（推荐）

方式二：通过配置文件和环境变量设置配置项（兼容原有方式）

如果安装的MemCache是1.0.x版本，只能选择方式二，1.1.x及以上版本，您可以根据需要选择其中一种启动方式即可

#### 方式一：通过Python接口直接设置配置项（推荐）

**1. 启动MetaService**

MetaService作为独立进程运行，进入python控制台或者编写python脚本如下即可拉起进程：
```bash
from memcache_hybrid import MetaService, MetaConfig

config = MetaConfig()
config.meta_service_url = "tcp://127.0.0.1:5000"
config.config_store_url = "tcp://127.0.0.1:6000"
config.metrics_url = "http://127.0.0.1:8000"
config.ha_enable = False
config.log_level = "info"

MetaService.setup(config)
MetaService.main()
```

**2. 启动LocalService**

LocalService作为客户端，以whl形式作为共享库提供API供应用进程调用加载，以下是通过Python接口设置配置项的一个简单示例

进入python控制台或者编写python脚本如下即可拉起示例进程：

```bash
from memcache_hybrid import DistributedObjectStore, LocalConfig

config = LocalConfig()
config.protocol = "device_rdma"
config.dram_size = "10GB"
config.max_dram_size = "64GB"
print(config)

store = DistributedObjectStore()
assert store.setup(config) == 0, "setup local config failed"
```

具体配置项可以参考[配置项说明](./memcache_config.md)，配置项的值请根据实际运行环境和场景进行修改

 👆 NOTE：此种方式设置的配置项优先级高于方式二中通过配置文件设置的值，如果同时使用方式二配置了配置文件，实际运行时使用的是此方法设置的值

#### 方式二：通过配置文件和环境变量设置配置项（兼容原有方式）

**1. 设置配置文件**

pip安装完成后可通过以下命令查看具体安装位置
```bash
# 查看安装位置和版本
pip show memcache_hybrid
```

查询结果如下：
```text
Name: memcache_hybrid
Version: 1.0.0
Summary: python api for memcache_hybrid
Home-page: https://gitcode.com/Ascend/memcache
Author:
Author-email:
License: Mulan PSL v2
Location: /usr/local/lib/python3.11/site-packages
Requires:
Required-by:
```
这里安装的位置在/usr/local/lib/python3.11/site-packages，则配置文件位于/usr/local/lib/python3.11/site-packages/memcache_hybrid/config

 👆 NOTE：1.0.x版本安装目录下不带配置文件，需要从[代码仓](https://gitcode.com/Ascend/memcache/tree/v1.0.0/config)获取

软件运行时，两个配置文件mmc-meta.conf（用于设置MetaService参数）和 mmc-local.conf（用于设置LocalService参数）可以位于任意目录

**建议将这两个配置文件复制到其他目录（比如/usr/local）再进行修改，防止重新安装后被覆盖**

运行前可以根据 [配置项说明](./memcache_config.md) 对配置文件 mmc-meta.conf 和 mmc-local.conf 进行相关配置

**2. 设置环境变量**

通过环境变量来设置配置文件的路径
```bash
# 两个配置文件可以放在任意路径，这里以默认安装路径为例，请修改为实际路径
export MMC_META_CONFIG_PATH=/usr/local/lib/python3.11/site-packages/memcache_hybrid/config/mmc-meta.conf
export MMC_LOCAL_CONFIG_PATH=/usr/local/lib/python3.11/site-packages/memcache_hybrid/config/mmc-local.conf
```

**3. 启动MetaService**

进入python控制台或者编写python脚本如下即可拉起进程：
```bash
from memcache_hybrid import MetaService
MetaService.main()
```

**3. 启动LocalService**

通过MemCache提供的[API](./memcache_python_api.md)初始化客户端并拉起LocalService，执行数据写入、查询、获取、删除等

下面的脚本[test_mmc_demo.py](../example/python/test_mmc_demo.py)是我们提供的一个示例：
```bash
python3 test_mmc_demo.py
```

## 软件卸载

```bash
pip uninstall memcache_hybrid
```
