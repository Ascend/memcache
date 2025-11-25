# Memcache

## 🔄Latest News

- [2025/11] memcached项目预计2025年11月30日开源，开源社区地址为：https://gitcode.com/Ascend/memcache

## 🎉概述

MemCache是针对LLM推理场景设计的高性能分布式键值 KV Cache 存储引擎，其主要特性包括：

- **基于对象操作的API**：支持批量和非批量的put/get/exist/remove操作
- **支持多副本**：单个对象支持多副本放置到不同的localservice，默认是单副本
- **高带宽低时延**：基于MemFabric作为多级内存和多通路传输的底座，在Ascend硬件上，基于device_rdma(A2)、device_sdma(A3)、host_rdma(A2/A3)等路径实现onecopy传输，提供高带宽，低时延的读写能力。
- **支持扩缩容**：支持localservice动态加入和移除
- **HA能力**：在K8S集群中，metaservice支持多活能力，提供尽力而为的HA能力。

![memcache_architecture.png](./doc/source/memcache_architecture.png)


## 🧩核心组件

MemCache包含LocalService和MetaService两大核心模块，基于MemFabric构建能力。

- **MetaService**：负责管理整个集群的内存池空间分配和元数据管理，并处理LocalService的加入与退出。MetaService作为独立进程运行，提供两种启动方式：python API启动；二进制启动，详见安装部署章节。
MetaService支持两种部署形态：
  - **1、单点模式**：MetaService由单个进程组成，部署方式简单，但存在单点故障的问题。如果MetaService进程崩溃或无法访问，系统将无法继续提供服务，直至重新恢复为止。
  - **2、HA模式**：该模式基于K8S的的ClusterIP
    Service和Lease资源构建，部署较为复杂，会部署多个MetaService进程实例，实现多活高可用。部署详见[MetaService HA](./doc/memcache_metaservice_HA.md)

- **LocalService**：负责承担如下功能：
  - **客户端**：作为客户端，以whl/so形式作为共享库被应用进程加载调用API
  - **内存提供者**：负责提供一段连续的内存区域作为内存池空间的一部分，其内存可以被其他LocalService实例基于地址直接访问。


## 🔥性能表现

- 在昇腾A2芯片上传输数据我们的带宽性能数据如下：

- 在昇腾A3芯片上传输数据我们的带宽性能数据如下：


## 🔍目录结构


## 🚀快速入门

请访问以下文档获取简易教程。

- [构建](./doc/build.md)：介绍组件编译和安装教程。

- 样例执行：具体流程参考example目录下各个样例中对应的README.md，example及其他样例代码仅供参考，在生产环境中请谨慎使用。

- [KV读写路径](./doc/KV.md)：介绍KV读写路径

## 📑学习教程

- [C接口](./doc/memcached_api.md)：C接口介绍以及C接口对应的API列表

## 📦软件硬件配套说明
[MemCache Conf](./doc/memcached_config.md)：MemCache将MetaService和LocalService的公共配置部分抽取为配置文件

## 📝相关信息