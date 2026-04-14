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

#include <arpa/inet.h>

#include "gtest/gtest.h"

#include "common/mmc_ip_validator.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

// 测试用常量定义
namespace {
    const char VALID_IPV4_URL[] = "tcp://192.168.1.1:8080";
    const char VALID_IPV6_URL[] = "tcp://[::1]:9090";
    const char VALID_HTTP_URL[] = "http://10.0.0.1:7070";
    const char VALID_HTTPS_URL[] = "https://172.16.0.1:6060";
    const char URL_WITHOUT_PROTOCOL[] = "192.168.1.100:5050";
    const char INVALID_URL_EMPTY[] = "";
    const char INVALID_URL_NO_PORT[] = "tcp://192.168.1.1";
    const char INVALID_URL_INVALID_PORT[] = "tcp://192.168.1.1:99999";
    const char INVALID_URL_ZERO_PORT[] = "tcp://192.168.1.1:0";
    const char INVALID_URL_NEGATIVE_PORT[] = "tcp://192.168.1.1:-1";
    const char INVALID_URL_INVALID_IPV4[] = "tcp://999.999.999.999:8080";
    const char INVALID_URL_INVALID_IPV6[] = "tcp://[invalid]:8080";
    const uint16_t VALID_PORT_8080 = 8080;
    const uint16_t VALID_PORT_9090 = 9090;
    const uint16_t VALID_PORT_7070 = 7070;
    const uint16_t VALID_PORT_6060 = 6060;
    const uint16_t VALID_PORT_5050 = 5050;
    const uint16_t MIN_VALID_PORT = 1;
    const uint16_t MAX_VALID_PORT = 65535;
    const int AF_INET_VALUE = AF_INET;
    const int AF_INET6_VALUE = AF_INET6;
}

class TestUrlParser : public testing::Test {
public:
    TestUrlParser();
    void SetUp() override;
    void TearDown() override;

protected:
    UrlParser parser_;
};

TestUrlParser::TestUrlParser() {}

void TestUrlParser::SetUp() {}

void TestUrlParser::TearDown() {}

// 测试 IPv4 URL 解析
TEST_F(TestUrlParser, ParseValidIpv4Url_Success)
{
    bool ret = parser_.Initialize(VALID_IPV4_URL);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "192.168.1.1");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_8080);
    EXPECT_FALSE(parser_.IsIpv6());
    EXPECT_EQ(parser_.GetAddressFamily(), AF_INET_VALUE);
    EXPECT_EQ(parser_.GetProtocol(), "tcp://");
}

// 测试 IPv6 URL 解析（带方括号）
TEST_F(TestUrlParser, ParseValidIpv6UrlWithBrackets_Success)
{
    bool ret = parser_.Initialize(VALID_IPV6_URL);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "::1");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_9090);
    EXPECT_TRUE(parser_.IsIpv6());
    EXPECT_EQ(parser_.GetAddressFamily(), AF_INET6_VALUE);
    EXPECT_EQ(parser_.GetProtocol(), "tcp://");
}

// 测试 HTTP 协议 URL 解析
TEST_F(TestUrlParser, ParseHttpUrl_Success)
{
    bool ret = parser_.Initialize(VALID_HTTP_URL);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "10.0.0.1");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_7070);
    EXPECT_FALSE(parser_.IsIpv6());
    EXPECT_EQ(parser_.GetProtocol(), "http://");
}

// 测试 HTTPS 协议 URL 解析
TEST_F(TestUrlParser, ParseHttpsUrl_Success)
{
    bool ret = parser_.Initialize(VALID_HTTPS_URL);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "172.16.0.1");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_6060);
    EXPECT_FALSE(parser_.IsIpv6());
    EXPECT_EQ(parser_.GetProtocol(), "https://");
}

// 测试不带协议的 URL 解析
TEST_F(TestUrlParser, ParseUrlWithoutProtocol_Success)
{
    bool ret = parser_.Initialize(URL_WITHOUT_PROTOCOL);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "192.168.1.100");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_5050);
    EXPECT_FALSE(parser_.IsIpv6());
    EXPECT_EQ(parser_.GetProtocol(), "");
}

// 测试空 URL
TEST_F(TestUrlParser, ParseEmptyUrl_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_EMPTY);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
    
    EXPECT_EQ(parser_.GetIp(), "");
    EXPECT_EQ(parser_.GetPort(), 0);
}

