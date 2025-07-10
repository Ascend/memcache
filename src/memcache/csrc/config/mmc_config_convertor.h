/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#pragma once

#include <string>

#include "mmc_ref.h"

namespace ock {
namespace mmc {

class Converter : public MmcReferable {
public:
    ~Converter() override = default;

    virtual std::string Convert(const std::string& str)
    {
        return str;
    }
};

using ConverterPtr = MmcRef<Converter>;

}
}