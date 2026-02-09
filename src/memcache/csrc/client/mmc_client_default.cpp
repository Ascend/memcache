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

#include "mmc_client_default.h"
#include "mmc_msg_client_meta.h"
#include "mmc_mem_obj_meta.h"
#include "mmc_bm_proxy.h"
#include "mmc_montotonic.h"
#include "mmc_ptracer.h"

namespace ock {
namespace mmc {
constexpr int CLIENT_THREAD_COUNT = 2;
constexpr uint32_t MMC_REGISTER_SET_MARK_BIT = 1U;
constexpr uint32_t MMC_REGISTER_SET_LEFT_MARK = 1U;
constexpr int32_t MMC_BATCH_TRANSPORT = 1U;
constexpr int32_t MMC_ASYNC_TRANSPORT = 2U;
constexpr uint32_t KEY_MAX_LENTH = 256U;

MmcClientDefault *MmcClientDefault::gClientHandler = nullptr;
std::mutex MmcClientDefault::gClientHandlerMtx;

Result MmcClientDefault::Start(const mmc_client_config_t &config)
{
    MMC_LOG_INFO("Starting client " << name_);
    std::lock_guard<std::mutex> guard(mutex_);
    if (started_) {
        MMC_LOG_INFO("MetaService " << name_ << " already started");
        return MMC_OK;
    }
    bmProxy_ = MmcBmProxyFactory::GetInstance("bmProxyDefault");
    MMC_ASSERT_RETURN(bmProxy_ != nullptr, MMC_MALLOC_FAILED);
    rankId_ = bmProxy_->RankId();

    threadPool_ = MmcMakeRef<MmcThreadPool>("client_pool", 1);
    MMC_ASSERT_RETURN(threadPool_ != nullptr, MMC_MALLOC_FAILED);
    MMC_RETURN_ERROR(threadPool_->Start(), "thread pool start failed");

    readThreadPool_ = MmcMakeRef<MmcThreadPool>("read_pool", config.readThreadPoolNum);
    aggregateIO_ = config.aggregateIO;
    aggregateNum_ = static_cast<size_t>(config.aggregateNum);
    MMC_ASSERT_RETURN(readThreadPool_ != nullptr, MMC_MALLOC_FAILED);
    MMC_RETURN_ERROR(readThreadPool_->Start(), "read thread pool start failed");

    writeThreadPool_ = MmcMakeRef<MmcThreadPool>("write_pool", config.writeThreadPoolNum);
    MMC_ASSERT_RETURN(writeThreadPool_ != nullptr, MMC_MALLOC_FAILED);
    MMC_RETURN_ERROR(writeThreadPool_->Start(), "write thread pool start failed");

    MMC_ASSERT_RETURN(memchr(config.discoveryURL, '\0', DISCOVERY_URL_SIZE) != nullptr, MMC_INVALID_PARAM);
    auto tmpNetClient = MetaNetClientFactory::GetInstance(config.discoveryURL, "MetaClientCommon").Get();
    MMC_ASSERT_RETURN(tmpNetClient != nullptr, MMC_NEW_OBJECT_FAILED);
    if (!tmpNetClient->Status()) {
        NetEngineOptions options;
        options.name = name_;
        options.threadCount = CLIENT_THREAD_COUNT;
        options.rankId = rankId_;
        options.startListener = false;
        options.tlsOption = config.tlsConfig;
        options.logLevel = config.logLevel;
        options.logFunc = config.logFunc;
        MMC_RETURN_ERROR(tmpNetClient->Start(options), "Failed to start net server of local service " << name_);
        MMC_RETURN_ERROR(tmpNetClient->Connect(config.discoveryURL),
                         "Failed to connect net server of local service " << name_);
    }

    metaNetClient_ = tmpNetClient;
    rpcRetryTimeOut_ = config.rpcRetryTimeOut;
    started_ = true;
    return MMC_OK;
}

void MmcClientDefault::Stop()
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (!started_) {
        MMC_LOG_WARN("MmcClientDefault has not been started");
        return;
    }
    if (readThreadPool_ != nullptr) {
        readThreadPool_->Destroy();
    }
    if (writeThreadPool_ != nullptr) {
        writeThreadPool_->Destroy();
    }
    if (threadPool_ != nullptr) {
        threadPool_->Destroy();
    }
    if (metaNetClient_ != nullptr) {
        metaNetClient_->Stop();
        metaNetClient_ = nullptr;
        MMC_LOG_INFO("MetaNetClient stopped.");
    }
    started_ = false;
}

const std::string &MmcClientDefault::Name() const
{
    return name_;
}

Result MmcClientDefault::Put(const char *key, mmc_buffer *buf, mmc_put_options &options, uint32_t flags)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (buf == nullptr || key == nullptr || key[0] == '\0' || strnlen(key, KEY_MAX_LENTH + 1) == KEY_MAX_LENTH + 1) {
        MMC_LOG_ERROR("Invalid arguments");
        return MMC_ERROR;
    }

