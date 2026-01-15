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

#ifndef MF_HYBRID_MMC_META_BACKUP_MGR_H
#define MF_HYBRID_MMC_META_BACKUP_MGR_H

#include <map>
#include "mmc_ref.h"
#include "mmc_types.h"
#include "mmc_blob_common.h"

namespace ock {
namespace mmc {

struct MMCMetaBackUpConf : public MmcReferable {};
using MMCMetaBackUpConfPtr = MmcRef<MMCMetaBackUpConf>;

class MMCMetaBackUpMgr : public MmcReferable {
public:
    virtual Result Start(MMCMetaBackUpConfPtr &confPtr) = 0;

    virtual void Stop() = 0;

    virtual Result Add(const std::string &key, MmcMemBlobDesc &blobDesc) = 0;

    virtual Result Remove(const std::string &key, MmcMemBlobDesc &blobDesc) = 0;

    virtual Result Load(std::map<std::string, MmcMemBlobDesc> &blobMap) = 0;
};
using MMCMetaBackUpMgrPtr = MmcRef<MMCMetaBackUpMgr>;
} // namespace mmc
} // namespace ock
#endif // MF_HYBRID_MMC_META_BACKUP_MGR_H