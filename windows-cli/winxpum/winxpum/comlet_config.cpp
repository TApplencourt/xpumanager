/*
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file comlet_config.cpp
 */

#include "comlet_config.h"

#include <nlohmann/json.hpp>

#include "core_stub.h"
#include "cli_table.h"



static CharTableConfig ComletDeviceConfiguration(R"({
    "indentation": 4,
    "columns": [{
        "title": "Device Type"
    }, {
        "title": "Device ID/Tile ID"
    }, {
        "title": "Configuration"
    }],
    "rows": [{
        "instance": "",
        "cells": [
            { "rowTitle": "GPU" },
            "device_id", [
                { "label": "Power Limit (w) ", "value": "power_limit" },
                { "label": "  Valid Range", "value": "power_vaild_range" },
                { "label": "Power Average Window (ms) ", "value": "power_average_window" },
                { "label": "  Valid Range", "value": "power_average_window_vaild_range" }
            ]
        ]
    }]
})"_json);

static CharTableConfig ComletTileConfiguration(R"({
    "indentation": 4,
    "columns": [{
        "title": "Device Type"
    }, {
        "title": "Device ID/Tile ID"
    }, {
        "title": "Configuration"
    }],
    "rows": [{
        "instance": "tile_config_data[]",
        "cells": [
            { "rowTitle": "GPU" },
            "tile_id", [
                { "label": "GPU Min Frequency (MHz) ", "value": "min_frequency" },
                { "label": "GPU Max Frequency (MHz) ", "value": "max_frequency" },
                { "label": "  Valid Options", "value": "gpu_frequency_valid_options" },
                {"rowTitle": " " },
                { "label": "Standby Mode", "value": "standby_mode" },
                { "label": "  Valid Options", "value": "standby_mode_valid_options" },
                {"rowTitle": " " },
                { "label": "Scheduler Mode", "value": "scheduler_mode" },
                { "label": "  Timeout (us) ", "value": "scheduler_watchdog_timeout" },
                { "label": "  Interval (us) ", "value": "scheduler_timeslice_interval" },
                { "label": "  Yield Timeout (us) ", "value": "scheduler_timeslice_yield_timeout" },
                {"rowTitle": " " },
                { "label": "Engine Type", "value": "compute_engine" },
                { "label": "  Performance Factor", "value": "compute_performance_factor" },
                { "label": "Engine Type", "value": "media_engine" },
                { "label": "  Performance Factor", "value": "media_performance_factor" },
                {"rowTitle": " " },
                { "label": "Xe Link ports", "value": " " },
                { "label": "  Up", "value": "port_up" },
                { "label": "  Down", "value": "port_down" },
                { "label": "  Beaconing On", "value": "beaconing_on" },
                { "label": "  Beaconing Off", "value": "beaconing_off" }
            ]
        ]
    }]
})"_json);

void ComletConfig::setupOptions() {
    this->opts = std::unique_ptr<ComletConfigOptions>(new ComletConfigOptions());
    addOption("-d,--device", this->opts->deviceId, "device id");
    addOption("-t,--tile", this->opts->tileId, "tile id");
    addOption("--frequencyrange", this->opts->frequencyrange, "GPU tile-level core frequency range.");
    addOption("--powerlimit", this->opts->powerlimit, "Device-level power limit.");
    addOption("--standby", this->opts->standby, "Tile-level standby mode. Valid options: \"default\"; \"never\".");
    addOption("--scheduler", this->opts->scheduler, "Tile-level scheduler mode. Value options: \"timeout\",timeoutValue (us); \"timeslice\",interval (us),yieldtimeout (us);\"exclusive\".The valid range of all time values (us) is from 5000 to 100,000,000.");
    //addFlag("--reset", this->opts->resetDevice, "Hard reset the GPU. All applications that are currently using this device will be forcibly killed.");
    //addOption("--timeslice", this->opts->schedulerTimeslice, "set scheduler timeslice mode");
    //addOption("--timeout", this->opts->schedulerTimeout, "set scheduler timeout mode");
    //addFlag("--exclusive", this->opts->schedulerExclusive, "set scheduler exclusive mode");

    addOption("--performancefactor", this->opts->performancefactor,
        "Set the tile-level performance factor. Valid options: \"compute/media\";factorValue. The factor value should be\n\
between 0 to 100. 100 means that the workload is completely compute bounded and requires very little support from the memory support. 0 means that the workload is completely memory bouded and the performance of the memory controller needs to be increased.");
    addOption("--xelinkport", this->opts->xelinkportEnable, "Change the Xe Link port status. The value 0 means down and 1 means up.");
    addOption("--xelinkportbeaconing", this->opts->xelinkportBeaconing, "Change the Xe Link port beaconing status. The value 0 means off and 1 means on.");
    //addOption("--memoryecc", this->opts->setecc,"Enable/disable memory Ecc setting.");
}
std::vector<std::string> ComletConfig::split(std::string str, std::string delimiter) {
    size_t pos = 0;
    std::string token;
    std::string str1 = str;
    std::vector<std::string> paraList;
    while ((pos = str1.find(delimiter)) != std::string::npos) {
        token = str1.substr(0, pos);
        paraList.push_back(token);
        str1.erase(0, pos + delimiter.length());
    }
    paraList.push_back(str1);
    return paraList;
}