    MmcBufferArray buffArr{};
    buffArr.AddBuffer(*buf);
    return Put(key, buffArr, options, flags);
}

Result MmcClientDefault::PrepareAllocOpt(const MmcBufferArray &bufArr, const mmc_put_options &options, uint32_t flags,
                                         AllocOptions &allocOpt)
{
    allocOpt.blobSize_ = bufArr.TotalSize();
    allocOpt.numBlobs_ = std::max<uint16_t>(options.replicaNum, 1u);
    allocOpt.mediaType_ = MEDIA_NONE;
    allocOpt.flags_ = flags;

    std::copy_if(std::begin(options.preferredLocalServiceIDs), std::end(options.preferredLocalServiceIDs),
                 std::back_inserter(allocOpt.preferredRank_), [](const int32_t x) { return x >= 0; });
     // preferredRank_数量需要小于等于replicaNum， 具体参考 MmcGlobalAllocator Alloc方法说明
    if (allocOpt.preferredRank_.size() > allocOpt.numBlobs_ || allocOpt.numBlobs_ > MAX_BLOB_COPIES) {
        MMC_LOG_ERROR("preferredRank size:" << allocOpt.preferredRank_.size()
                                            << " is greater than replicaNum:" << allocOpt.numBlobs_);
        return MMC_INVALID_PARAM;
    }

    if (!allocOpt.preferredRank_.empty()) {
        allocOpt.flags_ |= ALLOC_FORCE_BY_RANK;
    } else {
        allocOpt.preferredRank_.push_back(RankId(options.policy));
    }
    return MMC_OK;
}

Result MmcClientDefault::Put(const std::string &key, const MmcBufferArray &bufArr, mmc_put_options &options,
                             uint32_t flags)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);
    MMC_ASSERT_RETURN(!bufArr.Buffers().empty(), MMC_ERROR);
    uint64_t operateId = GenerateOperateId(rankId_);
    AllocRequest request{key, {}, operateId};
    MMC_VALIDATE_RETURN(PrepareAllocOpt(bufArr, options, flags, request.options_) == MMC_OK, "put error", MMC_ERROR);
    AllocResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " alloc " << key << " failed");
    if (response.result_ == MMC_DUPLICATED_OBJECT) {
        return response.result_;
    }
    MMC_RETURN_ERROR(response.result_, "client " << name_ << " alloc " << key << " failed");
    if (response.numBlobs_ == 0 || response.numBlobs_ != response.blobs_.size()) {
        MMC_LOG_ERROR("client " << name_ << " alloc " << key << " failed, blobs size:" << response.blobs_.size()
                                << ", numBlob:" << response.numBlobs_);
        return MMC_ERROR;
    }

    Result result = MMC_OK;
    BatchUpdateRequest updateRequest{};
    updateRequest.operateId_ = operateId;
    for (uint8_t i = 0; i < response.numBlobs_; i++) {
        auto blob = response.blobs_[i];
        MMC_LOG_DEBUG("Attempting to put to blob " << static_cast<int>(i) << " key " << key);
        auto ret = bmProxy_->BatchPut(bufArr, blob);
        if (ret != MMC_OK) {
            MMC_LOG_ERROR("client " << name_ << " put " << key << " blob rank: " << blob.rank_
                                    << ", media: " << blob.mediaType_ << " failed, ret: " << ret);
            updateRequest.actionResults_.push_back(MMC_WRITE_FAIL);
            result = ret;
        } else {
            updateRequest.actionResults_.push_back(MMC_WRITE_OK);
        }

        updateRequest.keys_.push_back(key);
        updateRequest.ranks_.push_back(blob.rank_);
        updateRequest.mediaTypes_.push_back(blob.mediaType_);
    }
    SyncUpdateState(updateRequest);
    return result;
}

