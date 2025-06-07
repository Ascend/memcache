/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_MMC_MSG_PACKER_H
#define MEMFABRIC_MMC_MSG_PACKER_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
class NetMsgPacker {
public:
    template <typename T>
    void Serialize(const T &val, typename std::enable_if<std::is_trivially_copyable<T>::value, int>::type = 0)
    {
        outStream_.write(reinterpret_cast<const char *>(&val), sizeof(T));
    }

    void Serialize(const std::string &val)
    {
        uint32_t size = val.size();
        outStream_.write(reinterpret_cast<const char *>(&size), sizeof(size));
        outStream_.write(val.data(), size);
    }

    template <typename K, typename V>
    void Serialize(const std::pair<K, V> &val)
    {
        Serialize(val.first);
        Serialize(val.second);
    }

    template <typename V>
    void Serialize(const std::vector<V> &container)
    {
        const std::size_t size = container.size();
        outStream_.write(reinterpret_cast<const char *>(&size), sizeof(size));
        for (const auto &item : container) {
            Serialize(item);
        }
    }

    template <typename K, typename V>
    void Serialize(const std::map<K, V> &container)
    {
        const std::size_t size = container.size();
        outStream_.write(reinterpret_cast<const char *>(&size), sizeof(size));
        for (const auto &item : container) {
            Serialize(item);
        }
    }

    std::string String() const
    {
        return outStream_.str();
    }

private:
    std::ostringstream outStream_;
};

class NetMsgUnpacker {
public:
    explicit NetMsgUnpacker(const std::string &value) : inStream_(value) {}

    template <typename T>
    void Deserialize(T &val, typename std::enable_if<std::is_trivially_copyable<T>::value, int>::type = 0)
    {
        inStream_.read(reinterpret_cast<char *>(&val), sizeof(T));
    }

    void Deserialize(std::string &val)
    {
        uint32_t size = 0;
        inStream_.read(reinterpret_cast<char *>(&size), sizeof(size));
        val.resize(size);
        inStream_.read(&val[0], size);
    }

    template <typename V>
    void Deserialize(std::vector<V> &container)
    {
        std::size_t size = 0;
        inStream_.read(reinterpret_cast<char *>(&size), sizeof(size));
        container.clear();
        container.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            V item;
            Deserialize(item);
            container.emplace_back(std::move(item));
        }
    }

    template <typename K, typename V>
    void Deserialize(std::map<K, V> &container)
    {
        std::size_t size = 0;
        inStream_.read(reinterpret_cast<char *>(&size), sizeof(size));
        container.clear();
        for (std::size_t i = 0; i < size; ++i) {
            K key;
            V value;
            Deserialize(key);
            Deserialize(value);
            container.emplace(std::move(key), std::move(value));
        }
    }

private:
    std::istringstream inStream_;
};
}
}

#endif  //MEMFABRIC_MMC_MSG_PACKER_H