std::unique_ptr<nlohmann::json> ComletConfig::run() {
    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());
    (*json)["return"] = "error";
    if (isQuery()) {
        json = this->coreStub->getDeviceConfig(this->opts->deviceId, this->opts->tileId);
        return json;
    }
    if (this->opts->deviceId >= 0) {
        if (this->opts->tileId >= 0 && !this->opts->scheduler.empty()) {
            (*json)["return"] = "unsupported feature";
            return json;
            std::vector<std::string> paralist = split(this->opts->scheduler, ",");
            int val1, val2;
            std::string command = paralist.at(0);
            std::for_each(command.begin(), command.end(), [](char& c) {
                c = ::tolower(c);
                });
            if (command.compare("timeout") == 0) {
                if (paralist.size() != 2 || paralist.at(1).empty()) {
                    (*json)["return"] = "invalid parameter: timeout";
                    return json;
                }
                val1 = std::stoi(paralist.at(1));
                //json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_TIMEOUT, val1, 0);
            }
            else if (command.compare("timeslice") == 0) {
                if (paralist.size() != 3 || paralist.at(1).empty() || paralist.at(2).empty()) {
                    (*json)["return"] = "invalid parameter: timeslice";
                    return json;
                }
                val1 = std::stoi(paralist.at(1));
                val2 = std::stoi(paralist.at(2));
                //json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_TIMESLICE, val1, val2);
            }
            else if (command.compare("exclusive") == 0) {
                if (paralist.size() != 1) {
                    (*json)["return"] = "invalid parameter: exclusive";
                    return json;
                }
                //json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_EXCLUSIVE, 0, 0);
            }
            else {
                (*json)["return"] = "invalid scheduler mode";
                return json;
            }
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change the scheduler mode on GPU " + std::to_string(this->opts->deviceId) +
                    " tile " + std::to_string(this->opts->tileId) + ".";
            }
            return json;
        }
        else if (/*this->opts->tileId >= 0 &&*/ !this->opts->powerlimit.empty()) {
            std::vector<std::string> paralist = split(this->opts->powerlimit, ",");
            if (paralist.size() == 2 && !paralist.at(0).empty() && !paralist.at(1).empty()) {
                int val1 = std::stoi(paralist.at(0));
                int val2 = std::stoi(paralist.at(1));
                this->opts->tileId = -1;
                json = this->coreStub->setDevicePowerlimit(this->opts->deviceId, this->opts->tileId, val1, val2);
                if ((*json)["status"] == "OK") {
                    (*json)["return"] = "Succeed to set the power limit on GPU " + std::to_string(this->opts->deviceId) /*+
                    " tile " + std::to_string(this->opts->tileId) */
                        + ".";
                }
            }
            else {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }
            return json;
        }
        else if (this->opts->tileId >= 0 && !this->opts->standby.empty()) {
            (*json)["return"] = "unsupported feature";
            return json;
        }
        else if (this->opts->tileId >= 0 && !this->opts->frequencyrange.empty()) {
            std::vector<std::string> paralist = split(this->opts->frequencyrange, ",");
            if (paralist.size() == 2 && !paralist.at(0).empty() && !paralist.at(1).empty()) {
                int val1 = std::stoi(paralist.at(0));
                int val2 = std::stoi(paralist.at(1));
                json = this->coreStub->setDeviceFrequencyRange(this->opts->deviceId, this->opts->tileId, val1, val2);
                if ((*json)["status"] == "OK") {
                    (*json)["return"] = "Succeed to change the core frequency range on GPU " + std::to_string(this->opts->deviceId) +
                        " tile " + std::to_string(this->opts->tileId) + ".";
                }
                return json;
            }
            else {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }
        }
        else if (this->opts->tileId >= 0 && !this->opts->performancefactor.empty()) {
            (*json)["return"] = "unsupported feature";
            return json;
        }
        else if (this->opts->tileId >= 0 && !this->opts->xelinkportEnable.empty()) {
            (*json)["return"] = "unsupported feature";
            return json;
        }
        else if (this->opts->tileId >= 0 && !this->opts->xelinkportBeaconing.empty()) {
            (*json)["return"] = "unsupported feature";
            return json;
        }
