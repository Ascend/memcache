/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/

#include <algorithm>

#include "mmc_client.h"
#include "mmc.h"
#include "mmc_ptracer.h"
#include "mmc_meta_service_process.h"
#include "common/mmc_functions.h"
#include "smem_bm_def.h"
#include "pymmc.h"

namespace py = pybind11;
using namespace ock::mmc;
using namespace ock::mf;

void DefineMmcStructModule(py::module_ &m)
{
    // Define the KeyInfo class
    py::class_<KeyInfo, std::shared_ptr<KeyInfo>>(m, "KeyInfo", py::buffer_protocol())
        .def("size", &KeyInfo::Size)
        .def("loc_list", &KeyInfo::GetLocs)
        .def("type_list", &KeyInfo::GetTypes)
        .def("__str__", &KeyInfo::ToString)
        .def("__repr__", &KeyInfo::ToString);

    py::class_<ReplicateConfig>(m, "ReplicateConfig", R"pbdoc(
         Configuration for replica allocation policy.
     )pbdoc")
        .def(py::init<>(), R"pbdoc(
         Default constructor.
         Initializes:
           - preferredLocalServiceIDs = {}
           - replicaNum = 0
     )pbdoc")
        .def_readwrite("replicaNum", &ReplicateConfig::replicaNum, R"pbdoc(
         Less than or equal to 8.
     )pbdoc")
        .def_readwrite("preferredLocalServiceIDs", &ReplicateConfig::preferredLocalServiceIDs, R"pbdoc(
         List of instance IDs for forced storage. The values in the list must be unique,
         and the list size must be equal to replicaNum.
     )pbdoc");

    auto localConfigClass =
        py::class_<local_config>(m, "LocalConfig", R"pbdoc(
         Local configuration for memcache client.
     )pbdoc")
            .def(py::init([]() { return create_default_local_config(); }),
                 R"pbdoc(
             Default constructor. Initializes with default values.
         )pbdoc")
            .def_property(
                "meta_service_url", [](const local_config &cfg) { return std::string(cfg.meta_service_url); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.meta_service_url, sizeof(cfg.meta_service_url));
                },
                R"pbdoc(
             Meta service start-up url.
         )pbdoc")
            .def_property(
                "config_store_url", [](const local_config &cfg) { return std::string(cfg.config_store_url); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_url, sizeof(cfg.config_store_url));
                },
                R"pbdoc(
             Config store url.
         )pbdoc")
            .def_property(
                "log_level", [](const local_config &cfg) { return std::string(cfg.log_level); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.log_level, sizeof(cfg.log_level));
                },
                R"pbdoc(
             Log level: debug, info, warn, error.
         )pbdoc")
            .def_readwrite("world_size", &local_config::world_size, R"pbdoc(
             The maximum supported rank count.
         )pbdoc")
            .def_property(
                "protocol", [](const local_config &cfg) { return std::string(cfg.protocol); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.protocol, sizeof(cfg.protocol));
                },
                R"pbdoc(
             Data transfer protocol: host_rdma, host_urma, host_tcp, device_rdma, device_sdma.
         )pbdoc")
            .def_property(
                "hcom_url", [](const local_config &cfg) { return std::string(cfg.hcom_url); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_url, sizeof(cfg.hcom_url));
                },
                R"pbdoc(
             HCOM URL for RDMA network.
         )pbdoc")
            .def_property(
                "dram_size", [](const local_config &cfg) { return std::string(cfg.dram_size); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.dram_size, sizeof(cfg.dram_size));
                },
                R"pbdoc(
             DRAM space usage, supports formats like 1GB, 2MB, etc.
         )pbdoc")
            .def_property(
                "hbm_size", [](const local_config &cfg) { return std::string(cfg.hbm_size); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hbm_size, sizeof(cfg.hbm_size));
                },
                R"pbdoc(
             HBM space usage.
         )pbdoc")
            .def_property(
                "max_dram_size", [](const local_config &cfg) { return std::string(cfg.max_dram_size); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.max_dram_size, sizeof(cfg.max_dram_size));
                },
                R"pbdoc(
             The MAX size of dram_size in all local processes.
         )pbdoc")
            .def_property(
                "max_hbm_size", [](const local_config &cfg) { return std::string(cfg.max_hbm_size); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.max_hbm_size, sizeof(cfg.max_hbm_size));
                },
                R"pbdoc(
             The MAX size of hbm_size in all local processes.
         )pbdoc")
            .def_readwrite("client_retry_milliseconds", &local_config::client_retry_milliseconds, R"pbdoc(
             The total retry duration when client requests meta service.
         )pbdoc")
            .def_readwrite("client_timeout_seconds", &local_config::client_timeout_seconds, R"pbdoc(
             Client request timeout in seconds.
         )pbdoc")
            .def_readwrite("read_thread_pool_size", &local_config::read_thread_pool_size, R"pbdoc(
             Read thread pool size.
         )pbdoc")
            .def_readwrite("write_thread_pool_size", &local_config::write_thread_pool_size, R"pbdoc(
             Write thread pool size.
         )pbdoc")
            .def_readwrite("aggregate_io", &local_config::aggregate_io, R"pbdoc(
             Enable read/write aggregate.
         )pbdoc")
            .def_readwrite("aggregate_num", &local_config::aggregate_num, R"pbdoc(
             Aggregate number.
         )pbdoc")
            .def_readwrite("ubs_io_enable", &local_config::ubs_io_enable, R"pbdoc(
             Enable UBS_IO.
         )pbdoc")
            .def_property(
                "memory_pool_mode", [](const local_config &cfg) { return std::string(cfg.memory_pool_mode); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.memory_pool_mode, sizeof(cfg.memory_pool_mode));
                },
                R"pbdoc(
             Memory pool mode: standard or expanded.
         )pbdoc")
            .def_readwrite("tls_enable", &local_config::tls_enable, R"pbdoc(
             Enable TLS for metaservice.
         )pbdoc")
            .def_property(
                "tls_ca_path", [](const local_config &cfg) { return std::string(cfg.tls_ca_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_ca_path, sizeof(cfg.tls_ca_path));
                },
                R"pbdoc(
             TLS CA certificate path.
         )pbdoc")
            .def_property(
                "tls_ca_crl_path", [](const local_config &cfg) { return std::string(cfg.tls_ca_crl_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_ca_crl_path, sizeof(cfg.tls_ca_crl_path));
                },
                R"pbdoc(
             TLS CA CRL path.
         )pbdoc")
            .def_property(
                "tls_cert_path", [](const local_config &cfg) { return std::string(cfg.tls_cert_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_cert_path, sizeof(cfg.tls_cert_path));
                },
                R"pbdoc(
             TLS certificate path.
         )pbdoc")
            .def_property(
                "tls_key_path", [](const local_config &cfg) { return std::string(cfg.tls_key_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_key_path, sizeof(cfg.tls_key_path));
                },
                R"pbdoc(
             TLS key path.
         )pbdoc")
            .def_property(
                "tls_key_pass_path", [](const local_config &cfg) { return std::string(cfg.tls_key_pass_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_key_pass_path, sizeof(cfg.tls_key_pass_path));
                },
                R"pbdoc(
             TLS key passphrase path.
         )pbdoc")
            .def_property(
                "tls_package_path", [](const local_config &cfg) { return std::string(cfg.tls_package_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_package_path, sizeof(cfg.tls_package_path));
                },
                R"pbdoc(
             TLS package path.
         )pbdoc")
            .def_property(
                "tls_decrypter_path", [](const local_config &cfg) { return std::string(cfg.tls_decrypter_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.tls_decrypter_path, sizeof(cfg.tls_decrypter_path));
                },
                R"pbdoc(
             TLS decrypter library path.
         )pbdoc")
            .def_readwrite("config_store_tls_enable", &local_config::config_store_tls_enable, R"pbdoc(
             Enable TLS for config_store.
         )pbdoc")
            .def_property(
                "config_store_tls_ca_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_ca_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_ca_path, sizeof(cfg.config_store_tls_ca_path));
                },
                R"pbdoc(
             Config store TLS CA certificate path.
         )pbdoc")
            .def_property(
                "config_store_tls_ca_crl_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_ca_crl_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_ca_crl_path, sizeof(cfg.config_store_tls_ca_crl_path));
                },
                R"pbdoc(
             Config store TLS CA CRL path.
         )pbdoc")
            .def_property(
                "config_store_tls_cert_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_cert_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_cert_path, sizeof(cfg.config_store_tls_cert_path));
                },
                R"pbdoc(
             Config store TLS certificate path.
         )pbdoc")
            .def_property(
                "config_store_tls_key_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_key_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_key_path, sizeof(cfg.config_store_tls_key_path));
                },
                R"pbdoc(
             Config store TLS key path.
         )pbdoc")
            .def_property(
                "config_store_tls_key_pass_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_key_pass_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_key_pass_path, sizeof(cfg.config_store_tls_key_pass_path));
                },
                R"pbdoc(
             Config store TLS key passphrase path.
         )pbdoc")
            .def_property(
                "config_store_tls_package_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_package_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_package_path, sizeof(cfg.config_store_tls_package_path));
                },
                R"pbdoc(
             Config store TLS package path.
         )pbdoc")
            .def_property(
                "config_store_tls_decrypter_path",
                [](const local_config &cfg) { return std::string(cfg.config_store_tls_decrypter_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.config_store_tls_decrypter_path, sizeof(cfg.config_store_tls_decrypter_path));
                },
                R"pbdoc(
             Config store TLS decrypter library path.
         )pbdoc")
            .def_readwrite("hcom_tls_enable", &local_config::hcom_tls_enable, R"pbdoc(
             Enable TLS for HCOM.
         )pbdoc")
            .def_property(
                "hcom_tls_ca_path", [](const local_config &cfg) { return std::string(cfg.hcom_tls_ca_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_ca_path, sizeof(cfg.hcom_tls_ca_path));
                },
                R"pbdoc(
             HCOM TLS CA certificate path.
         )pbdoc")
            .def_property(
                "hcom_tls_ca_crl_path", [](const local_config &cfg) { return std::string(cfg.hcom_tls_ca_crl_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_ca_crl_path, sizeof(cfg.hcom_tls_ca_crl_path));
                },
                R"pbdoc(
             HCOM TLS CA CRL path.
         )pbdoc")
            .def_property(
                "hcom_tls_cert_path", [](const local_config &cfg) { return std::string(cfg.hcom_tls_cert_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_cert_path, sizeof(cfg.hcom_tls_cert_path));
                },
                R"pbdoc(
             HCOM TLS certificate path.
         )pbdoc")
            .def_property(
                "hcom_tls_key_path", [](const local_config &cfg) { return std::string(cfg.hcom_tls_key_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_key_path, sizeof(cfg.hcom_tls_key_path));
                },
                R"pbdoc(
             HCOM TLS key path.
         )pbdoc")
            .def_property(
                "hcom_tls_key_pass_path",
                [](const local_config &cfg) { return std::string(cfg.hcom_tls_key_pass_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_key_pass_path, sizeof(cfg.hcom_tls_key_pass_path));
                },
                R"pbdoc(
             HCOM TLS key passphrase path.
         )pbdoc")
            .def_property(
                "hcom_tls_decrypter_path",
                [](const local_config &cfg) { return std::string(cfg.hcom_tls_decrypter_path); },
                [](local_config &cfg, const std::string &value) {
                    SafeCopy(value, cfg.hcom_tls_decrypter_path, sizeof(cfg.hcom_tls_decrypter_path));
                },
                R"pbdoc(
             HCOM TLS decrypter library path.
         )pbdoc");

    localConfigClass
        .def("str", &local_config_to_string, R"pbdoc(
             Returns a string representation of all member variables.
         )pbdoc")
        .def("__str__", &local_config_to_string, R"pbdoc(
             Returns a string representation of all member variables.
         )pbdoc")
        .def("__repr__", &local_config_to_string, R"pbdoc(
             Returns a string representation of all member variables.
         )pbdoc");
}

