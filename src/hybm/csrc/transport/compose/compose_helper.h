/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MF_HYBRID_COMPOSE_HELPER_H
#define MF_HYBRID_COMPOSE_HELPER_H

#include <sstream>
#include <vector>
#include <string>

namespace ock {
namespace mf {
namespace transport {
class ComposeHelper {
public:
    static inline std::vector<std::string> Split(const std::string& str, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }

        return result;
    }

    static inline bool StartsWith(const std::string& str, const std::string &prefix)
    {
        return str.length() >= prefix.length() &&
               str.compare(0, prefix.length(), prefix) == 0;
    }
};
}
}
}

#endif // MF_HYBRID_COMPOSE_HELPER_H