Result MmcClientDefault::BatchPut(const std::vector<std::string> &keys, const std::vector<mmc_buffer> &bufs,
                                  mmc_put_options &options, uint32_t flags, std::vector<int> &batchResult)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (keys.empty() || bufs.empty() || keys.size() != bufs.size()) {
        MMC_LOG_ERROR("client " << name_ << " batch get failed: keys size:" << keys.size()
                                << ", bufs size:" << bufs.size());
        return MMC_INVALID_PARAM;
    }

    std::vector<MmcBufferArray> bufferArrays{};
    bufferArrays.reserve(bufs.size());
    for (const auto &buf : bufs) {
        MmcBufferArray bufArr{};
        bufArr.AddBuffer(buf);
        bufferArrays.emplace_back(bufArr);
    }

    return BatchPut(keys, bufferArrays, options, flags, batchResult);
}

Result MmcClientDefault::BatchPut(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs,
                                  mmc_put_options &options, uint32_t flags, std::vector<int> &batchResult)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (keys.empty() || bufArrs.empty() || keys.size() != bufArrs.size()) {
        MMC_LOG_ERROR("client " << name_ << " batch get failed: keys size:" << keys.size()
                                << ", bufArrs size:" << bufArrs.size());
        return MMC_INVALID_PARAM;
    }

    // alloc blobs
    uint64_t operateId = GenerateOperateId(rankId_);
    BatchAllocRequest request(keys, {}, flags, operateId);
    for (const auto &bufArr : bufArrs) {
        AllocOptions tmpAllocOptions{};
        MMC_VALIDATE_RETURN(PrepareAllocOpt(bufArr, options, flags, tmpAllocOptions) == MMC_OK, "option param error",
                            MMC_ERROR);
        request.options_.emplace_back(std::move(tmpAllocOptions));
    }
    BatchAllocResponse allocResponse{};
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, allocResponse, rpcRetryTimeOut_), "batch put alloc failed");
    // check alloc result
    if (keys.size() != allocResponse.blobs_.size() || keys.size() != allocResponse.numBlobs_.size() ||
        keys.size() != allocResponse.results_.size()) {
        MMC_LOG_ERROR("Mismatch in number of keys and allocated blobs");
        return MMC_ERROR;
    }

    // put obj
    batchResult.resize(keys.size(), MMC_OK);
    auto ret = PutData2Blobs(keys, bufArrs, allocResponse, batchResult);

    // update blob state
    BatchUpdateRequest updateRequest{};
    updateRequest.operateId_ = operateId;
    for (size_t i = 0; i < keys.size(); ++i) {
        for (const auto &blob : allocResponse.blobs_[i]) {
            updateRequest.keys_.push_back(keys[i]);
            updateRequest.ranks_.push_back(blob.rank_);
            updateRequest.mediaTypes_.push_back(blob.mediaType_);
            updateRequest.actionResults_.push_back(batchResult[i] == 0 ? MMC_WRITE_OK : MMC_WRITE_FAIL);
        }
    }
    SyncUpdateState(updateRequest); // 写需要同步更新，异步更新会出现立即读查询blob不可读的情况

    if (ret != MMC_OK) {
        MMC_LOG_ERROR("client " << name_ << " batch put failed: " << ret);
    }
    return ret;
}

Result MmcClientDefault::Get(const char *key, mmc_buffer *buf, uint32_t flags)
{
    if (buf == nullptr || key == nullptr || key[0] == '\0' || strnlen(key, KEY_MAX_LENTH + 1) == KEY_MAX_LENTH + 1) {
        MMC_LOG_ERROR("Invalid arguments");
        return MMC_ERROR;
    }

    MmcBufferArray bufArr{};
    bufArr.AddBuffer(*buf);
    return Get(key, bufArr, flags);
}

