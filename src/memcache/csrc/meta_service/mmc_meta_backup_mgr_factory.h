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

#ifndef MF_HYBRID_MMC_META_BACKUP_MGR_FACTORY_H
#define MF_HYBRID_MMC_META_BACKUP_MGR_FACTORY_H
#include "mmc_meta_backup_mgr.h"
#include "mmc_meta_backup_mgr_default.h"
namespace ock {
namespace mmc {
class MMCMetaBackUpMgrFactory : public MmcReferable {
public:
    static MmcRef<MMCMetaBackUpMgr> GetInstance(const std::string inputName = "")
    {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        std::string key = inputName;
        auto it = instances_.find(key);
        if (it == instances_.end()) {
            MmcRef<MMCMetaBackUpMgrDefault> instance = new (std::nothrow) MMCMetaBackUpMgrDefault();
            if (instance == nullptr) {
                MMC_LOG_ERROR("new MetaNetClient failed, probably out of memory");
                return nullptr;
            }
            instances_[key] = instance.Get();
            return instance.Get();
        }
        return it->second;
    }

private:
    static std::map<std::string, MmcRef<MMCMetaBackUpMgr>> instances_;
    static std::mutex instanceMutex_;
};
} // namespace mmc
} // namespace ock
#endif // MF_HYBRID_MMC_META_BACKUP_MGR_FACTORY_H