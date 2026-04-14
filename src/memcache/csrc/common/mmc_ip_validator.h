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

#ifndef MMC_IP_VALIDATOR_H
#define MMC_IP_VALIDATOR_H

#include <arpa/inet.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace ock {
namespace mmc {

class UrlParser {
public:
    UrlParser() = default;
    bool Initialize(const std::string &url)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (initialized_) {
            return true;
        }

        if (url.empty()) {
            return false;
        }

        if (!ParseUrl(url) || !ResolveAddress()) {
            return false;
        }
        initialized_ = true;
        return true;
    }

    [[nodiscard]] std::string GetIp()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return ip_;
    }

    [[nodiscard]] uint16_t GetPort()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return port_;
    }

    [[nodiscard]] bool IsIpv6()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return is_ipv6_;
    }

    [[nodiscard]] int GetAddressFamily()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return address_family_;
    }

    [[nodiscard]] const struct sockaddr *GetSockAddr()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return reinterpret_cast<const struct sockaddr *>(&storage_);
    }

    [[nodiscard]] socklen_t GetAddrLen()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return addr_len_;
    }

    [[nodiscard]] std::string GetProtocol()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        return protocol_;
    }

    [[nodiscard]] bool IsInitialized()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return initialized_;
    }

    [[nodiscard]] std::pair<const sockaddr *, size_t> GetPeerAddress(const std::string &peerIp, const uint16_t port)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!initialized_) {
            return {};
        }
        std::fill_n(reinterpret_cast<uint8_t *>(&peerStorage_), sizeof(peerStorage_), 0);
        if (is_ipv6_) {
            auto *addr6 = reinterpret_cast<struct sockaddr_in6 *>(&peerStorage_);
            if (inet_pton(AF_INET6, peerIp.c_str(), &addr6->sin6_addr) != 1) {
                return {};
            }
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = htons(port);
            return {reinterpret_cast<const sockaddr *>(&peerStorage_), sizeof(sockaddr_in6)};
        }
        auto *addr4 = reinterpret_cast<struct sockaddr_in *>(&peerStorage_);
        if (inet_pton(AF_INET, peerIp.c_str(), &addr4->sin_addr) != 1) {
            return {};
        }
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        return {reinterpret_cast<const sockaddr *>(&peerStorage_), sizeof(sockaddr_in)};
    }

private:
    static std::vector<std::string> GetSupportedProtocols()
    {
        static const std::vector<std::string> protocols = {
            "tcp://",
            "http://",
            "https://",
        };
        return protocols;
    }

    bool ParseUrl(const std::string &url)
    {
        std::string host_port;

        protocol_ = "";
        for (const auto &protocol : GetSupportedProtocols()) {
            if (url.find(protocol) == 0) {
                host_port = url.substr(protocol.length());
                protocol_ = protocol;
                break;
            }
        }

        if (protocol_.empty()) {
            host_port = url;
        }

        size_t colon_pos = host_port.find_last_of(':');
        if (colon_pos == std::string::npos) {
            return false;
        }
        std::string host = host_port.substr(0, colon_pos);
        std::string port_str = host_port.substr(colon_pos + 1);
        // 解析端口
        try {
            int port = std::stoi(port_str);
            if (port < 1 || port > std::numeric_limits<uint16_t>::max()) {
                return false;
            }
            port_ = static_cast<uint16_t>(port);
        } catch (const std::exception &) {
            return false;
        }

        // 处理IPv6地址（包含在方括号中）
        if (host.front() == '[' && host.back() == ']') {
            constexpr size_t kLeftBracketLen = 1;   // 左括号 '[' 长度
            constexpr size_t kRightBracketLen = 1;  // 右括号 ']' 长度
            ip_ = host.substr(kLeftBracketLen, host.length() - kLeftBracketLen - kRightBracketLen);
            is_ipv6_ = true;
        } else {
            ip_ = host;
            // 检查是否是IPv6地址（包含冒号）
            is_ipv6_ = (host.find(':') != std::string::npos);
        }

        if (ip_.empty()) {
            return false;
        }
        return true;
    }

    // 解析地址信息
    [[nodiscard]] bool ResolveAddress()
    {
        std::fill_n(reinterpret_cast<uint8_t *>(&storage_), sizeof(storage_), 0);
        if (is_ipv6_) {
            auto *addr6 = reinterpret_cast<struct sockaddr_in6 *>(&storage_);
            addr_len_ = sizeof(struct sockaddr_in6);
            address_family_ = AF_INET6;
            if (inet_pton(AF_INET6, ip_.c_str(), &addr6->sin6_addr) != 1) {
                return false;
            }
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = htons(port_);
            return true;
        }
        auto *addr4 = reinterpret_cast<struct sockaddr_in *>(&storage_);
        addr_len_ = sizeof(struct sockaddr_in);
        address_family_ = AF_INET;
        if (inet_pton(AF_INET, ip_.c_str(), &addr4->sin_addr) != 1) {
            return false;
        }
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port_);
        return true;
    }

    std::mutex mutex_{};
    std::string ip_;
    std::string protocol_;
    uint16_t port_ = 0;
    int address_family_ = 0;
    socklen_t addr_len_ = 0;
    sockaddr_storage storage_{};
    sockaddr_storage peerStorage_{};
    bool initialized_ = false;
    bool is_ipv6_ = false;
};

class IpAddressParserMgr {
public:
    IpAddressParserMgr(const IpAddressParserMgr &) = delete;
    IpAddressParserMgr &operator=(const IpAddressParserMgr &) = delete;

    static IpAddressParserMgr &getInstance()
    {
        static IpAddressParserMgr instance;
        return instance;
    }

    std::shared_ptr<UrlParser> CreateParser(const std::string &url)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto find = url2Parsers_.find(url);
        if (find != url2Parsers_.end()) {
            return find->second;
        }
        auto *o = new (std::nothrow) UrlParser;
        if (o == nullptr) {
            return nullptr;
        }
        const std::shared_ptr<UrlParser> obj(o);
        if (!obj->Initialize(url)) {
            return nullptr;
        }
        url2Parsers_[url] = obj;
        port2Parsers_[obj->GetPort()] = obj;
        return obj;
    }

    std::shared_ptr<UrlParser> GetParser(const uint32_t serverPort)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto find = port2Parsers_.find(serverPort);
        if (find == port2Parsers_.end()) {
            return nullptr;
        }
        return find->second;
    }

private:
    std::mutex mutex_{};
    std::unordered_map<std::string, std::shared_ptr<UrlParser>> url2Parsers_{};
    std::unordered_map<uint32_t, std::shared_ptr<UrlParser>> port2Parsers_{};
    IpAddressParserMgr() = default;
};

} // namespace mmc
} // namespace ock

#endif