Result MmcClientDefault::Get(const std::string &key, const MmcBufferArray &bufArr, uint32_t flags)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    uint64_t operateId = GenerateOperateId(rankId_);
    GetRequest request{key, rankId_, operateId, true};
    AllocResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " get " << key << " failed");
    if (response.numBlobs_ == 0 || response.blobs_.empty()) {
        MMC_LOG_ERROR("client " << name_ << " get " << key
                                << " failed, numblob is:" << static_cast<uint64_t>(response.numBlobs_));
        return MMC_ERROR;
    }
    auto &blob = response.blobs_[0];
    auto ret = bmProxy_->BatchGet(bufArr, blob);

    BatchUpdateRequest updateRequest{};
    updateRequest.actionResults_.push_back(MMC_READ_FINISH);
    updateRequest.keys_.push_back(key);
    updateRequest.ranks_.push_back(blob.rank_);
    updateRequest.mediaTypes_.push_back(blob.mediaType_);
    updateRequest.operateId_ = operateId;
    AsyncUpdateState(updateRequest);

    if (ret != MMC_OK) {
        MMC_LOG_ERROR("client " << name_ << " get " << key << " read data failed.");
        return ret;
    }
    return MMC_OK;
}

Result MmcClientDefault::BatchGet(const std::vector<std::string> &keys, std::vector<mmc_buffer> &bufs, uint32_t flags,
                                  std::vector<int> &batchResult)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);
    if ((keys.empty() || bufs.empty() || keys.size() != bufs.size())) {
        MMC_LOG_ERROR("client " << name_ << " batch get failed: keys size:" << keys.size()
                                << ", bufArrs size:" << bufs.size());
        return MMC_INVALID_PARAM;
    }

    std::vector<MmcBufferArray> bufferArrays{};
    bufferArrays.reserve(bufs.size());
    for (const auto &buf : bufs) {
        MmcBufferArray bufArr{};
        bufArr.AddBuffer(buf);
        bufferArrays.emplace_back(bufArr);
    }
    return BatchGet(keys, bufferArrays, flags, batchResult);
}