// 测试缺少端口的 URL
TEST_F(TestUrlParser, ParseUrlWithoutPort_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_NO_PORT);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试端口号过大的 URL
TEST_F(TestUrlParser, ParseUrlWithPortTooLarge_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_INVALID_PORT);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试端口号为 0 的 URL
TEST_F(TestUrlParser, ParseUrlWithZeroPort_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_ZERO_PORT);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试负数端口号的 URL
TEST_F(TestUrlParser, ParseUrlWithNegativePort_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_NEGATIVE_PORT);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试无效 IPv4 地址
TEST_F(TestUrlParser, ParseInvalidIpv4Address_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_INVALID_IPV4);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试无效 IPv6 地址
TEST_F(TestUrlParser, ParseInvalidIpv6Address_Fail)
{
    bool ret = parser_.Initialize(INVALID_URL_INVALID_IPV6);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(parser_.IsInitialized());
}

// 测试重复初始化（应该返回 true）
TEST_F(TestUrlParser, InitializeMultipleTimes_Success)
{
    bool ret1 = parser_.Initialize(VALID_IPV4_URL);
    ASSERT_TRUE(ret1);
    
    bool ret2 = parser_.Initialize(VALID_IPV6_URL);
    ASSERT_TRUE(ret2);
    
    // 应该保留第一次初始化的值
    EXPECT_EQ(parser_.GetIp(), "192.168.1.1");
    EXPECT_EQ(parser_.GetPort(), VALID_PORT_8080);
}

// 测试获取 sockaddr 结构
TEST_F(TestUrlParser, GetSockAddr_ValidAfterInit)
{
    bool ret = parser_.Initialize(VALID_IPV4_URL);
    ASSERT_TRUE(ret);
    
    const struct sockaddr *addr = parser_.GetSockAddr();
    EXPECT_NE(addr, nullptr);
    
    socklen_t addrLen = parser_.GetAddrLen();
    EXPECT_EQ(addrLen, sizeof(struct sockaddr_in));
}

// 测试未初始化时获取 sockaddr
TEST_F(TestUrlParser, GetSockAddr_BeforeInit_ReturnNull)
{
    const struct sockaddr *addr = parser_.GetSockAddr();
    EXPECT_EQ(addr, nullptr);
    
    socklen_t addrLen = parser_.GetAddrLen();
    EXPECT_EQ(addrLen, 0);
}

// 测试获取对等方地址（IPv4）
TEST_F(TestUrlParser, GetPeerAddressIpv4_Success)
{
    bool ret = parser_.Initialize(VALID_IPV4_URL);
    ASSERT_TRUE(ret);
    
    const string peerIp = "192.168.1.2";
    const uint16_t peerPort = 8081;
    
    auto [addr, size] = parser_.GetPeerAddress(peerIp, peerPort);
    EXPECT_NE(addr, nullptr);
    EXPECT_EQ(size, sizeof(struct sockaddr_in));
    
    // 验证地址内容
    const auto *addrIn = reinterpret_cast<const struct sockaddr_in *>(addr);
    EXPECT_EQ(addrIn->sin_family, AF_INET_VALUE);
    EXPECT_EQ(ntohs(addrIn->sin_port), peerPort);
}

// 测试获取对等方地址（IPv6）
TEST_F(TestUrlParser, GetPeerAddressIpv6_Success)
{
    bool ret = parser_.Initialize(VALID_IPV6_URL);
    ASSERT_TRUE(ret);
    
    const string peerIp = "::2";
    const uint16_t peerPort = 9091;
    
    auto [addr, size] = parser_.GetPeerAddress(peerIp, peerPort);
    EXPECT_NE(addr, nullptr);
    EXPECT_EQ(size, sizeof(struct sockaddr_in6));
    
    // 验证地址内容
    const auto *addrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(addr);
    EXPECT_EQ(addrIn6->sin6_family, AF_INET6_VALUE);
    EXPECT_EQ(ntohs(addrIn6->sin6_port), peerPort);
}

// 测试获取对等方地址时传入无效 IP
TEST_F(TestUrlParser, GetPeerAddressWithInvalidIp_Fail)
{
    bool ret = parser_.Initialize(VALID_IPV4_URL);
    ASSERT_TRUE(ret);
    
    const string invalidIp = "999.999.999.999";
    const uint16_t peerPort = 8081;
    
    auto [addr, size] = parser_.GetPeerAddress(invalidIp, peerPort);
    EXPECT_EQ(addr, nullptr);
    EXPECT_EQ(size, 0U);
}

// 测试未初始化时获取对等方地址
TEST_F(TestUrlParser, GetPeerAddressBeforeInit_Fail)
{
    const string peerIp = "192.168.1.2";
    const uint16_t peerPort = 8081;
    
    auto [addr, size] = parser_.GetPeerAddress(peerIp, peerPort);
    EXPECT_EQ(addr, nullptr);
    EXPECT_EQ(size, 0U);
}

