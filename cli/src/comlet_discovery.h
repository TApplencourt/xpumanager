/* 
 *  Copyright (C) 2021-2023 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file comlet_discovery.h
 */

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "comlet_base.h"

namespace xpum::cli {

struct ComletDiscoveryOptions {
    std::string deviceId = "-1";
    int a = 0;
    bool listamcversions = false;
    bool showPfOnly = false;
    bool showVfOnly = false;
    std::vector<int> propIdList;
    std::string username = "";
    std::string password = "";
};

class ComletDiscovery : public ComletBase {
   public:
    ComletDiscovery();
    virtual ~ComletDiscovery() {}

    virtual void setupOptions() override;
    virtual std::unique_ptr<nlohmann::json> run() override;

    virtual void getTableResult(std::ostream &out) override;

    void checkBadDevices(nlohmann::json &deviceJsonList);

    inline bool isDeviceList() {
        return opts->deviceId == "-1";
    }

    inline bool isDumping() {
        return opts->propIdList.size() > 0;
    }

   private:
    std::unique_ptr<ComletDiscoveryOptions> opts;
};
} // end namespace xpum::cli
