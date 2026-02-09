# benchmark
提供python benchmark，以方便测试memcache内存池put/get性能       
主要涉及如下步骤

## 一、启动元数据服务
```shell
export MMC_META_CONFIG_PATH=${your config path}/mmc-meta.conf;
bash bench_start_meta.sh
```

## 二、写数据测试
```shell
export MMC_LOCAL_CONFIG_PATH=${your config path}/mmc-local.conf;
bash bench_start.sh -t write -p 1 -b 16 -s 1048576 -n 100 -d 1 -e memcache -l npu
```

## 三、读数据测试
```shell
export MMC_LOCAL_CONFIG_PATH=${your config path}/mmc-local.conf;
bash bench_start.sh -t read -p 1 -b 16 -s 1048576 -n 100 -d 1 -e memcache -l npu
```

## 入参说明

| **参数**        | **可选项**               | **说明** |
|----------------|--------------------------|----------|
| -t             | [read, write]            | 控制测试是写入数据还是读取数据      |
| -p             | [1, 32]                  | process count, 运行的进程数量, 受限于NPU的个数      |
| -b             | [1, ]                    | batch size, 单次IO key的个数      |
| -s             | [1, ]                    | block size, 单次IO key对应的value的大小，单位byte      |
| -n             | [10, ]                   | call count, 循环测试次数，受限于内存池总容量，会生成 batch_size * process_count * call_count个key   |
| -d             | [1, 2]                   | 测试连续tensor（1）还是离散tensor（2），离散tensor默认为61*128KB + 61*16KB，连续tensor大小为block size   |
| -e             | [memcache, mooncake]     | 内存池后端，默认支持memcache，mooncake须安装相关软件     |
| -l             | [npu, cpu]               | 测试数据tensor位于NPU还是CPU      |