// 测试边界端口号（最小值）
TEST_F(TestUrlParser, ParseUrlWithMinPort_Success)
{
    const string url = "tcp://192.168.1.1:1";
    bool ret = parser_.Initialize(url);
    ASSERT_TRUE(ret);
    EXPECT_EQ(parser_.GetPort(), MIN_VALID_PORT);
}

// 测试边界端口号（最大值）
TEST_F(TestUrlParser, ParseUrlWithMaxPort_Success)
{
    const string url = "tcp://192.168.1.1:65535";
    bool ret = parser_.Initialize(url);
    ASSERT_TRUE(ret);
    EXPECT_EQ(parser_.GetPort(), MAX_VALID_PORT);
}

class TestIpAddressParserMgr : public testing::Test {
public:
    TestIpAddressParserMgr();
    void SetUp() override;
    void TearDown() override;

protected:
    IpAddressParserMgr &mgr_ = IpAddressParserMgr::getInstance();
};

TestIpAddressParserMgr::TestIpAddressParserMgr() {}

void TestIpAddressParserMgr::SetUp() {}

void TestIpAddressParserMgr::TearDown() {}

// 测试创建解析器
TEST_F(TestIpAddressParserMgr, CreateParser_ValidUrl_Success)
{
    auto parser = mgr_.CreateParser(VALID_IPV4_URL);
    ASSERT_NE(parser, nullptr);
    EXPECT_TRUE(parser->IsInitialized());
    EXPECT_EQ(parser->GetIp(), "192.168.1.1");
    EXPECT_EQ(parser->GetPort(), VALID_PORT_8080);
}

// 测试创建解析器失败（无效 URL）
TEST_F(TestIpAddressParserMgr, CreateParser_InvalidUrl_ReturnNull)
{
    auto parser = mgr_.CreateParser(INVALID_URL_EMPTY);
    EXPECT_EQ(parser, nullptr);
}

// 测试缓存机制（相同 URL 返回相同解析器）
TEST_F(TestIpAddressParserMgr, CreateParser_SameUrl_ReturnSameParser)
{
    auto parser1 = mgr_.CreateParser(VALID_IPV4_URL);
    ASSERT_NE(parser1, nullptr);
    
    auto parser2 = mgr_.CreateParser(VALID_IPV4_URL);
    ASSERT_NE(parser2, nullptr);
    
    EXPECT_EQ(parser1, parser2);
}

// 测试通过端口获取解析器
TEST_F(TestIpAddressParserMgr, GetParser_ByPort_Success)
{
    auto parser = mgr_.CreateParser(VALID_IPV4_URL);
    ASSERT_NE(parser, nullptr);
    
    auto retrievedParser = mgr_.GetParser(VALID_PORT_8080);
    ASSERT_NE(retrievedParser, nullptr);
    EXPECT_EQ(retrievedParser, parser);
}

// 测试通过不存在的端口获取解析器
TEST_F(TestIpAddressParserMgr, GetParser_NonExistentPort_ReturnNull)
{
    const uint32_t nonExistentPort = 12345;
    auto parser = mgr_.GetParser(nonExistentPort);
    EXPECT_EQ(parser, nullptr);
}

// 测试创建多个不同 URL 的解析器
TEST_F(TestIpAddressParserMgr, CreateParser_MultipleUrls_Success)
{
    auto parser1 = mgr_.CreateParser(VALID_IPV4_URL);
    ASSERT_NE(parser1, nullptr);
    
    auto parser2 = mgr_.CreateParser(VALID_IPV6_URL);
    ASSERT_NE(parser2, nullptr);
    
    auto parser3 = mgr_.CreateParser(VALID_HTTP_URL);
    ASSERT_NE(parser3, nullptr);
    
    // 验证不同的解析器
    EXPECT_NE(parser1, parser2);
    EXPECT_NE(parser1, parser3);
    EXPECT_NE(parser2, parser3);
    
    // 验证可以通过端口获取
    EXPECT_NE(mgr_.GetParser(VALID_PORT_8080), nullptr);
    EXPECT_NE(mgr_.GetParser(VALID_PORT_9090), nullptr);
    EXPECT_NE(mgr_.GetParser(VALID_PORT_7070), nullptr);
}

// 测试单例模式
TEST_F(TestIpAddressParserMgr, GetInstance_ReturnSameInstance)
{
    auto &instance1 = IpAddressParserMgr::getInstance();
    auto &instance2 = IpAddressParserMgr::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}
