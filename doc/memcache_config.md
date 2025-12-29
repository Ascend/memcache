## MemCache Configs

#### MetaService Config

| key                                     | value type | requirement | default                 | valid range               | description                                                  |
| --------------------------------------- | ---------- | ----------- | ----------------------- | ------------------------- | ------------------------------------------------------------ |
| ock.mmc.meta_service_url                | string     | optional    | tcp://127.0.0.1:5000    | tcp://\<ip>\<port>        | port in [1025, 65535]                                        |
| ock.mmc.meta_service.config_store_url   | string     | optional    | tcp://127.0.0.1:6000    | tcp://\<ip>\<port>        | port in [1025, 65535]                                        |
| ock.mmc.meta_service.metrics_url        | string     | optional    | 127.0.0.1:8000          | \<ip>\<port>              | port in [1025, 65535]                                        |
| ock.mmc.meta.ha.enable                  | bool       | optional    | false                   | true/false                | enable meta service master/backup HA in k8s cluster          |
| ock.mmc.log_level                       | string     | optional    | info                    | debug/info/warn/error     | log level                                                    |
| ock.mmc.log_path                        | string     | optional    | .                       | relative or absolute path | log path, the absolute path is start with '/'                |
| ock.mmc.log_rotation_file_size          | int        | optional    | 20                      | 1 <= n <= 500             | log rotation file size(MB)                                   |
| ock.mmc.log_rotation_file_count         | int        | optional    | 50                      | 1 <= n <= 50              | log rotation file num                                        |
| ock.mmc.evict_threshold_high            | int        | optional    | 70                      | 1 <= n <= 100             | evict threshold, 70 mean 70%                                 |
| ock.mmc.evict_threshold_low             | int        | optional    | 60                      | 0 <= n <= 99              | after evict threshold                                        |
|                                         |            |             |                         |                           |                                                              |
| ock.mmc.tls.enable                      | bool       | optional    | false                   | true/false                | metaservice TLS switch                                       |
| ock.mmc.tls.ca.path                     | string     | optional    |                         | 0 <= len < 256            | Root Certificate path                                        |
| ock.mmc.tls.ca.crl.path                 | string     | optional    |                         | 0 <= len < 256            | Certificate Revocation List path                             |
| ock.mmc.tls.cert.path                   | string     | optional    |                         | 0 <= len < 256            | server certificate path                                      |
| ock.mmc.tls.key.path                    | string     | optional    |                         | 0 <= len < 256            | server private key path                                      |
| ock.mmc.tls.key.pass.path               | string     | optional    |                         | 0 <= len < 256            | server private key passphrase path (leave empty if the private key is not encrypted) |
| ock.mmc.tls.package.path                | string     | optional    | /opt/ock/security/libs/ | 0 <= len < 256            | openssl dynamic libraries path                               |
| ock.mmc.tls.decrypter.path              | string     | optional    |                         | 0 <= len < 256            | the keypass decrypter library path (leave empty if the password is not encrypted) |
|                                         |            |             |                         |                           |                                                              |
| ock.mmc.config_store.tls.enable         | bool       | optional    | false                   | true/false                | config store TLS switch                                      |
| ock.mmc.config_store.tls.ca.path        | string     | optional    |                         | 0 <= len < 256            | Root Certificate path for config store                       |
| ock.mmc.config_store.tls.ca.crl.path    | string     | optional    |                         | 0 <= len < 256            | Certificate Revocation List path for config store            |
| ock.mmc.config_store.tls.cert.path      | string     | optional    |                         | 0 <= len < 256            | server certificate path for config store                     |
| ock.mmc.config_store.tls.key.path       | string     | optional    |                         | 0 <= len < 256            | server private key path for config store                     |
| ock.mmc.config_store.tls.key.pass.path  | string     | optional    |                         | 0 <= len < 256            | server private key passphrase path for config store (leave empty if the private key is not encrypted) |
| ock.mmc.config_store.tls.package.path   | string     | optional    | /opt/ock/security/libs/ | 0 <= len < 256            | openssl dynamic libraries path for config store              |
| ock.mmc.config_store.tls.decrypter.path | string     | optional    |                         | 0 <= len < 256            | the keypass decrypter library path for config store (leave empty if the password is not encrypted) |

#### LocalService Config

