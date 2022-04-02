/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file comlet_health.cpp
 */

#include "comlet_health.h"

#include <nlohmann/json.hpp>

#include "cli_table.h"
#include "core_stub.h"

namespace xpum::cli {

static CharTableConfig ComletConfigHealthDeviceId(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "",
        "cells": [
            { "rowTitle": "Device ID" },
            "device_id"
        ]
    }]
})"_json);

static CharTableConfig ComletConfigHealthCoreTemp(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "core_temperature",
        "cells": [
            { "rowTitle": "1. GPU Core Temperature" }, [
            { "label": "Status", "value": "status" },
            { "label": "Description", "value": "description" },
            { "label": "Throttle Threshold", "suffix": " Celsius Degree", "value": "throttle_threshold", "fixer": "negint_novalue" },
            { "label": "Shutdown Threshold", "suffix": " Celsius Degree", "value": "shutdown_threshold", "fixer": "negint_novalue" },
            { "label": "Custom Threshold", "suffix": " Celsius Degree", "value": "custom_threshold", "fixer": "negint_novalue" }
        ]]
    }]
})"_json);

static CharTableConfig ComletConfigHealthMemTemp(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "memory_temperature",
        "cells": [
            { "rowTitle": "2. GPU Memory Temperature" }, [
            { "label": "Status", "value": "status" },
            { "label": "Description", "value": "description" },
            { "label": "Throttle Threshold", "suffix": " Celsius Degree", "value": "throttle_threshold", "fixer": "negint_novalue" },
            { "label": "Shutdown Threshold", "suffix": " Celsius Degree", "value": "shutdown_threshold", "fixer": "negint_novalue" },
            { "label": "Custom Threshold", "suffix": " Celsius Degree", "value": "custom_threshold", "fixer": "negint_novalue" }
        ]]
    }]
})"_json);

static CharTableConfig ComletConfigHealthPower(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "power",
        "cells": [
            { "rowTitle": "3. GPU Power" }, [
            { "label": "Status", "value": "status" },
            { "label": "Description", "value": "description" },
            { "label": "Throttle Threshold", "suffix": " watts", "value": "throttle_threshold", "fixer": "negint_novalue" },
            { "label": "Custom Threshold", "suffix": " watts", "value": "custom_threshold", "fixer": "negint_novalue" }
        ]]
    }]
})"_json);

static CharTableConfig ComletConfigHealthMemory(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "memory",
        "cells": [
            { "rowTitle": "4. GPU Memory" }, [
            { "label": "Status", "value": "status" },
            { "label": "Description", "value": "description" }
        ]]
    }]
})"_json);

static CharTableConfig ComletConfigHealthFabricPort(R"({
    "showTitleRow": false,
    "columns": [{
        "title": "none",
        "size": 26
    }, {
        "title": "none"
    }],
    "rows": [{
        "instance": "xe_link_port",
        "cells": [
            { "rowTitle": "5. Xe Link Port" }, [
            { "label": "Status", "value": "status" },
            { "label": "Description", "value": "description" }
        ]]
    }]
})"_json);

void ComletHealth::setupOptions() {
    this->opts = std::unique_ptr<ComletHealthOptions>(new ComletHealthOptions());
    addFlag("-l,--list", this->opts->listAll, "Display health info for all devices");
    addOption("-d,--device", this->opts->deviceId, "The device ID");
    addOption("-g,--group", this->opts->groupId, "The group ID");
    addOption("-c,--component", this->opts->componentType,
              "Component types\n\
      1. GPU Core Temperature\n\
      2. GPU Memory Temperature\n\
      3. GPU Power\n\
      4. GPU Memory\n\
      5. Xe Link Port");
    addOption("--threshold", this->opts->threshold, "Set custom threshold for device component");
}

