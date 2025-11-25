# 编译

## 安装部署
### MetaService
* **python形式**：
*以下均以python311版本whl包（memcache-1.0.0-cp311-cp311-linux_aarch64.whl）为例*
```
1、安装whl包
pip install memcache-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_META_CONFIG_PATH=/usr/local/mxc/memfabric_hybrid/latest/config/mmc-meta.conf

3、进入python控制台或者编写python脚本如下即可拉起进程：
from memcache import MetaService
MetaService.main()
```
* **bin形式**：
```
1、安装run包
run包格式为 mxc-memfabric-hybrid-${version}_${os}_${arch}.run
默认安装根路径为 /usr/local/
参考安装命令如下：
bash mxc-memfabric_hybrid-1.0.0_linux_aarch64.run

如果想要自定义安装路径，可以添加--install-path参数
bash mxc-memfabric_hybrid-1.0.0_linux_aarch64.run --install-path=${your path}
如果自定义安装路径，下述 /usr/local 需替换为 ${your path}

2、设置环境变量
source /usr/local/mxc/memfabric_hybrid/set_env.sh
export MMC_META_CONFIG_PATH=/usr/local/mxc/memfabric_hybrid/latest/config/mmc-meta.conf

3、拉起二进制
/usr/local/mxc/memfabric_hybrid/latest/aarch64-linux/bin/mmc_meta_service
```

### LocalService
* **whl（python）**：
```
1、安装whl包
pip install memcache-1.0.0-cp311-cp311-linux_aarch64.whl

2、设置配置文件环境变量
export MMC_LOCAL_CONFIG_PATH=/usr/local/mxc/memfabric_hybrid/latest/config/mmc-local.conf

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