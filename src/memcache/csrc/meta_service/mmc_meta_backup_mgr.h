/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef MF_HYBRID_MMC_META_BACKUP_MGR_H
#define MF_HYBRID_MMC_META_BACKUP_MGR_H

#include "mmc_ref.h"
#include "mmc_types.h"
#include "mmc_blob_common.h"

namespace ock {
namespace mmc {

struct MMCMetaBackUpConf : public MmcReferable {
};
using MMCMetaBackUpConfPtr = MmcRef<MMCMetaBackUpConf>;

class MMCMetaBackUpMgr : public MmcReferable {
public:

    virtual Result Start(MMCMetaBackUpConfPtr &confPtr) = 0;

    virtual void Stop() = 0;

    virtual Result Add(const std::string &key, MmcMemBlobDesc &blobDesc) = 0;

    virtual Result Remove(const std::string &key, MmcMemBlobDesc &blobDesc) = 0;
};
using MMCMetaBackUpMgrPtr = MmcRef<MMCMetaBackUpMgr>;
}
}
#endif // MF_HYBRID_MMC_META_BACKUP_MGR_H
