/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file comlet_discovery.cpp
 */

#include "comlet_discovery.h"

#include <regex>
#include <map>
#include <stack>
#include <nlohmann/json.hpp>

#include "cli_table.h"
#include "core_stub.h"
#include "utility.h"
#include "exit_code.h"

namespace xpum::cli {

#define ALL_PROP_ID -1

typedef struct dump_prop_config {
    std::string label;
    std::string value;
    int dumpId;
    std::string suffix;
    double scale;
} dump_prop_config;

static nlohmann::json discoveryBasicJson = R"({
    "columns": [{
        "title": "Device ID"
    }, {
        "title": "Device Information"
    }],
    "rows": [{
        "instance": "device_list[]",
        "cells": [
            "device_id", [
                { "label": "Device Name", "value": "device_name" },
                { "label": "Vendor Name", "value": "vendor_name" },
                { "label": "UUID", "value": "uuid" },
                { "label": "PCI BDF Address", "value": "pci_bdf_address" }
            ]
        ]
    }]
})"_json;

static nlohmann::json discoveryDetailedJson = R"({
    "columns": [{
        "title": "Device ID"
    }, {
        "title": "Device Information"
    }],
    "rows": [{
        "instance": "",
        "cells": [
            "device_id", [
                { "label": "Device Type", "value": "device_type"},
                { "label": "Device Name", "value": "device_name", "dumpId": 2 },
                { "label": "Vendor Name", "value": "vendor_name", "dumpId": 3 },
                { "label": "UUID", "value": "uuid", "dumpId": 4 },
                { "label": "Serial Number", "value": "serial_number", "dumpId": 5 },
                { "label": "Core Clock Rate", "value": "core_clock_rate_mhz", "suffix": " MHz", "dumpId": 6 },
                { "label": "Stepping", "value": "device_stepping", "dumpId": 7 },
                { "rowTitle": " " },
                { "label": "Driver Version", "value": "driver_version", "dumpId": 8 },
                { "label": "GFX Firmware Name", "value": "gfx_firmware_name" },
                { "label": "GFX Firmware Version", "value": "gfx_firmware_version", "dumpId": 9 },
                { "label": "GFX Data Firmware Name", "value": "gfx_data_firmware_name" },
                { "label": "GFX Data Firmware Version", "value": "gfx_data_firmware_version", "dumpId": 10 },
                { "rowTitle": " " },
                { "label": "PCI BDF Address", "value": "pci_bdf_address", "dumpId": 11 },
                { "label": "PCI Slot", "value": "pci_slot", "dumpId": 12 },
                { "label": "PCIe Generation", "value": "pcie_generation", "dumpId": 13 },
                { "label": "PCIe Max Link Width", "value": "pcie_max_link_width", "dumpId": 14 },
                { "rowTitle": " " },
                { "label": "Memory Physical Size", "value": "memory_physical_size_byte", "suffix": " MiB", "scale": 1048576, "dumpId": 15 },
                { "label": "Max Mem Alloc Size", "value": "max_mem_alloc_size_byte", "suffix": " MiB", "scale": 1048576 },
                { "label": "Number of Memory Channels", "value": "number_of_memory_channels", "dumpId": 16 },
                { "label": "Memory Bus Width", "value": "memory_bus_width", "dumpId": 17 },
                { "label": "Max Hardware Contexts", "value": "max_hardware_contexts" },
                { "label": "Max Command Queue Priority", "value": "max_command_queue_priority" },
                { "rowTitle": " " },
                { "label": "Number of EUs", "value": "number_of_eus", "dumpId": 18 },
                { "label": "Number of Tiles", "value": "number_of_tiles" },
                { "label": "Number of Slices", "value": "number_of_slices" },
                { "label": "Number of Sub Slices per Slice", "value": "number_of_sub_slices_per_slice" },
                { "label": "Number of Threads per EU", "value": "number_of_threads_per_eu" },
                { "label": "Physical EU SIMD Width", "value": "physical_eu_simd_width" },
                { "label": "Number of Media Engines", "value": "number_of_media_engines", "dumpId": 19 },
                { "label": "Number of Media Enhancement Engines", "value": "number_of_media_enh_engines", "dumpId": 20 },
                { "rowTitle": " " },
                { "label": "Number of Xe Link ports", "value": "number_of_fabric_ports" },
                { "label": "Max Tx/Rx Speed per Xe Link port", "value": "max_fabric_port_speed", "suffix": " MiB/s", "scale": 1048576 },
                { "label": "Number of Lanes per Xe Link port", "value": "number_of_lanes_per_fabric_port" }
            ]
        ]
    }]
})"_json;

static CharTableConfig ComletConfigDiscoveryBasic(discoveryBasicJson);
static CharTableConfig ComletConfigDiscoveryDetailed(discoveryDetailedJson);
static std::vector<dump_prop_config> dumpFieldConfig;