Result MmcClientDefault::BatchGet(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs,
                                  uint32_t flags, std::vector<int> &batchResult)
{
    MMC_VALIDATE_RETURN(bmProxy_ != nullptr, "BmProxy is null", MMC_CLIENT_NOT_INIT);
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if ((keys.empty() || bufArrs.empty() || keys.size() != bufArrs.size())) {
        MMC_LOG_ERROR("client " << name_ << " batch get failed: keys size:" << keys.size()
                                << ", bufArrs size:" << bufArrs.size());
        return MMC_INVALID_PARAM;
    }
    // get meta
    batchResult.resize(keys.size(), MMC_ERROR);
    const uint64_t operateId = GenerateOperateId(rankId_);
    BatchGetRequest request{keys, rankId_, operateId};
    BatchAllocResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " batch get failed");
    // check rsp meta
    if (response.blobs_.size() != keys.size() || response.numBlobs_.size() != keys.size()) {
        MMC_LOG_ERROR("client " << name_ << " batch get response size mismatch: expected " << keys.size() << ", got "
                                << response.blobs_.size());
        return MMC_ERROR;
    }
    // read data
    MediaType mediaType = MEDIA_NONE;
    std::vector<std::tuple<uint32_t, uint32_t, std::future<int32_t>>> futures;
    size_t startKeyIndex = 0;
    BatchCopyDesc copyDesc{};
    for (size_t i = 0; i < keys.size(); ++i) {
        const MmcBufferArray &bufArr = bufArrs[i];
        const auto &blobs = response.blobs_[i];
        uint8_t numBlobs = response.numBlobs_[i];
        if (numBlobs <= 0 || blobs.empty() || blobs.size() != numBlobs) {
            MMC_LOG_ERROR("client " << name_ << " batch get failed for key " << keys[i]
                                    << ", blob:" << std::to_string(numBlobs) << ", size:" << blobs.size());
            continue;
        }
        if (bufArr.TotalSize() != blobs[0].size_) {
            MMC_LOG_ERROR("client " << name_ << " batch get failed for key " << keys[i]
                                    << ", blob:" << std::to_string(numBlobs) << ", size:" << blobs.size()
                                    << " key size:" << bufArr.TotalSize());
            continue;
        }

        BatchCopyDesc keyCopyDesc{};
        batchResult[i] = PrepareBlob(bufArr, blobs[0], mediaType, keyCopyDesc, true);
        if (batchResult[i] != MMC_OK) {
            MMC_LOG_ERROR("client " << name_ << " prepare blob failed for key " << keys[i]);
            continue;
        }
        copyDesc.Append(keyCopyDesc);
        /**
         * 1. 开启聚合是避免单流读取地址太少，避免调用栈开销
         * 2. 也要避免聚合io聚合地址太多，不利于并发，实测单流性能低于多流
         */
        if (aggregateIO_ && (copyDesc.sizes.size() < aggregateNum_) && (i != (keys.size() - 1))) {
            continue;
        }
        auto future = SubmitGetTask(copyDesc, mediaType, !(startKeyIndex == 0 && i == (keys.size() - 1)));
        futures.push_back(std::make_tuple(startKeyIndex, i, std::move(future)));
        copyDesc.Clear();
        startKeyIndex = i + 1;
    }
    //  last key is invalid, task not submit
    if (!copyDesc.sizes.empty()) {
        auto future = SubmitGetTask(copyDesc, mediaType, !(startKeyIndex == 0));
        futures.push_back(std::make_tuple(startKeyIndex, (keys.size() - 1), std::move(future)));
    }

    TP_TRACE_BEGIN(TP_MMC_LOCAL_GET_WAIT_FUTURE);
    WaitFeatures(futures, batchResult);
    TP_TRACE_END(TP_MMC_LOCAL_GET_WAIT_FUTURE, 0);
    // update read state
    BatchUpdateRequest updateRequest{};
    updateRequest.operateId_ = operateId;
    for (size_t i = 0; i < keys.size(); ++i) {
        for (const auto &blob : response.blobs_[i]) {
            updateRequest.keys_.push_back(keys[i]);
            updateRequest.ranks_.push_back(blob.rank_);
            updateRequest.mediaTypes_.push_back(blob.mediaType_);
            updateRequest.actionResults_.push_back(MMC_READ_FINISH);
        }
    }
    AsyncUpdateState(updateRequest);
    return MMC_OK;
}

Result MmcClientDefault::Remove(const char *key, uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    RemoveRequest request{key};
    Response response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " remove " << key << " failed");
    return response.ret_;
}

Result MmcClientDefault::BatchRemove(const std::vector<std::string> &keys, std::vector<Result> &remove_results,
                                     uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    BatchRemoveRequest request{keys};
    BatchRemoveResponse response;

    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " BatchRemove failed");

    if (response.results_.size() != keys.size()) {
        MMC_LOG_ERROR("BatchRemove response size mismatch. Expected: " << keys.size()
                                                                       << ", Got: " << response.results_.size());
        std::fill(remove_results.begin(), remove_results.end(), MMC_ERROR);
        return MMC_ERROR;
    }

    remove_results = response.results_;
    return MMC_OK;
}

Result MmcClientDefault::RemoveAll(uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    RemoveAllRequest request{};
    Response response;

    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " RemoveAll failed");

    return MMC_OK;
}

Result MmcClientDefault::IsExist(const std::string &key, uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (key.empty()) {
        MMC_LOG_ERROR("Get empty key!");
        return MMC_INVALID_PARAM;
    }

    IsExistRequest request{key};
    Response response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " IsExist " << key << " failed");
    return response.ret_;
}

Result MmcClientDefault::BatchIsExist(const std::vector<std::string> &keys, std::vector<int32_t> &exist_results,
                                      uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (keys.empty()) {
        MMC_LOG_ERROR("Get empty keys!");
        return MMC_INVALID_PARAM;
    }

    BatchIsExistRequest request{keys};
    BatchIsExistResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " BatchIsExist failed");

    if (response.results_.size() != keys.size()) {
        MMC_LOG_ERROR("BatchIsExist response size mismatch. Expected: " << keys.size()
                                                                        << ", Got: " << response.results_.size());
        std::fill(exist_results.begin(), exist_results.end(), MMC_ERROR);
        return MMC_ERROR;
    }

    exist_results = response.results_;
    return MMC_OK;
}