PYBIND11_MODULE(_pymmc, m)
{
    DefineMmcStructModule(m);
    ock::mmc::ReplicateConfig defaultConfig;
    // Support starting the meta service from python
    py::class_<MmcMetaServiceProcess>(m, "MetaService", R"pbdoc(
         Class for memcache meta service process.
     )pbdoc")
        .def_static(
            "main", []() { return MmcMetaServiceProcess::getInstance().MainForPython(); },
            "Start the meta service process directly. This is a blocking call.");

    // Define the MmcacheStore class
    py::class_<MmcacheStore>(m, "DistributedObjectStore")
        .def(py::init<>())
        .def("setup", &MmcacheStore::Setup, py::call_guard<py::gil_scoped_release>(), py::arg("config"),
             "Setup local configuration")
        .def("init", &MmcacheStore::Init, py::call_guard<py::gil_scoped_release>(), py::arg("device_id"),
             py::arg("init_bm") = true)
        .def("remove", &MmcacheStore::Remove, py::call_guard<py::gil_scoped_release>())
        .def("remove_batch", &MmcacheStore::BatchRemove, py::call_guard<py::gil_scoped_release>())
        .def("remove_all", &MmcacheStore::RemoveAll, py::call_guard<py::gil_scoped_release>())
        .def("is_exist", &MmcacheStore::IsExist, py::call_guard<py::gil_scoped_release>())
        .def("batch_is_exist", &MmcacheStore::BatchIsExist, py::call_guard<py::gil_scoped_release>(), py::arg("keys"),
             "Check if multiple objects exist. Returns list of results: 1 if exists, 0 if not exists, -1 if error")
        .def("get_key_info", &MmcacheStore::GetKeyInfo, py::call_guard<py::gil_scoped_release>())
        .def("batch_get_key_info", &MmcacheStore::BatchGetKeyInfo, py::call_guard<py::gil_scoped_release>(),
             py::arg("keys"))
        .def("close", &MmcacheStore::TearDown)
        .def(
            "register_buffer",
            [](MmcacheStore &self, uintptr_t buffer_ptr, size_t size) {
                // Register memory buffer for RDMA operations
                void *buffer = reinterpret_cast<void *>(buffer_ptr);
                py::gil_scoped_release release;
                return self.RegisterBuffer(buffer, size);
            },
            py::arg("buffer_ptr"), py::arg("size"), "Register a memory buffer for direct access operations")
        .def(
            "unregister_buffer",
            [](MmcacheStore &self, uintptr_t buffer_ptr, size_t size) {
                // UnRegister memory buffer for RDMA operations
                void *buffer = reinterpret_cast<void *>(buffer_ptr);
                py::gil_scoped_release release;
                return self.UnRegisterBuffer(buffer, size);
            },
            py::arg("buffer_ptr"), py::arg("size"), "UnRegister a memory buffer for direct access operations")
        .def(
            "get_into",
            [](MmcacheStore &self, const std::string &key, uintptr_t buffer_ptr, size_t size, const int32_t &direct) {
                py::gil_scoped_release release;
                return self.GetInto(key, reinterpret_cast<void *>(buffer_ptr), size, direct);
            },
            py::arg("key"), py::arg("buffer_ptr"), py::arg("size"), py::arg("direct") = SMEMB_COPY_G2H,
            "Get object data directly into a pre-allocated buffer")
        .def(
            "batch_get_into",
            [](MmcacheStore &self, const std::vector<std::string> &keys, const std::vector<uintptr_t> &buffer_ptrs,
               const std::vector<size_t> &sizes, const int32_t &direct) {
                std::vector<void *> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (uintptr_t ptr : buffer_ptrs) {
                    buffers.push_back(reinterpret_cast<void *>(ptr));
                }
                py::gil_scoped_release release;
                return self.BatchGetInto(keys, buffers, sizes, direct);
            },
            py::arg("keys"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_G2H,
            "Get object data directly into pre-allocated buffers for multiple "
            "keys")
        .def(
            "get_into_layers",
            [](MmcacheStore &self, const std::string &key, const std::vector<uintptr_t> &buffer_ptrs,
               const std::vector<size_t> &sizes, const int32_t &direct) {
                TP_TRACE_BEGIN(TP_MMC_PYBIND_GET_LAYERS);
                std::vector<void *> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (uintptr_t ptr : buffer_ptrs) {
                    buffers.push_back(reinterpret_cast<void *>(ptr));
                }
                py::gil_scoped_release release;
                auto ret = self.GetIntoLayers(key, buffers, sizes, direct);
                TP_TRACE_END(TP_MMC_PYBIND_GET_LAYERS, 0);
                return ret;
            },
            py::arg("key"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_G2H)
        .def(
            "batch_get_into_layers",
            [](MmcacheStore &self, const std::vector<std::string> &keys,
               const std::vector<std::vector<uintptr_t>> &buffer_ptrs, const std::vector<std::vector<size_t>> &sizes,
               const int32_t &direct) {
                TP_TRACE_BEGIN(TP_MMC_PYBIND_BATCH_GET_LAYERS);
                std::vector<std::vector<void *>> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (auto vec : buffer_ptrs) {
                    std::vector<void *> tmp;
                    for (uintptr_t ptr : vec) {
                        tmp.push_back(reinterpret_cast<void *>(ptr));
                    }
                    buffers.push_back(tmp);
                }
                py::gil_scoped_release release;
                auto ret = self.BatchGetIntoLayers(keys, buffers, sizes, direct);
                TP_TRACE_END(TP_MMC_PYBIND_BATCH_GET_LAYERS, 0);
                return ret;
            },
            py::arg("keys"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_G2H)
        .def(
            "get_local_service_id",
            [](MmcacheStore &self) {
                py::gil_scoped_release release;
                uint32_t localServiceId = std::numeric_limits<uint32_t>::max();
                self.GetLocalServiceId(localServiceId);
                return localServiceId;
            },
            "Get local serviceId")
        .def(
            "put_from",
            [](MmcacheStore &self, const std::string &key, uintptr_t buffer_ptr, size_t size, const int32_t &direct,
               const ReplicateConfig &replicateConfig) {
                py::gil_scoped_release release;
                return self.PutFrom(key, reinterpret_cast<void *>(buffer_ptr), size, direct, replicateConfig);
            },
            py::arg("key"), py::arg("buffer_ptr"), py::arg("size"), py::arg("direct") = SMEMB_COPY_H2G,
            py::arg("replicateConfig") = defaultConfig, "Put object data directly from a pre-allocated buffer")
        .def(
            "batch_put_from",
            [](MmcacheStore &self, const std::vector<std::string> &keys, const std::vector<uintptr_t> &buffer_ptrs,
               const std::vector<size_t> &sizes, const int32_t &direct, const ReplicateConfig &replicateConfig) {
                std::vector<void *> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (uintptr_t ptr : buffer_ptrs) {
                    buffers.push_back(reinterpret_cast<void *>(ptr));
                }
                py::gil_scoped_release release;
                return self.BatchPutFrom(keys, buffers, sizes, direct, replicateConfig);
            },
            py::arg("keys"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_H2G,
            py::arg("replicateConfig") = defaultConfig,
            "Put object data directly from pre-allocated buffers for multiple "
            "keys")
        .def(
            "put_from_layers",
            [](MmcacheStore &self, const std::string &key, const std::vector<uintptr_t> &buffer_ptrs,
               const std::vector<size_t> &sizes, const int32_t &direct, const ReplicateConfig &replicateConfig) {
                TP_TRACE_BEGIN(TP_MMC_PYBIND_PUT_LAYERS);
                std::vector<void *> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (uintptr_t ptr : buffer_ptrs) {
                    buffers.push_back(reinterpret_cast<void *>(ptr));
                }
                py::gil_scoped_release release;
                auto ret = self.PutFromLayers(key, buffers, sizes, direct, replicateConfig);
                TP_TRACE_END(TP_MMC_PYBIND_PUT_LAYERS, 0);
                return ret;
            },
            py::arg("key"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_H2G,
            py::arg("replicateConfig") = defaultConfig)
        .def(
            "batch_put_from_layers",
            [](MmcacheStore &self, const std::vector<std::string> &keys,
               const std::vector<std::vector<uintptr_t>> &buffer_ptrs, const std::vector<std::vector<size_t>> &sizes,
               const int32_t &direct, const ReplicateConfig &replicateConfig) {
                TP_TRACE_BEGIN(TP_MMC_PYBIND_BATCH_PUT_LAYERS);
                std::vector<std::vector<void *>> buffers;
                buffers.reserve(buffer_ptrs.size());
                for (auto vec : buffer_ptrs) {
                    std::vector<void *> tmp;
                    for (uintptr_t ptr : vec) {
                        tmp.push_back(reinterpret_cast<void *>(ptr));
                    }
                    buffers.push_back(tmp);
                }
                py::gil_scoped_release release;
                auto ret = self.BatchPutFromLayers(keys, buffers, sizes, direct, replicateConfig);
                TP_TRACE_END(TP_MMC_PYBIND_BATCH_PUT_LAYERS, 0);
                return ret;
            },
            py::arg("keys"), py::arg("buffer_ptrs"), py::arg("sizes"), py::arg("direct") = SMEMB_COPY_H2G,
            py::arg("replicateConfig") = defaultConfig)
        .def(
            "put",
            [](MmcacheStore &self, const std::string &key, const py::buffer &buf,
               const ReplicateConfig &replicateConfig) {
                py::buffer_info info = buf.request(false);
                mmc_buffer buffer = {.addr = reinterpret_cast<uint64_t>(info.ptr),
                                     .type = MEDIA_DRAM,
                                     .offset = 0,
                                     .len = static_cast<uint64_t>(info.size)};
                py::gil_scoped_release release;
                return self.Put(key, buffer, replicateConfig);
            },
            py::arg("key"), py::arg("buf"), py::arg("replicateConfig") = defaultConfig)
        .def(
            "put_batch",
            [](MmcacheStore &self, const std::vector<std::string> &keys, const std::vector<py::buffer> &buffers,
               const ReplicateConfig &replicateConfig) {
                // Convert pybuffers to spans without copying
                std::vector<py::buffer_info> infos;
                std::vector<mmc_buffer> bufs;
                infos.reserve(buffers.size());
                bufs.reserve(buffers.size());

                for (const auto &buf : buffers) {
                    infos.emplace_back(buf.request(false));
                    const auto &info = infos.back();
                    bufs.emplace_back(mmc_buffer{.addr = reinterpret_cast<uint64_t>(info.ptr),
                                                 .type = MEDIA_DRAM,
                                                 .offset = 0,
                                                 .len = static_cast<uint64_t>(info.size)});
                }
                py::gil_scoped_release release;
                return self.PutBatch(keys, bufs, replicateConfig);
            },
            py::arg("keys"), py::arg("values"), py::arg("replicateConfig") = defaultConfig)
        .def("get",
             [](MmcacheStore &self, const std::string &key) {
                 mmc_buffer buffer = self.Get(key);
                 py::gil_scoped_acquire acquire_gil;
                 if (buffer.addr == 0) {
                     return py::bytes("");
                 } else {
                     auto dataPtr = reinterpret_cast<char *>(buffer.addr);
                     std::shared_ptr<char[]> dataSharedPtr(dataPtr, [](char *p) { delete[] p; });
                     return py::bytes(dataPtr, buffer.len);
                 }
             })
        .def("get_batch", [](MmcacheStore &self, const std::vector<std::string> &keys) {
            const auto kNullString = pybind11::bytes("\\0", 0);
            {
                py::gil_scoped_release release_gil;
                auto bufferArray = self.GetBatch(keys);

                py::gil_scoped_acquire acquire_gil;
                std::vector<pybind11::bytes> results;
                results.reserve(keys.size());

                for (size_t i = 0; i < keys.size(); ++i) {
                    mmc_buffer buffer = bufferArray[i];
                    if (buffer.addr == 0) {
                        results.emplace_back(kNullString);
                        continue;
                    }
                    auto dataPtr = reinterpret_cast<char *>(buffer.addr);
                    std::shared_ptr<char[]> dataSharedPtr(dataPtr, [](char *p) { delete[] p; });
                    results.emplace_back(py::bytes(dataPtr, buffer.len));
                }
                return results;
            }
        });
}