static void readDumpPropConfig(nlohmann::json &conf, std::stack<std::string> &keys, 
                           std::vector<dump_prop_config> &fields) {
    if (keys.empty()) {
        if (conf.is_array()) {
            for (size_t i = 0; i < conf.size(); i++) {
                auto itemDef = conf.at(i);
                if (!itemDef.contains("dumpId")) {
                    continue;
                }
                if (itemDef.contains("label") && itemDef.contains("value")) {
                    dump_prop_config prop;
                    prop.label = itemDef["label"];
                    prop.value = itemDef["value"];
                    prop.suffix = itemDef.contains("suffix") ? itemDef["suffix"] : "";

                    if (itemDef.contains("scale")) {
                        prop.scale = static_cast<double>(itemDef["scale"]);
                    } else {
                        prop.scale = 0;
                    }

                    prop.dumpId = static_cast<int>(itemDef["dumpId"]);
                    fields.push_back(prop);
                }
            } 
        }
        return;
    }

    auto items = conf.find(keys.top());
    keys.pop();
    for (size_t i = 0; i < items->size(); i++) {
        readDumpPropConfig(items->at(i), keys, fields);
    }
}

static void initDumpPropConfig() {
    std::stack<std::string> keys;
    
    keys.push("cells");
    keys.push("rows");

    dumpFieldConfig.clear();
    dumpFieldConfig.push_back(dump_prop_config{"Device ID", "device_id", 1, "", 0});

    readDumpPropConfig(discoveryDetailedJson, keys, dumpFieldConfig);
}

static std::unique_ptr<dump_prop_config> getDumpPropConfig(int dumpId) {
    for (auto& conf : dumpFieldConfig) {
        if (conf.dumpId == dumpId) {
            auto prop = std::make_unique<dump_prop_config>();
            *prop = conf;
            return prop;
        }
    }
    return nullptr;
}

ComletDiscovery::ComletDiscovery() : ComletBase("discovery", "Discover the GPU devices installed on this machine and provide the device info.") {
}

void ComletDiscovery::setupOptions() {
    initDumpPropConfig();
    this->opts = std::unique_ptr<ComletDiscoveryOptions>(new ComletDiscoveryOptions());
#ifndef DAEMONLESS 
    auto deviceIdOpt = addOption("-d,--device", this->opts->deviceId, "Device ID to query. It will show more detailed info.");
#else
    auto deviceIdOpt = addOption("-d,--device", this->opts->deviceId, "Device ID or PCI BDF address to query. It will show more detailed info.");
#endif
    deviceIdOpt->check([](const std::string &str) {
#ifndef DAEMONLESS        
        std::string errStr = "Device id should be integer larger than or equal to 0";
        if (!isValidDeviceId(str)) {
            return errStr;
        }
        return std::string();
#else
    std::string errStr = "Device id should be a non-negative integer or a BDF string";
    if (isValidDeviceId(str)) {
        return std::string();
    } else if (isBDF(str)) {
        return std::string();
    }
    return errStr;
#endif
    });
#ifdef DAEMONLESS
    std::string dumpHelp = "Property ID to dump device properties in CSV format. Separated by the comma. \"-1\" means all properties.";
    for (auto& conf : dumpFieldConfig) {
        dumpHelp += "\n";
        dumpHelp += std::to_string(conf.dumpId);
        dumpHelp += ". ";
        dumpHelp += conf.label;
    }

    auto dumpOpt = addOption("--dump", this->opts->propIdList, dumpHelp);
    dumpOpt->delimiter(',');
    dumpOpt->check([](const std::string &str) {
        std::string errStr = "Invalid Device Propery ID";
        std::regex regex = std::regex("\\,");
        std::vector<std::string> ids(std::sregex_token_iterator(
                                     str.begin(), str.end(), regex, -1),
                                     std::sregex_token_iterator());
        for (auto &id: ids) {
            if (!isInteger(id)) {
                return errStr;
            }

            auto propId = std::stoi(id);
            if (propId == ALL_PROP_ID) {
                continue;
            }
            
            if (getDumpPropConfig(propId) == nullptr) {
                return errStr;
            }
        }
        return std::string();
    });
#else
    auto listamcversionsOpt = addFlag("--listamcversions", this->opts->listamcversions, "Show all AMC firmware versions.");
    deviceIdOpt->excludes(listamcversionsOpt);
#endif
}