Result MmcClientDefault::Query(const std::string &key, mmc_data_info &query_info, uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (key.empty()) {
        MMC_LOG_ERROR("Get empty key!");
        return MMC_INVALID_PARAM;
    }

    QueryRequest request{key};
    QueryResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " Query " << key << " failed");
    query_info.size = response.queryInfo_.size_;
    query_info.prot = response.queryInfo_.prot_;
    query_info.numBlobs =
        response.queryInfo_.numBlobs_ > MAX_BLOB_COPIES ? MAX_BLOB_COPIES : response.queryInfo_.numBlobs_;
    query_info.valid = response.queryInfo_.valid_;
    for (int i = 0; i < query_info.numBlobs && i < MAX_BLOB_COPIES; i++) {
        query_info.ranks[i] = response.queryInfo_.blobRanks_[i];
        query_info.types[i] = response.queryInfo_.blobTypes_[i];
    }
    return MMC_OK;
}

Result MmcClientDefault::BatchQuery(const std::vector<std::string> &keys, std::vector<mmc_data_info> &query_infos,
                                    uint32_t flags) const
{
    MMC_VALIDATE_RETURN(metaNetClient_ != nullptr, "MetaNetClient is null", MMC_CLIENT_NOT_INIT);

    if (keys.empty()) {
        MMC_LOG_ERROR("Get empty keys!");
        return MMC_INVALID_PARAM;
    }

    BatchQueryRequest request{keys};
    BatchQueryResponse response;
    MMC_RETURN_ERROR(metaNetClient_->SyncCall(request, response, rpcRetryTimeOut_),
                     "client " << name_ << " BatchIsExist failed");

    if (response.batchQueryInfos_.size() != keys.size()) {
        MMC_LOG_ERROR("BatchQuery get a response with mismatched size ("
                      << response.batchQueryInfos_.size() << "), should get size (" << keys.size() << ").");
        MemObjQueryInfo info_fill;
        query_infos.resize(keys.size(), {});
        return MMC_ERROR;
    }

    for (const auto &info : response.batchQueryInfos_) {
        mmc_data_info outInfo{};
        outInfo.valid = info.valid_;
        if (!outInfo.valid) {
            query_infos.push_back(outInfo);
            continue;
        }

        for (int i = 0; i < info.numBlobs_ && i < MAX_BLOB_COPIES; i++) {
            outInfo.ranks[i] = info.blobRanks_[i];
            outInfo.types[i] = info.blobTypes_[i];
        }
        outInfo.size = info.size_;
        outInfo.prot = info.prot_;
        outInfo.numBlobs = info.numBlobs_;
        query_infos.push_back(outInfo);
    }
    return MMC_OK;
}

void MmcClientDefault::WaitFeatures(std::vector<std::tuple<uint32_t, uint32_t, std::future<int32_t>>> &futures,
                                    std::vector<int> &batchResult)
{
    for (auto &tuple : futures) {
        auto res = std::get<2>(tuple).get();
        if (res == MMC_OK) {
            continue;
        }
        auto start = std::get<0>(tuple);
        auto end = std::get<1>(tuple);
        MMC_LOG_ERROR("batch key from " << start << " to " << end << " failed, error code " << res);
        for (size_t i = start; i <= end && i < batchResult.size(); i++) {
            if (batchResult[i] == MMC_OK) {
                batchResult[i] = res;
            }
        }
    }
}

void MmcClientDefault::SyncUpdateState(BatchUpdateRequest &updateRequest)
{
    TP_TRACE_BEGIN(TP_MMC_LOCAL_BATCH_UPDATE);
    BatchUpdateResponse updateResponse;
    Result updateResult = metaNetClient_->SyncCall(updateRequest, updateResponse, rpcRetryTimeOut_);
    TP_TRACE_END(TP_MMC_LOCAL_BATCH_UPDATE, updateResult);
    if (updateResult != MMC_OK || updateResponse.results_.size() != updateRequest.keys_.size()) {
        MMC_LOG_ERROR("client " << name_ << " batch get update failed:" << updateResult << ", key size:"
                                << updateRequest.keys_.size() << ", ret size:" << updateResponse.results_.size());
    } else {
        for (size_t i = 0; i < updateRequest.keys_.size() && i < updateRequest.keys_.size(); ++i) {
            if (updateResponse.results_[i] != MMC_OK) {
                MMC_LOG_ERROR("client " << name_ << " batch put update for key " << updateRequest.keys_[i]
                                        << " failed:" << updateResponse.results_[i]);
            }
        }
    }
}