#if 0
        else if (!this->opts->setecc.empty()) {
            bool enabled = false;
            int eccVal;
            try {
                eccVal = std::stoi(this->opts->setecc);
            }
            catch (std::invalid_argument const& e) {
                (*json)["return"] = "invalid parameter value";
                return json;
            }
            if (eccVal == 1) {
                enabled = true;
            }
            else if (eccVal == 0) {
                enabled = false;
            }
            else {
                (*json)["return"] = "invalid parameter value";
                return json;
            }
            json = this->coreStub->setMemoryEccState(this->opts->deviceId, enabled);
            if ((*json)["status"] == "OK") {
                std::string available = (*json)["memory_ecc_available"];
                std::string configurable = (*json)["memory_ecc_configurable"];
                std::string current = (*json)["memory_ecc_current_state"];
                std::string pending = (*json)["memory_ecc_pending_state"];
                std::string pendingAction = (*json)["memory_ecc_pending_action"];

                /* (*json)["return"] = "Succeed to set memory Ecc state: available: " + available +
                    " configurable: " + configurable +
                    " current: " + current +
                    " pending: " + pending +
                    " action: " +  pendingAction;*/
                if (available.compare("true") == 0 && configurable.compare("true") == 0) {
                    (*json)["return"] = "Succeed to change the ECC mode to be " + pending + " on GPU "
                        + std::to_string(this->opts->deviceId) + " Please reset GPU or reboot OS to take effect.";
                }
                else {
                    (*json)["return"] = "Failed to change the ECC mode. The current Ecc mode is " + current + ", the pending Ecc mode is " + pending +
                        " and the pending action is " + pendingAction; " on GPU " + std::to_string(this->opts->deviceId);
                }
            }
            return json;
        }
#endif
        /*else if (this->opts->tileId == -1 && this->opts->resetDevice) {
            char confirmed;
            if (this->opts->deviceId >= 0) {
                json = this->coreStub->getDeviceProcessState(this->opts->deviceId);
                std::cout << "The process(es) below are using this device." << "\n";

                for (auto it = (*json)["device_process_list"].begin(); it != (*json)["device_process_list"].end();++it) {
                    std::cout << "PID: " << (*it)["process_id"] << " ,";
                    std::cout << " Command: " << (*it)["process_name"];
                    std::cout << "\n";
                }
                //std::cout << json->dump(4) <<"\n";
                std::cout << "All process(es) above will be forcibly killed if you reset it. Do you want to continue? (Y/N):";
                std::cin >> confirmed;
                if (std::tolower(confirmed) == 'y') {
                    json = this->coreStub->resetDevice(this->opts->deviceId, true);
                }
                else {
                    json->clear();
                    (*json)["status"] = "CANCEL";
                    (*json)["return"] = "Reset is cancelled";
                }
            }
            if ((*json)["status"] == "OK") {
                //json->clear();
                (*json)["return"] = "Succeed to reset the GPU " + std::to_string(this->opts->deviceId);
            }
            return json;
        }*/
        (*json)["return"] = "unknown or invalid command, parameter or device/tile Id";
        return json;
    }
    (*json)["return"] = "invalid device Id";
    return json;
}

static void showConfigurations(std::ostream& out, std::shared_ptr<nlohmann::json> json) {
    CharTable table1(ComletDeviceConfiguration, *json);
    CharTable table2(ComletTileConfiguration, *json, true);
    table1.show(out);
    table2.show(out);
}

static void showPureCommandOutput(std::ostream& out, std::shared_ptr<nlohmann::json> json) {
}

void ComletConfig::getTableResult(std::ostream& out) {
    auto res = run();
    if (res->contains("return")) {
        out << "Return: " << (*res)["return"].get<std::string>() << std::endl;
        return;
    }
    else if (res->contains("error")) {
        out << "Error: " << (*res)["error"].get<std::string>() << std::endl;
        return;
    }
    std::shared_ptr<nlohmann::json> json = std::make_shared<nlohmann::json>();
    *json = *res;

    if (isQuery()) {
        showConfigurations(out, json);
    }
    else {
        showPureCommandOutput(out, json);
    }
}
