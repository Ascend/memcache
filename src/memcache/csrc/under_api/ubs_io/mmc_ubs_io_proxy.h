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
#ifndef MEM_FABRIC_MMC_UBS_IO_PROXY_H
#define MEM_FABRIC_MMC_UBS_IO_PROXY_H

#include <mutex>
#include <map>
#include <vector>
#include <string>
#include "mmc_logger.h"
#include "mmc_def.h"
#include "mmc_types.h"
#include "mmc_ref.h"

namespace ock {
namespace mmc {

class MmcUbsIoProxy : public MmcReferable {
public:
    explicit MmcUbsIoProxy(const std::string &name) : name_(name) {}
    ~MmcUbsIoProxy() override = default;

    // 删除拷贝构造函数和赋值运算符
    MmcUbsIoProxy(const MmcUbsIoProxy&) = delete;
    MmcUbsIoProxy& operator=(const MmcUbsIoProxy&) = delete;

    Result InitUbsIo(int32_t deviceId = -1);
    void DestroyUbsIo();
    Result Put(const std::string &key, void *buf, size_t length);
    Result Get(const std::string &key, void *buf, size_t length);
    Result Exist(const std::string &key);
    Result Delete(const std::string &key);
    Result GetLength(const std::string &key, size_t &length);
    Result BatchPut(const std::vector<std::string> &keys, const std::vector<void *> &bufs,
        const std::vector<size_t> &lengths, std::vector<int> &results);
    Result BatchGet(const std::vector<std::string> &keys, void **bufs,
        std::vector<size_t> &lengths, std::vector<int> &results);
    Result BatchGetWithHBM(const std::vector<std::string> &keys, std::vector<std::vector<void*>>& npuBufAddrs,
        std::vector<std::vector<size_t>>& npuBufLengths, std::vector<int> &results);
    Result BatchExist(const std::vector<std::string> &keys, bool *results);
    Result BatchDelete(const std::vector<std::string> &keys, std::vector<int32_t> &results);
    Result BatchGetLength(const std::vector<std::string> &keys, std::vector<size_t> &lengths,
        std::vector<int32_t> &results);
    Result BatchGetFree(void **bufs, int keysCount);

private:
    std::string name_;
    bool started_ = false;
    std::mutex mutex_;
};

using MmcUbsIoProxyPtr = MmcRef<MmcUbsIoProxy>;

class MmcUbsIoProxyFactory : public MmcReferable {
public:
    static MmcUbsIoProxyPtr GetInstance(const std::string& key = "")
    {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        const auto it = instances_.find(key);
        if (it == instances_.end()) {
            MmcRef<MmcUbsIoProxy> instance = new (std::nothrow)MmcUbsIoProxy("ubsIoProxy");
            if (instance == nullptr) {
                MMC_LOG_ERROR("new object failed, probably out of memory");
                return nullptr;
            }
            instances_[key] = instance;
            return instance;
        }
        return it->second;
    }

private:
    static std::map<std::string, MmcRef<MmcUbsIoProxy>> instances_;
    static std::mutex instanceMutex_;
};
}
}

#endif  // MEM_FABRIC_MMC_UBS_IO_PROXY_H
