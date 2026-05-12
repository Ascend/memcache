# run包编译、安装及使用
介绍run包的安装及使用方法，适用于需要使用C++接口的用户

run包没有发布二进制包，需要用户自行编译

## run包编译

### 环境准备

编译工具建议版本

- OS: Ubuntu 22.04 LTS+
- cmake: 3.20.x
- gcc: 11.4+
- python 3.11.10
- pybind11 2.10.3
- make 4.3 or ninja 1.10.1

### 编译

**1. 下载代码**

```bash
git clone https://gitcode.com/Ascend/memcache
cd memcache
git clean -xdf
git reset --hard
```

**2. 拉取第三方库**

```bash
# Initialize and update only the required submodules (excluding test dependencies)
git submodule update --init 3rdparty/

# Update memfabric_hybrid to the latest version of a specific branch (e.g., master)
# Replace 'master' with your target branch name
git -c submodule.3rdparty/memfabric_hybrid.branch=master submodule update --remote 3rdparty/memfabric_hybrid
```

**说明：**
- `--init 3rdparty` 只初始化并更新指定的子模块，避免拉取 test 目录等不必要的依赖
- 通过 `-c submodule.3rdparty/memfabric_hybrid.branch=<branch_name>` 参数可以指定拉取的目标分支
- 若需拉取所有子模块（包括测试依赖），可使用 `git submodule update --recursive --init`

**3. 编译**

执行如下命令进行编译

```bash
bash script/build_and_pack_run.sh --build_mode RELEASE --build_test OFF
```

- build_and_pack_run.sh支持2个参数，分别是<build_mode> <build_test>
- build_mode: 编译类型，可填RELEASE或DEBUG，默认RELEASE
- build_test: 是否打包测试工具，可填ON或OFF，默认OFF

编译成功后，生成的run包在output目录下，生成的whl包在output/memcache/wheel目录下

## run包安装

### 安装MemFabric

MemCache依赖MemFabric，需要先安装MemFabric，详细安装方法可参考[MemFabric使用指导](https://gitcode.com/Ascend/memfabric_hybrid/blob/master/doc/installation.md)

### 安装MemCache

MemCache将所有特性集成到run包中供用户使用，run包格式为 ```memcache_hybrid-${version}_${os}_${arch}.run```

其中，version表示MemCache的版本；os表示操作系统，如linux；arch表示架构，如x86或aarch64

run包的默认安装根路径为 /usr/local/

参考安装命令如下：

```bash
cd output
bash memcache_hybrid-*_linux_aarch64.run # 请修改为实际路径和文件名
```

如果想要自定义安装路径，可以添加--install-path参数

```bash
bash memcache_hybrid-*_linux_aarch64.run --install-path=${your path}  # 请修改为实际路径和文件名
```

安装的run包可以通过如下命令查看版本（此处以默认安装路径为例）

```bash
cat /usr/local/memcache_hybrid/latest/version.info
```

## 软件使用

### 设置环境变量

```bash
# 这里使用的是默认安装路径，请修改为实际路径
source /usr/local/memcache_hybrid/set_env.sh
source /usr/local/memfabric_hybrid/set_env.sh
```

### 启动MetaService

MetaService作为独立进程运行，可以在设置配置项之后直接拉起

**1. 修改配置文件**

安装完成后配置文件位于安装目录下的memcache_hybrid/latest/config/mmc-meta.conf

**建议将配置文件复制到其他目录再进行修改，防止重新安装后被覆盖**

运行前需要根据 [MetaService配置项](./memcache_config.md) 对配置文件 mmc-meta.conf 进行相关配置

**2. 设置配置文件路径**

```bash
# 这里使用的是默认安装路径，请修改为实际路径
export MMC_META_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-meta.conf
```

**3. 启动MetaService**

```bash
/usr/local/memcache_hybrid/latest/aarch64-linux/bin/mmc_meta_service
```

### LocalService

LocalService作为客户端，以共享库的形式提供API供应用进程调用加载

**1. 修改配置文件**

安装完成后配置文件位于安装目录下的memcache_hybrid/latest/config/mmc-local.conf

**建议将配置文件复制到其他目录再进行修改，防止重新安装后被覆盖**

运行前需要根据 [LocalService配置项](./memcache_config.md) 对配置文件 mmc-local.conf 进行相关配置

**2. 设置配置文件路径**

```bash
# 这里使用的是默认安装路径，请修改为实际路径
export MMC_LOCAL_CONFIG_PATH=/usr/local/memcache_hybrid/latest/config/mmc-local.conf
```

**3. 导入头文件**

```bash
#include "mmcache.h"
```

**4. 调用相关API**

通过调用MemCache提供的[API](./memcache_c++_api.md)初始化客户端，执行数据写入、查询、获取、删除等

可参考示例代码[memcache_cpp_test.cpp](../example/cpp/memcache_cpp_test.cpp)

## 软件卸载

卸载脚本是位于安装目录下的memcache_hybrid/latest/uninstall.sh，直接执行即可完成卸载

```bash
# 这里使用的是默认安装路径，请修改为实际路径
bash /usr/local/memcache_hybrid/latest/uninstall.sh
```
