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

#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "mmc_lock.h"
#include "mmc_types.h"

namespace ock {
namespace mmc {
using KvPair = struct KvPairs {
    std::string name;
    std::string value;
};

class KVParser {
public:
    KVParser();
    ~KVParser();

    KVParser(const KVParser &) = delete;
    KVParser &operator=(const KVParser &) = delete;
    KVParser(const KVParser &&) = delete;
    KVParser &operator=(const KVParser &&) = delete;

    Result FromFile(const std::string &filePath);

    Result GetItem(const std::string &key, std::string &outValue);
    Result SetItem(const std::string &key, const std::string &value);

    uint32_t Size();
    void GetI(const uint32_t index, std::string &outKey, std::string &outValue);

    void Dump();
    bool CheckSet(const std::vector<std::string> &keys);

private:
    Result ParseLine(std::string &strLine);

    std::map<std::string, uint32_t> mItemsIndex;
    std::vector<KvPair *> mItems;
    std::unordered_map<std::string, bool> mGotKeys;
    Lock mLock;
};
} // namespace mmc
} // namespace ock