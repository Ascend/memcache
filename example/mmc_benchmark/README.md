# urma环境，容器命令

# xxx 替换自己的名称

# openeuler-24.03-ub:lastest 为openeuler-24.03官网镜像+memcache的包，也可以直接用openeuler-24.03，手动安装memcache包

```shell
docker run -itd --privileged=true --name xxx --net=host --entrypoint bash \
--device=/dev/uburma/bonding_dev_0 \
--device=/dev/uburma/udma2 \
--device=/dev/uburma/udma3 \
--device=/dev/uburma/udma4 \
--device=/dev/uburma/udma5 \
--device=/dev/uburma/udma6 \
--device=/dev/uburma/udma7 \
--device=/dev/ummu/tid \
-v /home:/home \
-v /usr/lib64:/usr/lib64:ro \
-v /usr/lib:/usr/lib:ro \
-v /usr/bin:/usr/bin:ro \
-v /sys/class/ubcore:/sys/class/ubcore:delegated \
-v /sys/devices/ub_bus_controller0:/sys/devices/ub_bus_controller0:delegated \
-v /sys/devices/ub_bus_controller1:/sys/devices/ub_bus_controller1:delegated \
-v /sys/devices/system/cpu/online:/sys/devices/system/cpu/online:delegated \
-v /sys/devices/virtual/ubcore:/sys/devices/virtual/ubcore:delegated \
-v /etc/localtime:/etc/localtime:delegated \
openeuler-24.03-ub:lastest
```