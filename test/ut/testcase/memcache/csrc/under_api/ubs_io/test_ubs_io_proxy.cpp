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

#include <gtest/gtest.h>
#include "mmc_ubs_io_proxy.h"

using namespace ock::mmc;

class TestUbsIoProxy : public ::testing::Test {
protected:
    void SetUp() override
    {
        proxy_ = MmcUbsIoProxyFactory::GetInstance("test");
        ASSERT_NE(proxy_, nullptr);
        result_ = proxy_->InitUbsIo();
        ASSERT_EQ(result_, MMC_OK);
    }

    void TearDown() override
    {
        if (proxy_ != nullptr) {
            proxy_->DestroyUbsIo();
        }
    }

    MmcUbsIoProxyPtr proxy_;
    Result result_;
};

TEST_F(TestUbsIoProxy, InitAndDestroy)
{
    // 测试重复初始化
    result_ = proxy_->InitUbsIo();
    ASSERT_EQ(result_, MMC_OK);

    // 测试销毁
    proxy_->DestroyUbsIo();

    // 测试重新初始化
    result_ = proxy_->InitUbsIo();
    ASSERT_EQ(result_, MMC_OK);
}

TEST_F(TestUbsIoProxy, PutAndGet)
{
    // 测试基本的Put和Get操作
    std::string key = "test_key";
    std::string value = "test_value";

    // Put操作
    result_ = proxy_->Put(key, const_cast<char*>(value.c_str()), value.size());
    ASSERT_EQ(result_, MMC_OK);

    // Get操作
    char buffer[100] = {0};
    result_ = proxy_->Get(key, buffer, sizeof(buffer));
    ASSERT_EQ(result_, MMC_OK);
    ASSERT_STREQ(buffer, value.c_str());
}

TEST_F(TestUbsIoProxy, GetNonExistent)
{
    // 测试获取不存在的键
    std::string key = "non_existent_key";
    char buffer[100] = {0};
    result_ = proxy_->Get(key, buffer, sizeof(buffer));
    ASSERT_NE(result_, MMC_OK);
}

TEST_F(TestUbsIoProxy, Exist)
{
    // 测试Exist操作
    std::string key = "exist_test_key";
    std::string value = "exist_test_value";

    // 检查不存在
    result_ = proxy_->Exist(key);
    ASSERT_NE(result_, true);

    // Put操作
    result_ = proxy_->Put(key, const_cast<char*>(value.c_str()), value.size());
    ASSERT_EQ(result_, MMC_OK);

    // 检查存在
    result_ = proxy_->Exist(key);
    ASSERT_EQ(result_, true);
}

TEST_F(TestUbsIoProxy, Delete)
{
    // 测试Delete操作
    std::string key = "delete_test_key";
    std::string value = "delete_test_value";

    // Put操作
    result_ = proxy_->Put(key, const_cast<char*>(value.c_str()), value.size());
    ASSERT_EQ(result_, MMC_OK);

    // 检查存在
    result_ = proxy_->Exist(key);
    ASSERT_EQ(result_, true);

    // Delete操作
    result_ = proxy_->Delete(key);
    ASSERT_EQ(result_, MMC_OK);

    // 检查不存在
    result_ = proxy_->Exist(key);
    ASSERT_NE(result_, true);
}

TEST_F(TestUbsIoProxy, GetLength)
{
    // 测试GetLength操作
    std::string key = "length_test_key";
    std::string value = "length_test_value";
    size_t length = 0;

    // Put操作
    result_ = proxy_->Put(key, const_cast<char*>(value.c_str()), value.size());
    ASSERT_EQ(result_, MMC_OK);

    // GetLength操作
    result_ = proxy_->GetLength(key, length);
    ASSERT_EQ(result_, MMC_OK);
    ASSERT_EQ(length, value.size());
}

