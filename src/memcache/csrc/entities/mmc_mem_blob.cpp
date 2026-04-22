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
#include "mmc_mem_blob.h"
namespace ock {
namespace mmc {

const StateTransTable MmcMemBlob::stateTransTable_ = BlobStateMachine::GetGlobalTransTable();

Result MmcMemBlob::UpdateState(const std::string &key, uint32_t rankId, uint32_t operateId, BlobActionResult ret)
{
    auto curStateIter = stateTransTable_.find(state_);
    if (curStateIter == stateTransTable_.end()) {
        MMC_LOG_ERROR("Cannot update state:" << ret << "! The current state " << state_
                                             << " is not in the stateTransTable! key:" << key);
        return MMC_UNMATCHED_STATE;
    }

    const auto retIter = curStateIter->second.find(ret);
    if (retIter == curStateIter->second.end()) {
        MMC_LOG_ERROR("Cannot find " << ret << "from " << state_ << "! key:" << key);
        return MMC_UNMATCHED_RET;
    }

    MMC_LOG_DEBUG("update [" << key << "] state from " << std::to_string(state_) << " to ("
                             << std::to_string(retIter->second.state_) << ")");

    auto oldState = state_;
    state_ = retIter->second.state_;
    if (retIter->second.action_) {
        auto res = retIter->second.action_(metaLeaseManager_, rankId, operateId);
        if (res != MMC_OK) {
            MMC_LOG_ERROR("Blob update current state is " << std::to_string(state_) << " by ret(" << std::to_string(ret)
                                                          << ") failed! key" << key << ", res=" << res);
            return res;
        }
    }

    if (oldState == ALLOCATED && ret == MMC_WRITE_OK) {
        auto bakRet = Backup(key);
        if (bakRet != MMC_OK) {
            MMC_LOG_ERROR("backup failed " << bakRet << " for key:" << key);
            // 备份失败是可以容忍的，不应该打断update流程
        }
    }

    return MMC_OK;
}

Result MmcMemBlob::UpdateState(const BlobActionResult ret)
{
    auto curStateIter = stateTransTable_.find(state_);
    if (curStateIter == stateTransTable_.end()) {
        MMC_LOG_ERROR("Cannot update state:" << ret << "! The current state " << state_
                                             << " is not in the stateTransTable!");
        return MMC_UNMATCHED_STATE;
    }

    const auto retIter = curStateIter->second.find(ret);
    if (retIter == curStateIter->second.end()) {
        MMC_LOG_ERROR("cannot find " << std::to_string(ret) << " from " << std::to_string(state_));

        return MMC_UNMATCHED_RET;
    }

    state_ = retIter->second.state_;
    // 仅支持 CopyBlob 流程使用！！blob申请时没有申请租约，所以也不需要释放租约
    // 但是没有进行备份动作，这个blob没有能进行备份，v1.0.0不支持多级淘汰，此问题不存在
    return MMC_OK;
}

Result MmcMemBlob::Backup(const std::string &key)
{
    MMCMetaBackUpMgrPtr mmcBackupPtr = MMCMetaBackUpMgrFactory::GetInstance("DefaultMetaBackup");
    if (mmcBackupPtr == nullptr) {
        MMC_LOG_ERROR("key " << key << " meta back up failed");
        return MMC_META_BACKUP_ERROR;
    }
    MmcMemBlobDesc desc = GetDesc();
    MMC_LOG_DEBUG("Backup add " << key << " for blob: " << desc);
    return mmcBackupPtr->Add(key, desc);
}

Result MmcMemBlob::BackupRemove(const std::string &key)
{
    MMCMetaBackUpMgrPtr mmcBackupPtr = MMCMetaBackUpMgrFactory::GetInstance("DefaultMetaBackup");
    if (mmcBackupPtr == nullptr) {
        MMC_LOG_ERROR("key " << key << " meta back up failed");
        return MMC_META_BACKUP_ERROR;
    }
    MmcMemBlobDesc desc = GetDesc();
    MMC_LOG_DEBUG("Backup remove " << key << " for blob: " << desc);
    return mmcBackupPtr->Remove(key, desc);
}

} // namespace mmc
} // namespace ock