# MemCache

## 🔄Latest News

- [2025/11] MemCache项目于2025年11月开源，开源社区地址为：https://gitcode.com/Ascend/memcache

## 🎉概述

MemCache是针对LLM推理、GR推理场景设计的高性能分布式KVCache存储引擎，其主要特性包括：

- **基于对象操作的API**：支持批量和非批量的put/get/exist/remove操作
- **支持多副本**：单个对象支持多副本放置到不同的LocalService，默认是单副本
- **高带宽低时延**：使用 [MemFabric](https://gitcode.com/Ascend/memfabric_hybrid)
  作为多级内存和多通路传输的底座，在Ascend硬件上，基于device_rdma(A2)、device_sdma(A3)、host_rdma(A2/A3)
  等路径实现oneCopy传输，提供高带宽，低时延的读写能力。
- **支持扩缩容**：支持LocalService动态加入和移除
- **HA能力**：在K8S集群中，MetaService支持多活能力，提供尽力而为的HA能力。
![memcache_architecture.png](./doc/source/memcache_architecture.png)

## 🧩核心组件

MemCache包含LocalService和MetaService两大核心模块，基于MemFabric构建能力。

- **MetaService**：负责管理整个集群的内存池空间分配和元数据管理，并处理LocalService的加入与退出。MetaService作为独立进程运行，提供两种启动方式：python API启动；二进制启动，详见安装部署章节。
MetaService支持两种部署形态：
  - **1、单点模式**：MetaService由单个进程组成，部署方式简单，但存在单点故障的问题。如果MetaService进程崩溃或无法访问，系统将无法继续提供服务，直至重新恢复为止。
  - **2、HA模式**：该模式基于K8S的的ClusterIP
    Service和Lease资源构建，部署较为复杂，会部署多个MetaService进程实例，实现多活高可用。部署详见 [MetaService HA](./doc/memcache_metaservice_HA.md)

- **LocalService**：负责承担如下功能：
  - **客户端**：作为客户端，以whl/so形式作为共享库被应用进程加载调用API
  - **内存提供者**：负责提供一段连续的内存区域作为内存池空间的一部分，其内存可以被其他LocalService实例基于地址直接访问。


## 🔥性能表现

MemCache核心能力是提供大容量内存池和高性能的H2D、D2H、**D2RH、RH2D**
数据访问能力，由于MemCache以 [MemFabric](https://gitcode.com/Ascend/memfabric_hybrid)
作为池化底座，所以支持RH2D、D2RH等跨机一跳访问能力，下图为RH2D示意图。
![memcache_rh2d.png](./doc/source/memcache_rh2d.png)

模拟构造DeepSeek-R1模型KV大小的block，单个block size为：61x128K + 61x16K = 8784KB ≈ 8.57MB，共122个离散地址。

- 使用2个昇腾A2节点(每节点8张卡)组成双机内存池进行读写测试性能如下：
![memcache_a2](./doc/source/memcache_a2.png)

- 使用2个昇腾A3节点(每节点8张卡16Die)组成双机内存池进行读写测试性能如下：
![memcache_a3](./doc/source/memcache_a3.png)

## 🔍目录结构

```
├── LICENSE                                 # LICENSE
├── .clang-format                           # 格式化配置
├── .gitmodules                             # git配置
├── .gitignore                              # git忽视配置文件
├── CMakeLists.txt                          # 项目的CMakeList
├── doc                                     # 文档目录
├── example                                 # 样例
│  ├── cpp                                  # c++样例
│  └── python                               # python样例
├── script                                  # 构建脚本
│  ├── build_and_pack_run.sh                # 编译+打包脚本
│  ├── build.sh                             # 编译脚本
│  ├── run_ut.sh                            # 编译+运行ut脚本
├── test                                    # test目录
│  ├── python                               # python测试用例
│  ├── k8s_deploy                           # k8s ha样例脚本
│  ├── fuzz                                 # fuzz目录
│  └── ut                                   # 单元测试用例
├── src                                     # 源码
│  ├── memcache                             # MemCache 源码
├── config                                  # 配置目录
│  ├── mmc-local.conf                       # 本地服务配置文件模板
│  ├── mmc-meta.conf                        # meta服务配置文件模板
├── README.md
```

## 🚀快速入门

请访问以下文档获取简易教程。

- [构建](./doc/build.md)：介绍组件编译和安装教程。

- [样例执行](./example/examples.md)：介绍如何端到端执行样例代码，包括C++和Python样例。

## 📑学习教程

- [C接口](./doc/mamcached_api.md)：C接口介绍以及C接口对应的API列表
- [python接口](./doc/pythonAPI.md)：python接口介绍以及python接口对应的API列表

## 📦软件硬件配套说明

[MemCache Conf](./doc/memcached_config.md)：MemCache将MetaService和LocalService的公共配置部分抽取为配置文件

## 📝相关信息

- [安全声明](./doc/SECURITYNOTE.md)

- [许可证](./LICENSE)