std::unique_ptr<nlohmann::json> ComletHealth::run() {
    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());
    if (this->opts->listAll) {
        json = this->coreStub->getAllHealth();
        return json;
    }

    if (this->opts->deviceId != INT_MIN && this->opts->deviceId < 0) {
        (*json)["error"] = "device not found";
        return json;
    }

    if (this->opts->groupId == 0) {
        (*json)["error"] = "group not found";
        return json;
    }

    if (this->opts->componentType != INT_MIN && (this->opts->componentType < 1 || this->opts->componentType > 5)) {
        (*json)["error"] = "invalid component";
        return json;
    }

    if ((this->opts->threshold != INT_MIN && this->opts->threshold < -1) || (this->opts->threshold == 0)) {
        (*json)["error"] = "invalid threshold";
        return json;
    }

    if (this->opts->deviceId >= 0) {
        if (this->opts->threshold >= -1) {
            if (this->opts->componentType == 1) {
                json = this->coreStub->setHealthConfig(this->opts->deviceId, HEALTH_CORE_THERMAL_LIMIT, this->opts->threshold);
            } else if (this->opts->componentType == 2) {
                json = this->coreStub->setHealthConfig(this->opts->deviceId, HEALTH_MEMORY_THERMAL_LIMIT, this->opts->threshold);
            } else if (this->opts->componentType == 3) {
                json = this->coreStub->setHealthConfig(this->opts->deviceId, HEALTH_POWER_LIMIT, this->opts->threshold);
            } else {
                (*json)["error"] = "threshold setting unsupported";
            }
            if ((*json).contains("error")) {
                return json;
            }
            json = this->coreStub->getHealth(this->opts->deviceId, this->opts->componentType);
            return json;
        } else {
            if (this->opts->componentType >= 1 && this->opts->componentType <= 5)
                json = this->coreStub->getHealth(this->opts->deviceId, this->opts->componentType);
            else
                json = this->coreStub->getHealth(this->opts->deviceId, -1);
            return json;
        }
    }

    if (this->opts->groupId > 0 && this->opts->groupId != UINT_MAX) {
        if (this->opts->threshold >= -1) {
            if (this->opts->componentType == 1) {
                json = this->coreStub->setHealthConfigByGroup(this->opts->groupId, HEALTH_CORE_THERMAL_LIMIT, this->opts->threshold);
            } else if (this->opts->componentType == 2) {
                json = this->coreStub->setHealthConfigByGroup(this->opts->groupId, HEALTH_MEMORY_THERMAL_LIMIT, this->opts->threshold);
            } else if (this->opts->componentType == 3) {
                json = this->coreStub->setHealthConfigByGroup(this->opts->groupId, HEALTH_POWER_LIMIT, this->opts->threshold);
            } else {
                (*json)["error"] = "threshold setting unsupported";
            }
            if ((*json).contains("error")) {
                return json;
            }
            json = this->coreStub->getHealthByGroup(this->opts->groupId, this->opts->componentType);
            return json;
        } else {
            if (this->opts->componentType >= 1 && this->opts->componentType <= 5)
                json = this->coreStub->getHealthByGroup(this->opts->groupId, this->opts->componentType);
            else
                json = this->coreStub->getHealthByGroup(this->opts->groupId, -1);
            return json;
        }
    }
    (*json)["error"] = "Wrong argument or unknown operation, run with --help for more information.";
    return json;
}

static void showHealth(std::ostream &out, std::shared_ptr<nlohmann::json> json, CharTableConfig &healthConfig, const bool cont = true) {
    CharTable table(healthConfig, *json, cont);
    table.show(out);
}

static void showHealthAllComps(std::ostream &out, std::shared_ptr<nlohmann::json> json, const bool cont = false) {
    showHealth(out, json, ComletConfigHealthDeviceId, cont);
    showHealth(out, json, ComletConfigHealthCoreTemp);
    showHealth(out, json, ComletConfigHealthMemTemp);
    showHealth(out, json, ComletConfigHealthPower);
    showHealth(out, json, ComletConfigHealthMemory);
    showHealth(out, json, ComletConfigHealthFabricPort);
}

static void showHealthComp(std::ostream &out, std::shared_ptr<nlohmann::json> json, CharTableConfig &healthConfig, const bool cont = false) {
    showHealth(out, json, ComletConfigHealthDeviceId, cont);
    showHealth(out, json, healthConfig);
}

static void showHealthMultiDevicesAllComps(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
    auto devices = (*json)["device_list"].get<std::vector<nlohmann::json>>();
    bool cont = false;
    for (auto device : devices) {
        showHealthAllComps(out, std::make_shared<nlohmann::json>(device), cont);
        cont = true;
    }
}

static void showHealthMultiDeviceComp(std::ostream &out, std::shared_ptr<nlohmann::json> json, CharTableConfig &healthConfig) {
    auto devices = (*json)["device_list"].get<std::vector<nlohmann::json>>();
    bool cont = false;
    for (auto device : devices) {
        showHealthComp(out, std::make_shared<nlohmann::json>(device), healthConfig, cont);
        cont = true;
    }
}

void ComletHealth::getTableResult(std::ostream &out) {
    auto res = run();
    if (res->contains("error")) {
        out << "Error: " << (*res)["error"].get<std::string>() << std::endl;
        return;
    }
    std::shared_ptr<nlohmann::json> json = std::make_shared<nlohmann::json>();
    *json = *res;

    const int ct = getCompType();
    if (this->opts->listAll) {
        showHealthMultiDevicesAllComps(out, json);
        return;
    }
    if (this->opts->deviceId >= 0) {
        if (ct >= 1 && ct <= 5) {
            switch (ct) {
                case 1:
                    showHealthComp(out, json, ComletConfigHealthCoreTemp);
                    break;
                case 2:
                    showHealthComp(out, json, ComletConfigHealthMemTemp);
                    break;
                case 3:
                    showHealthComp(out, json, ComletConfigHealthPower);
                    break;
                case 4:
                    showHealthComp(out, json, ComletConfigHealthMemory);
                    break;
                case 5:
                    showHealthComp(out, json, ComletConfigHealthFabricPort);
                    break;
            }
        } else {
            showHealthAllComps(out, json);
        }
        return;
    }
    if (this->opts->groupId > 0) {
        if (ct >= 1 && ct <= 5) {
            switch (ct) {
                case 1:
                    showHealthMultiDeviceComp(out, json, ComletConfigHealthCoreTemp);
                    break;
                case 2:
                    showHealthMultiDeviceComp(out, json, ComletConfigHealthMemTemp);
                    break;
                case 3:
                    showHealthMultiDeviceComp(out, json, ComletConfigHealthPower);
                    break;
                case 4:
                    showHealthMultiDeviceComp(out, json, ComletConfigHealthMemory);
                    break;
                case 5:
                    showHealthMultiDeviceComp(out, json, ComletConfigHealthFabricPort);
                    break;
            }
        } else {
            showHealthMultiDevicesAllComps(out, json);
        }
        return;
    }
}
} // end namespace xpum::cli