void MmcClientDefault::AsyncUpdateState(BatchUpdateRequest &updateRequest)
{
    auto future = threadPool_->Enqueue([&](BatchUpdateRequest updateRequestL) { SyncUpdateState(updateRequestL); },
                                       updateRequest);
    if (!future.valid()) {
        SyncUpdateState(updateRequest);
    }
}

Result MmcClientDefault::PrepareBlob(const MmcBufferArray &bufArr, const MmcMemBlobDesc &blob, MediaType &mediaType,
                                     BatchCopyDesc &copyDesc, bool blobIsSrc)
{
    if (bufArr.Buffers().empty()) {
        MMC_LOG_ERROR("buffer is empty");
        return MMC_INVALID_PARAM;
    }
    uint64_t shift = 0;
    for (size_t k = 0; k < bufArr.Buffers().size(); ++k) {
        auto buf = &bufArr.Buffers()[k];
        if (buf->type == MEDIA_NONE) {
            MMC_LOG_ERROR("unexcepted buf type:" << buf->type);
            return MMC_INVALID_PARAM;
        }
        if (mediaType == MEDIA_NONE) {
            mediaType = static_cast<MediaType>(buf->type);
        } else if (mediaType != buf->type) {
            MMC_LOG_ERROR("not all data type same as " << mediaType << ", unexcepted buf type:" << buf->type);
            return MMC_INVALID_PARAM;
        }
        if (blobIsSrc) {
            copyDesc.dsts.push_back(reinterpret_cast<void *>(buf->addr + buf->offset));
            copyDesc.srcs.push_back(reinterpret_cast<void *>(blob.gva_ + shift));
        } else {
            copyDesc.srcs.push_back(reinterpret_cast<void *>(buf->addr + buf->offset));
            copyDesc.dsts.push_back(reinterpret_cast<void *>(blob.gva_ + shift));
        }
        copyDesc.sizes.push_back(buf->len);
        shift += MmcBufSize(*buf);
    }
    return MMC_OK;
}

Result MmcClientDefault::PrepareMultiBlobs(const MmcBufferArray &bufArr, const std::vector<MmcMemBlobDesc> &blobs,
                                           MediaType &mediaType, BatchCopyDesc &copyDesc, bool blobIsSrc)
{
    for (uint8_t j = 0; j < blobs.size(); ++j) {
        auto ret = PrepareBlob(bufArr, blobs[j], mediaType, copyDesc, blobIsSrc);
        if (ret != MMC_OK) {
            return ret;
        }
    }
    return MMC_OK;
}

std::future<int32_t> MmcClientDefault::SubmitPutTask(BatchCopyDesc &copyDesc, MediaType mediaType, bool asyncExec)
{
    if (asyncExec) {
        auto future = writeThreadPool_->Enqueue(
            [&](BatchCopyDesc copyDescL, MediaType localMediaL) -> int32_t {
                return bmProxy_->BatchDataPut(copyDescL.srcs, copyDescL.dsts, copyDescL.sizes, localMediaL);
            },
            copyDesc, mediaType);
        if (future.valid()) {
            return future;
        }
    }

    // 提交失败 或 直接执行
    std::promise<int32_t> prom{};
    std::future<int32_t> future = prom.get_future();
    TP_TRACE_BEGIN(TP_MMC_LOCAL_BATCH_PUT_SYNC);
    auto ret = bmProxy_->BatchDataPut(copyDesc.srcs, copyDesc.dsts, copyDesc.sizes, mediaType);
    TP_TRACE_END(TP_MMC_LOCAL_BATCH_PUT_SYNC, ret);
    prom.set_value(ret);
    return future;
}

