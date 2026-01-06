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

#include <sstream>
#include <iomanip>

#include "prometheus/text_serializer.h"

#include "mmc_meta_metric_manager.h"

namespace prometheus {
namespace simpleapi {
auto registry_ptr = std::make_shared<Registry>();
Registry &registry = *registry_ptr;
} // namespace simpleapi
} // namespace prometheus

constexpr int FP_OUTPUT_PRECISION = 2;

namespace ock {
namespace mmc {

MmcMetaMetricManager::MmcMetaMetricManager()
    : allocCounter_("memcache_alloc_operations_total", "Total number of allocation operations"),
      removeCounter_("memcache_remove_operations_total", "Total number of remove operations"),
      getCounter_("memcache_get_operations_total", "Total number of get operations"),
      evictCounter_("memcache_evict_operations_total", "Total number of eviction operations"),
      keyCountGauge_("memcache_stored_keys", "Current number of stored keys")
{}

std::string MmcMetaMetricManager::GetSummary() const
{
    std::ostringstream oss;

    oss << "=== MemCache Metrics Summary ===" << std::endl;
    oss << std::fixed << std::setprecision(FP_OUTPUT_PRECISION);

    oss << "Allocation Operations: " << allocCounter_.value() << std::endl;
    oss << "Remove Operations: " << removeCounter_.value() << std::endl;
    oss << "Get Operations: " << getCounter_.value() << std::endl;

    return oss.str();
}

std::string MmcMetaMetricManager::GetPrometheusSummary()
{
    auto metrics = prometheus::simpleapi::registry.Collect();
    std::ostringstream ss;
    prometheus::TextSerializer::Serialize(ss, metrics);
    return ss.str();
}

} // namespace mmc
} // namespace ock