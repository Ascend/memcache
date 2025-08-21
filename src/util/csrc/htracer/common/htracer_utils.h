/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HTRACE_UTILS_H
#define HTRACE_UTILS_H

#include <vector>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

namespace ock {
namespace mf {
constexpr int nameWidth = 50;
constexpr int digitWidth = 15;
constexpr int unitStep = 1000;
constexpr double iopsDiff = 90.000;
constexpr int numberPrecision = 3;
constexpr int widthTwo = 2;
constexpr mode_t DEFAULT_DIR_MODE = 0750;

class HTracerUtils {
public:
    static std::string CurrentTime()
    {
        time_t rawTime;
        time(&rawTime);
        auto tmInfo = localtime(&rawTime);
        if (tmInfo == nullptr) {
            return "";
        }
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(widthTwo) << std::right << tmInfo->tm_hour << ":" << std::setfill('0') <<
            std::setw(widthTwo) << std::right << tmInfo->tm_min << ":" << std::setfill('0') << std::setw(widthTwo) <<
            std::right << tmInfo->tm_sec;
        return ss.str();
    }

    static void Split(const std::string &src, const std::string &sep, std::vector<std::string> &out)
    {
        std::string::size_type pos1 = 0;
        std::string::size_type pos2 = src.find(sep);

        std::string tmpStr;
        while (std::string::npos != pos2) {
            tmpStr = src.substr(pos1, pos2 - pos1);
            out.emplace_back(tmpStr);
            pos1 = pos2 + sep.size();
            pos2 = src.find(sep, pos1);
        }

        if (pos1 != src.length()) {
            tmpStr = src.substr(pos1);
            out.emplace_back(tmpStr);
        }
    }

    static std::string FormatString(std::string &name, uint64_t begin, uint64_t goodEnd, uint64_t badEnd, uint64_t min,
        uint64_t max, uint64_t total)
    {
        std::string str;
        std::ostringstream os(str);
        os.flags(std::ios::fixed);
        os.precision(numberPrecision);
        os << std::left << std::setw(nameWidth) << name << "\t" << std::left << std::setw(digitWidth) << begin <<
            "\t" << std::left << std::setw(digitWidth) << goodEnd << "\t" << std::left << std::setw(digitWidth) <<
            badEnd << "\t" << std::left << std::setw(digitWidth) <<
            ((begin > goodEnd + badEnd) ? (begin - goodEnd - badEnd) : 0) << "\t" << std::left <<
            std::setw(digitWidth) << (static_cast<double>(begin) / iopsDiff) << "\t" << std::left <<
            std::setw(digitWidth) << (min == UINT64_MAX ? 0 : ((double)min / unitStep)) << "\t" << std::left <<
            std::setw(digitWidth) << static_cast<double>(max) / unitStep << "\t" << std::left <<
            std::setw(digitWidth) <<
            (goodEnd == 0 ? 0 : static_cast<double>(total) / static_cast<double>(goodEnd) / unitStep) << "\t" <<
            std::left << std::setw(digitWidth) << static_cast<double>(total) / unitStep;
        return os.str();
    }

    static int CreateDirectory(const std::string &name)
    {
        std::vector<std::string> paths;
        Split(name, "/", paths);
        int32_t ret = 0;
        std::string pathTmp;
        for (auto &item : paths) {
            if (item.empty()) {
                continue;
            }

            pathTmp += "/" + item;
            if (access(pathTmp.c_str(), F_OK) != 0) {
                ret = mkdir(pathTmp.c_str(), DEFAULT_DIR_MODE);
                if (ret != 0 && errno != EEXIST) {
                    break;
                }
            }
        }
        return ret;
    }

    static std::string HeaderString()
    {
        std::stringstream ss;
        ss << "\t" << std::left << std::setw(nameWidth) << "NAME"
           << "\t" << std::left << std::setw(digitWidth) << "BEGIN"
           << "\t" << std::left << std::setw(digitWidth) << "GOOD_END"
           << "\t" << std::left << std::setw(digitWidth) << "BAD_END"
           << "\t" << std::left << std::setw(digitWidth) << "ON_FLY"
           << "\t" << std::left << std::setw(digitWidth) << "IOPS"
           << "\t" << std::left << std::setw(digitWidth) << "MIN(us)"
           << "\t" << std::left << std::setw(digitWidth) << "MAX(us)"
           << "\t" << std::left << std::setw(digitWidth) << "AVG(us)"
           << "\t" << std::left << std::setw(digitWidth) << "TOTAL(us)";
        return ss.str();
    }
};
}
}
#endif // HTRACE_UTILS_H
