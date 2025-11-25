# KV读写路径
MemCache在LLM推理场景，核心能力是提供大容量内存池和高性能的H2D、D2H、**D2RH、RH2D** 数据传输，其中本地swap，即H2D、D2H为通用路径。D2RH、RH2D路径差异由下图所示（*此示意图为RH2D，推理block，即block4，由离散tensor组成，D2RH是反向过程，在此不赘述*）
![memcache_rh2d.png](./source/memcache_rh2d.png)
* **路径0**：基于host_rdma，MemCache支持，性能受限于rdma网卡传输带宽，常见规格为100GE/200GE
* **路径1**：基于device D2D中转，利用device侧高带宽，对于在**Host1**发起读请求的应用，将block1传输到block4，需要**一次内部RPC和3次拷贝**，MemCache未支持此实现
* **路径2**：基于device_sdma(A3)/device_rdma(A2)，MemCache支持，对于在**Host1**发起读请求的应用，直接在Host1访问block1地址即可将其拷贝到block3，再将其拷贝到离散tensor组成的block4，**直接地址访问，无需内部RPC**，适用于block组成的tensor size较小，num较多的模型
* **路径3**：基于device_sdma(A3)/device_rdma(A2)，MemCache支持，对于在**Host1**发起读请求的应用，直接在Host1访问block1地址即可将其拷贝到block4，**直接地址访问，无需内部RPC**，适用于block组成的tensor size较大，num较少的模型