std::future<int32_t> MmcClientDefault::SubmitGetTask(BatchCopyDesc &copyDesc, MediaType mediaType, bool asyncExec)
{
    if (asyncExec) {
        auto future = readThreadPool_->Enqueue(
            [&](BatchCopyDesc copyDescL, MediaType localMediaL) -> int32_t {
                return bmProxy_->BatchDataGet(copyDescL.srcs, copyDescL.dsts, copyDescL.sizes, localMediaL);
            },
            copyDesc, mediaType);
        if (future.valid()) {
            return future;
        }
    }
    // 提交失败 或 直接执行
    std::promise<int32_t> prom{};
    std::future<int32_t> future = prom.get_future();
    TP_TRACE_BEGIN(TP_MMC_LOCAL_BATCH_GET_SYNC);
    auto ret = bmProxy_->BatchDataGet(copyDesc.srcs, copyDesc.dsts, copyDesc.sizes, mediaType);
    TP_TRACE_END(TP_MMC_LOCAL_BATCH_GET_SYNC, ret);
    prom.set_value(ret);
    return future;
}

Result MmcClientDefault::PutData2Blobs(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs,
                                       const BatchAllocResponse &allocResponse, std::vector<int> &batchResult)
{
    MediaType mediaType = MEDIA_NONE;
    std::vector<std::tuple<uint32_t, uint32_t, std::future<int32_t>>> futures;
    BatchCopyDesc copyDesc{};
    size_t startKeyIndex = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        const std::string &key = keys[i];
        const MmcBufferArray &bufArr = bufArrs[i];
        const auto &blobs = allocResponse.blobs_[i];
        const auto numBlobs = allocResponse.numBlobs_[i];
        if (allocResponse.results_[i] != MMC_OK) {
            // alloc has error, reserve alloc error code
            if (allocResponse.results_[i] != MMC_DUPLICATED_OBJECT) {
                MMC_LOG_ERROR("Alloc blob failed for key " << key << ", error code=" << allocResponse.results_[i]);
            }
            batchResult[i] = allocResponse.results_[i];
            continue;
        } else if (numBlobs == 0 || blobs.size() != numBlobs) {
            MMC_LOG_ERROR("Invalid number of blobs" << numBlobs << " , " << blobs.size() << " for key " << key);
            continue;
        }

        // 一个key对应的所有blob副本
        BatchCopyDesc keyCopyDesc{};
        batchResult[i] = PrepareMultiBlobs(bufArr, blobs, mediaType, keyCopyDesc, false);
        if (batchResult[i] != MMC_OK) {
            MMC_LOG_ERROR("Prepare multi blobs failed for key " << key);
            continue;
        }

        copyDesc.Append(keyCopyDesc);
        if (aggregateIO_ && (copyDesc.sizes.size() < aggregateNum_) && (i != (keys.size() - 1))) {
            continue;
        }

        auto future = SubmitPutTask(copyDesc, mediaType, !(startKeyIndex == 0 && i == (keys.size() - 1)));
        futures.push_back(std::make_tuple(startKeyIndex, i, std::move(future)));

        copyDesc.Clear();
        startKeyIndex = i + 1;
    }
    //  last key is invalid, task not submit
    if (!copyDesc.sizes.empty()) {
        auto future = SubmitPutTask(copyDesc, mediaType, !(startKeyIndex == 0));
        futures.push_back(std::make_tuple(startKeyIndex, (keys.size() - 1), std::move(future)));
    }

    TP_TRACE_BEGIN(TP_MMC_LOCAL_PUT_WAIT_FUTURE);
    WaitFeatures(futures, batchResult);
    TP_TRACE_END(TP_MMC_LOCAL_PUT_WAIT_FUTURE, 0);

    return MMC_OK;
}

Result MmcClientDefault::RegisterBuffer(uint64_t addr, uint64_t size)
{
    return bmProxy_->RegisterBuffer(addr, size);
}

Result MmcClientDefault::UnRegisterBuffer(uint64_t addr, uint64_t size)
{
    return bmProxy_->UnRegisterBuffer(addr);
}

} // namespace mmc
} // namespace ock