| key                                           | value type | requirement  | default                 | valid range                                          | description                                                  |
| --------------------------------------------- | ---------- | ------------ | ----------------------- | ---------------------------------------------------- | ------------------------------------------------------------ |
| ock.mmc.meta_service_url                      | string     | optional     | tcp://127.0.0.1:5000    | tcp://\<ip>\<port>                                   | port in [1025, 65535], <br/> <ip> is cluster-ip in HA        |
| ock.mmc.log_level                             | string     | optional     | info                    | debug/info/warn/error                                | log level                                                    |
|                                               |            |              |                         |                                                      |                                                              |
| ock.mmc.tls.enable                            | bool       | optional     | false                   | true/false                                           | metaservice TLS switch                                       |
| ock.mmc.tls.ca.path                           | string     | optional     |                         | 0 <= len < 256                                       | Root Certificate path                                        |
| ock.mmc.tls.ca.crl.path                       | string     | optional     |                         | 0 <= len < 256                                       | Certificate Revocation List path                             |
| ock.mmc.tls.cert.path                         | string     | optional     |                         | 0 <= len < 256                                       | client certificate path                                      |
| ock.mmc.tls.key.path                          | string     | optional     |                         | 0 <= len < 256                                       | client private key path                                      |
| ock.mmc.tls.key.pass.path                     | string     | optional     |                         | 0 <= len < 256                                       | client private key passphrase path (leave empty if the private key is not encrypted) |
| ock.mmc.tls.package.path                      | string     | optional     | /opt/ock/security/libs/ | 0 <= len < 256                                       | openssl dynamic libraries path                               |
| ock.mmc.tls.decrypter.path                    | string     | optional     |                         | 0 <= len < 256                                       | the keypass decrypter library path (leave empty if the password is not encrypted) |
|                                               |            |              |                         |                                                      |                                                              |
| ock.mmc.local_service.world_size              | integer    | optional     | 16                      | [1, 1024]                                            |                                                              |
| ock.mmc.local_service.config_store_url        | string     | optional     | tcp://127.0.0.1:6000    | tcp://\<ip>\<port>                                   | port in [1025, 65535]                                        |
| ock.mmc.config_store.tls.enable               | bool       | optional     | false                   | true/false                                           | config store TLS switch                                      |
| ock.mmc.config_store.tls.ca.path              | string     | optional     |                         | 0 <= len < 256                                       | Root Certificate path for config store                       |
| ock.mmc.config_store.tls.ca.crl.path          | string     | optional     |                         | 0 <= len < 256                                       | Certificate Revocation List path for config store            |
| ock.mmc.config_store.tls.cert.path            | string     | optional     |                         | 0 <= len < 256                                       | client certificate path for config store                     |
| ock.mmc.config_store.tls.key.path             | string     | optional     |                         | 0 <= len < 256                                       | client private key path for config store                     |
| ock.mmc.config_store.tls.key.pass.path        | string     | optional     |                         | 0 <= len < 256                                       | client private key passphrase path for config store (leave empty if the private key is not encrypted) |
| ock.mmc.config_store.tls.package.path         | string     | optional     | /opt/ock/security/libs/ | 0 <= len < 256                                       | openssl dynamic libraries path for config store              |
| ock.mmc.config_store.tls.decrypter.path       | string     | optional     |                         | 0 <= len < 256                                       | the keypass decrypter library path for config store (leave empty if the password is not encrypted) |
|                                               |            |              |                         |                                                      |                                                              |
| ock.mmc.local_service.protocol                | string     | **required** | host_rdma               | host_rdma/host_urma/host_tcp/device_sdma/device_rdma |                                                              |
| ock.mmc.local_service.dram.size               | integer    | **required** | 128MB                   | [0, 1TB]                                             | Supports formats such as 134217728, 2048KB, 200mb, 2.5G or 1TB, <br/> and automatic alignment to 2MB (host_sdma or host_tcp) or 1GB (device_sdma or device_rdma). |
| ock.mmc.local_service.hbm.size                | integer    | **required** | 0                       | [0, 1TB]                                             | Supports formats such as 134217728, 2048KB, 200mb, 2.5G or 1TB, <br/> and automatic alignment to 2MB (host_sdma or host_tcp) or 1GB (device_sdma or device_rdma). |
|                                               |            |              |                         |                                                      |                                                              |
| ock.mmc.local_service.hcom_url                | string     | optional     | tcp://127.0.0.1:7000    | tcp://\<ip>\<port>                                   | used in dram pool, port in [1024, 65535]                     |
| ock.mmc.local_service.hcom.tls.enable         | bool       | optional     | false                   | true/false                                           | hcom TLS switch                                              |
| ock.mmc.local_service.hcom.tls.ca.path        | string     | optional     |                         | 0 <= len < 256                                       | Root Certificate path for hcom                               |
| ock.mmc.local_service.hcom.tls.ca.crl.path    | string     | optional     |                         | 0 <= len < 256                                       | Certificate Revocation List path for hcom                    |
| ock.mmc.local_service.hcom.tls.cert.path      | string     | optional     |                         | 0 <= len < 256                                       | client certificate path for hcom                             |
| ock.mmc.local_service.hcom.tls.key.path       | string     | optional     |                         | 0 <= len < 256                                       | client private key path for hcom                             |
| ock.mmc.local_service.hcom.tls.key.pass.path  | string     | optional     |                         | 0 <= len < 256                                       | client private key passphrase path for hcom (leave empty if the private key is not encrypted) |
| ock.mmc.local_service.hcom.tls.decrypter.path | string     | optional     |                         | 0 <= len < 256                                       | the keypass decrypter library path for hcom (leave empty if the password is not encrypted) |
|                                               |            |              |                         |                                                      |                                                              |
| ock.mmc.client.retry_milliseconds             | integer    | optional     | 0                       | [0, 600000]                                          | The total retry time when the meta service connection does not exist |
| ock.mmc.client.timeout.seconds                | integer    | optional     | 60                      | [1, 600]                                             |                                                              |
| ock.mmc.client.read_thread_pool.size          | integer    | optional     | 32                      | [1, 64]                                              |                                                              |
| ock.mmc.client.aggregate.io                   | bool       | optional     | false                   | true/false                                           |                                                              |
| ock.mmc.client.write_thread_pool.size         | integer    | optional     | 4                       | [1, 64]                                              |                                                              |
