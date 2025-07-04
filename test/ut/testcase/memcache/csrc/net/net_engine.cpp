/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <iostream>
#include "gtest/gtest.h"
#include "mmc_ref.h"
#include "mmc_net_engine.h"
#include "mmc_net_engine_acc.h"
#include "mmc_msg_client_meta.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

class MMcNetEngine : public testing::Test {
public:
    MMcNetEngine();

    void SetUp() override;

    void TearDown() override;

protected:
};
MMcNetEngine::MMcNetEngine()
{
}

void MMcNetEngine::SetUp()
{
    cout << "this is NetEngine TEST_F setup:";
}

void MMcNetEngine::TearDown()
{
    cout << "this is NetEngine TEST_F teardown";
}
int32_t HandleTestRequest(NetContextPtr &ctx)
{
    std::cout << "HandleTestRequest ut test" << std::endl;
    PingMsg recv;
    recv.destRankId = 0;
    recv.msgId = MsgId::PING_MSG;
    recv.num = 888;

    NetMsgPacker packer;
    recv.Serialize(packer);
    std::string serializedData = packer.String();
    uint32_t retSize = serializedData.length();
    return ctx->Reply(0, const_cast<char* >(serializedData.c_str()), retSize);
}

int32_t HandleTestLinkServer(const NetLinkPtr &link)
{
    std::cout << "HandleTestLinkServer" << std::endl;
    return MMC_OK;
}

TEST_F(MMcNetEngine, Init)
{
    NetEngineOptions options;
    options.name = "test_engine";
    options.ip = "127.0.0.1";
    options.port = 5678;
    options.threadCount = 2;
    options.rankId = 0;
    options.startListener = true;

    NetEngineOptions optionsClient;
    optionsClient.name = "test_engine_client";
    optionsClient.ip = "127.0.0.1";
    optionsClient.port = 5679;
    optionsClient.threadCount = 2;
    optionsClient.rankId = 0;
    optionsClient.startListener = false;
    NetEnginePtr server = NetEngine::Create();

    server->RegRequestReceivedHandler(0, std::bind(&HandleTestRequest, std::placeholders::_1));
    server->RegNewLinkHandler(std::bind(&HandleTestLinkServer, std::placeholders::_1));
    ASSERT_TRUE(server->Start(options) == MMC_OK);

    NetEnginePtr client = NetEngine::Create();
    client->RegRequestReceivedHandler(0, nullptr);
    ASSERT_TRUE(client->Start(optionsClient) == MMC_OK);

    NetLinkPtr linkPtr;
    ASSERT_TRUE(client->ConnectToPeer(options.rankId, options.ip, options.port, linkPtr, false) == MMC_OK);
    PingMsg send;
    send.destRankId = 0;
    send.msgId = MsgId::PING_MSG;
    send.num = 888;
    PingMsg recv;
    recv.destRankId = 0;
    recv.msgId = MsgId::PING_MSG;
    recv.num = 666;
    int16_t rspResult = 0;
    ASSERT_TRUE(client->Call(options.rankId, send.msgId, send, recv, rspResult, 30) == MMC_OK);
    ASSERT_TRUE(recv.num == 888);
    server->Stop();
    client->Stop();
}