TEST_F(TestUbsIoProxy, BatchPutAndGet)
{
    // 测试批量Put和Get操作
    std::vector<std::string> keys = {"batch_key1", "batch_key2", "batch_key3"};
    std::vector<std::string> values = {"batch_value1", "batch_value2", "batch_value3"};
    std::vector<void*> bufs;
    std::vector<size_t> lengths;
    std::vector<int> results(keys.size());

    // 准备数据
    for (const auto& value : values) {
        bufs.push_back(const_cast<char*>(value.c_str()));
        lengths.push_back(value.size());
    }

    // 批量Put操作
    result_ = proxy_->BatchPut(keys, bufs, lengths, results);
    ASSERT_EQ(result_, MMC_OK);
    for (int res : results) {
        ASSERT_EQ(res, 0);
    }

    // 批量Get操作
    std::vector<void*> get_bufs(keys.size(), nullptr);
    std::vector<size_t> get_lengths(keys.size(), 0);
    std::vector<int> get_results(keys.size());

    result_ = proxy_->BatchGet(keys, get_bufs.data(), get_lengths, get_results);
    ASSERT_EQ(result_, MMC_OK);
    for (int i = 0; i < get_results.size(); ++i) {
        ASSERT_EQ(get_results[i], 0);
        ASSERT_STREQ(static_cast<char*>(get_bufs[i]), values[i].c_str());
    }

    // 测试BatchGetFree操作（释放DFC分配的内存）
    result_ = proxy_->BatchGetFree(get_bufs.data(), get_bufs.size());
    ASSERT_EQ(result_, MMC_OK);
}

TEST_F(TestUbsIoProxy, BatchExist)
{
    // 测试批量Exist操作
    std::vector<std::string> keys = {"exist_key1", "exist_key2", "exist_key3"};
    std::vector<std::string> values = {"value1", "value2", "value3"};

    // 只Put前两个键
    for (int i = 0; i < 2; ++i) {
        result_ = proxy_->Put(keys[i], const_cast<char*>(values[i].c_str()), values[i].size());
        ASSERT_EQ(result_, MMC_OK);
    }

    // 批量Exist操作
    bool exist_results[3] = {false};
    result_ = proxy_->BatchExist(keys, exist_results);
    ASSERT_EQ(result_, MMC_OK);
    ASSERT_TRUE(exist_results[0]);
    ASSERT_TRUE(exist_results[1]);
    ASSERT_FALSE(exist_results[2]);
}

TEST_F(TestUbsIoProxy, BatchDelete)
{
    // 测试批量Delete操作
    std::vector<std::string> keys = {"delete_key1", "delete_key2", "delete_key3"};
    std::vector<std::string> values = {"value1", "value2", "value3"};

    // Put所有键
    for (int i = 0; i < keys.size(); ++i) {
        result_ = proxy_->Put(keys[i], const_cast<char*>(values[i].c_str()), values[i].size());
        ASSERT_EQ(result_, MMC_OK);
    }

    // 批量Delete操作
    std::vector<int32_t> delete_results(keys.size());
    result_ = proxy_->BatchDelete(keys, delete_results);
    ASSERT_EQ(result_, MMC_OK);
    for (int32_t res : delete_results) {
        ASSERT_EQ(res, 0);
    }

    // 检查所有键都不存在
    for (const auto& key : keys) {
        result_ = proxy_->Exist(key);
        ASSERT_NE(result_, true);
    }
}

TEST_F(TestUbsIoProxy, BatchGetLength)
{
    // 测试批量GetLength操作
    std::vector<std::string> keys = {"length_key1", "length_key2"};
    std::vector<std::string> values = {"short", "longer_value"};

    // Put所有键
    for (int i = 0; i < keys.size(); ++i) {
        result_ = proxy_->Put(keys[i], const_cast<char*>(values[i].c_str()), values[i].size());
        ASSERT_EQ(result_, MMC_OK);
    }

    // 批量GetLength操作
    std::vector<size_t> lengths(keys.size());
    std::vector<int32_t> results(keys.size());
    result_ = proxy_->BatchGetLength(keys, lengths, results);
    ASSERT_EQ(result_, MMC_OK);
    for (int i = 0; i < results.size(); ++i) {
        ASSERT_EQ(results[i], 0);
        ASSERT_EQ(lengths[i], values[i].size());
    }
}
