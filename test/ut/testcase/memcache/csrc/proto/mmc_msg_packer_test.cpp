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

#include "mmc_msg_packer.h"

using namespace ock::mmc;

class MmcMsgPacker : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MmcMsgPacker, pack_and_unpack)
{
    int32_t a = 1;
    std::string b = "hello";
    std::vector<int32_t> c = {1, 2, 3, 4, 5};
    std::map<std::string, int32_t> d = {{"one", 1}, {"two", 2}};
    NetMsgPacker packer;
    packer.Serialize(a);
    packer.Serialize(b);
    packer.Serialize(c);
    packer.Serialize(d);

    auto serialized = packer.String();

    int32_t aa = 0;
    std::string bb = "";
    std::vector<int32_t> cc;
    std::map<std::string, int32_t> dd;
    NetMsgUnpacker unpacker(serialized);
    unpacker.Deserialize(aa);
    unpacker.Deserialize(bb);
    unpacker.Deserialize(cc);
    unpacker.Deserialize(dd);

    ASSERT_EQ(aa == a, true);
    ASSERT_EQ(bb == b, true);
    ASSERT_EQ(cc == c, true);
    ASSERT_EQ(dd == d, true);

    std::cout << aa << " " << bb << " " << std::endl;
}