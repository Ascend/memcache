/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_client.h"
#include "mmc_common_includes.h"
#include "mmc_client_default.h"

using namespace ock::mmc;
static MmcClientDefault *mmmc_client_handler = nullptr;

MMC_API int32_t mmcc_init(mmc_client_config_t *config)
{
    auto *client_handler = new (std::nothrow)MmcClientDefault("mmc_client");
    MMC_ASSERT_RETURN(client_handler != nullptr, MMC_NEW_OBJECT_FAILED);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(client_handler->Start(*config), client_handler->Name() << " init failed!");
    mmmc_client_handler = client_handler;
    return MMC_OK;
}

MMC_API void mmcc_uninit()
{
    MMC_ASSERT_RET_VOID(mmmc_client_handler != nullptr);
    mmmc_client_handler->Stop();
    return;
}

MMC_API int32_t mmcc_put(const char *key, mmc_buffer *buf, uint32_t flags)
{
    MMC_ASSERT_RETURN(mmmc_client_handler != nullptr, MMC_CLIENT_NOT_INIT);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(mmmc_client_handler->Put(key, buf, flags),
                                    mmmc_client_handler->Name() << " put key " << key << " failed!");
    ;
    return MMC_OK;
}

MMC_API int32_t mmcc_get(const char *key, mmc_buffer *buf, uint32_t flags)
{
    MMC_ASSERT_RETURN(mmmc_client_handler != nullptr, MMC_CLIENT_NOT_INIT);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(mmmc_client_handler->Get(key, buf, flags),
                                    mmmc_client_handler->Name() << " gut key " << key << " failed!");
    return MMC_OK;
}

MMC_API mmc_location_t mmcc_get_location(const char *key, uint32_t flags)
{
    MMC_ASSERT_RETURN(mmmc_client_handler != nullptr, { 0 });
    return mmmc_client_handler->GetLocation(key, flags);
}

MMC_API int32_t mmcc_remove(const char *key, uint32_t flags)
{
    MMC_ASSERT_RETURN(mmmc_client_handler != nullptr, MMC_CLIENT_NOT_INIT);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(mmmc_client_handler->Remove(key, flags),
                                    mmmc_client_handler->Name() << " remove key " << key << " failed!");
    return MMC_OK;
}