std::unique_ptr<nlohmann::json> ComletDiscovery::run() {
    if (this->opts->listamcversions) {
        auto json = this->coreStub->getAMCFirmwareVersions();
        return json;
    }

    if (this->opts->deviceId != "-1") {
        if (isNumber(this->opts->deviceId)) {
            auto json = this->coreStub->getDeviceProperties(std::stoi(this->opts->deviceId));
            return json;
        } else {
            auto json = this->coreStub->getDeviceProperties(this->opts->deviceId.c_str());
            return json;
        }
    }

    if (this->opts->propIdList.size() > 0) {
        auto json = std::make_unique<nlohmann::json>();
        auto deviceListJson = this->coreStub->getDeviceList();
        auto deviceList = (*deviceListJson)["device_list"];
        nlohmann::json deviceJsonList;
        for (auto& device : deviceList) {
            auto deviceDetailedJson = this->coreStub->getDeviceProperties(device["device_id"]);
            auto deviceJson = nlohmann::json::parse(deviceDetailedJson->dump());
            deviceJsonList.push_back(deviceJson);
        }
        (*json)["device_list"] = deviceJsonList;
        return json;
    }

    auto json = this->coreStub->getDeviceList();
    return json;
}

static void showBasicInfo(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
    if (!json->contains("device_list") || (*json)["device_list"].size() <= 0) {
        out << "No device discovered" << std::endl;
        return;
    }

    CharTable table(ComletConfigDiscoveryBasic, *json);
    table.show(out);
}

static void showDetailedInfo(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
    CharTable table(ComletConfigDiscoveryDetailed, *json);
    table.show(out);
}

static void dumpAllDeviceInfo(std::ostream &out, std::shared_ptr<nlohmann::json> json, 
                              std::vector<int> &propIdList) {
    bool dumpAllProp = false;
    for (size_t i = 0; i < propIdList.size(); ++i) {
        if (propIdList[i] == ALL_PROP_ID) {
            dumpAllProp = true;
            break;
        }
    }

    if (dumpAllProp) {
        propIdList.clear();
        for (auto& conf : dumpFieldConfig) {
            propIdList.push_back(conf.dumpId);
        }
    }
                                
    for (size_t i = 0; i < propIdList.size(); ++i) {
        auto prop = getDumpPropConfig(propIdList[i]);
        assert(prop != nullptr);
        if (prop == nullptr) {
            return ;
        }
        if (i > 0) {
            out << ",";
        }
        out << prop->label;
    }
    out << std::endl;

    auto devices = (*json)["device_list"];
    for (auto device : devices) {
        for (size_t i = 0; i < propIdList.size(); ++i) {
            auto prop = getDumpPropConfig(propIdList[i]);
            if (prop == nullptr) {
                return ;
            }
            if (i > 0) {
                out << ",";
            }

            auto value = device[prop->value];
            if (prop->scale > 0) {
                if (prop->suffix != "") {
                   out << "\""; 
                }
                if (value.is_number()) {
                   out << scale_double_value(std::to_string(static_cast<double>(value)),
                                             prop->scale);
                } else if (value.is_string()) {
                    out << scale_double_value(value, prop->scale);
                } else {
                    out << value;
                }
                if (prop->suffix != "") {
                   out << prop->suffix << "\""; 
                }
                continue;
            } else if (value.is_string() && prop->suffix != "") {
                out << "\"" << (std::string(device[prop->value]) + prop->suffix) << "\"";
                continue;
            } else {
                out << device[prop->value];
            }

            if (prop->suffix != "") {
                out << prop->suffix;
            }
        }
        out << std::endl;
    }
}

static void showAmcFwVersion(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
    auto versions = (*json)["amc_fw_version"];
    out << versions.size() << " AMC are found" << std::endl;
    int i = 1;
    for (auto version : versions) {
        out << "AMC " << i++ << " firmware version: " << version.get<std::string>() << std::endl;
    }
}

void ComletDiscovery::getTableResult(std::ostream &out) {
    auto res = run();
    if (res->contains("error")) {
        out << "Error: " << (*res)["error"].get<std::string>() << std::endl;
        setExitCodeByJson(*res);
        return;
    }
    std::shared_ptr<nlohmann::json> json = std::make_shared<nlohmann::json>();
    *json = *res;

    if (this->opts->listamcversions) {
        showAmcFwVersion(out, json);
    } else if (this->opts->propIdList.size() > 0) {
        dumpAllDeviceInfo(out, json, this->opts->propIdList);
    } else if (this->opts->deviceId != "-1") {
        showDetailedInfo(out, json);        
        if (strcasecmp(std::string((*json)["gfx_firmware_version"]).c_str(), "unknown") == 0 ||
            strcasecmp(std::string((*json)["gfx_data_firmware_version"]).c_str(), "unknown") == 0) {
            exit_code = XPUM_CLI_ERROR_FIRMWARE_VERSION_ERROR;    
        }       
    } else {
        showBasicInfo(out, json);
    }
}
} // end namespace xpum::cli
