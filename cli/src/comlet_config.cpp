/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file comlet_config.cpp
 */

#include "comlet_config.h"

#include <nlohmann/json.hpp>

#include "cli_table.h"
#include "core_stub.h"
#include "utility.h"
#include "exit_code.h"

namespace xpum::cli {

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
                {"rowTitle": " " },
                { "label": "Memory ECC", "value": " " },
                { "label": "  Current", "value": "memory_ecc_current_state" },
                { "label": "  Pending", "value": "memory_ecc_pending_state" }
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
/*
                { "label": "  Available", "value": "memory_ecc_available" },
                { "label": "  Configurable", "value": "memory_ecc_configurable" },
                { "label": "  Action", "value": "memory_ecc_pending_action" },
*/
void ComletConfig::setupOptions() {
    this->opts = std::unique_ptr<ComletConfigOptions>(new ComletConfigOptions());
#ifndef DAEMONLESS
     auto deviceIdOpt = addOption("-d,--device", this->opts->deviceId, "device id");
#else
     auto deviceIdOpt = addOption("-d,--device", this->opts->device, "The device ID or PCI BDF address to query");
#endif

    deviceIdOpt->check([this](const std::string &str) {
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

    addOption("-t,--tile", this->opts->tileId, "The tile ID");
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
    addOption("--memoryecc", this->opts->setecc,"Enable/disable memory ECC setting. 0:disable; 1:enable");
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
    if (this->opts->device != "") {
        if (isNumber(this->opts->device)) {
            this->opts->deviceId = std::stoi(this->opts->device);
        } else {
            auto json = this->coreStub->getDeivceIdByBDF(
                this->opts->device.c_str(), &this->opts->deviceId);
            if (json->contains("error")) {
                return json;
            }
        }
    }
    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());
    (*json)["return"] = "error";
    if (isQuery()) {
        json = this->coreStub->getDeviceConfig(this->opts->deviceId, this->opts->tileId);
        return json;
    }
    if (this->opts->deviceId >= 0) {
        if (this->opts->tileId >= 0 && !this->opts->scheduler.empty()) {
            std::vector<std::string> paralist = split(this->opts->scheduler, ",");
            int val1, val2;
            std::string command = paralist.at(0);
            std::for_each(command.begin(), command.end(), [](char &c) {
                c = ::tolower(c);
            });
            if (command.compare("timeout") == 0) {
                if (paralist.size() != 2 || paralist.at(1).empty()) {
                    (*json)["return"] = "invalid parameter: timeout";
                    return json;
                }

                try {
                    val1 = std::stoi(paralist.at(1));
                } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: timeout";
                    return json;
                }

                if (val1 <= 0) {
                    (*json)["return"] = "invalid parameter: timeout should bigger than 0.";
                    return json;
                }

                json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_TIMEOUT,
                                                              val1, 0);
            } else if (command.compare("timeslice") == 0) {
                if (paralist.size() != 3 || paralist.at(1).empty() || paralist.at(2).empty()) {
                    (*json)["return"] = "invalid parameter: timeslice";
                    return json;
                }
                try {
                    val1 = std::stoi(paralist.at(1));
                    val2 = std::stoi(paralist.at(2));
                } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: timeslice";
                    return json;
                }

                if (val1 <= 0 || val2 <= 0) {
                    (*json)["return"] = "invalid parameter: time slice should bigger than 0.";
                    return json;
                }
                json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_TIMESLICE, val1, val2);
            } else if (command.compare("exclusive") == 0) {
                if (paralist.size() != 1) {
                    (*json)["return"] = "invalid parameter: exclusive";
                    return json;
                }
                json = this->coreStub->setDeviceSchedulerMode(this->opts->deviceId, this->opts->tileId, SCHEDULER_EXCLUSIVE, 0, 0);
            } else {
                (*json)["return"] = "invalid scheduler mode";
                return json;
            }
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change the scheduler mode on GPU " + std::to_string(this->opts->deviceId) +
                                    " tile " + std::to_string(this->opts->tileId) + ".";
            }
            return json;
        } else if (/*this->opts->tileId >= 0 &&*/ !this->opts->powerlimit.empty()) {
            std::vector<std::string> paralist = split(this->opts->powerlimit, ",");
            if (paralist.size() >= 1 && !paralist.at(0).empty()) {
                int val1;
                try {
                    val1 = std::stoi(paralist.at(0));
                } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: powerlimit";
                    return json;
                }

                if (paralist.size() == 2 && paralist.at(1).empty()) {
                    (*json)["return"] = "invalid parameter: please check help information";
                    return json;
                }
                if (val1 <= 0) {
                    (*json)["return"] = "invalid parameter: power limit should bigger than 0.";
                    return json;
                }
                int val2 = 0; //std::stoi(paralist.at(1));
                this->opts->tileId = -1;
                json = this->coreStub->setDevicePowerlimit(this->opts->deviceId, this->opts->tileId, val1, val2);
                if ((*json)["status"] == "OK") {
                    (*json)["return"] = "Succeed to set the power limit on GPU " + std::to_string(this->opts->deviceId) /*+
                    " tile " + std::to_string(this->opts->tileId) */
                                        + ".";
                }
            } else {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }
            return json;
        } else if (this->opts->tileId >= 0 && !this->opts->standby.empty()) {
            XpumStandbyMode mode;
            std::for_each(this->opts->standby.begin(), this->opts->standby.end(), [](char &c) {
                c = ::tolower(c);
            });
            if (this->opts->standby.compare("never") == 0) {
                mode = STANDBY_NEVER;
            } else if (this->opts->standby.compare("default") == 0) {
                mode = STANDBY_DEFAULT;
            } else {
                (*json)["return"] = "invalid parameter: standby mode";
                return json;
            }
            json = this->coreStub->setDeviceStandby(this->opts->deviceId, this->opts->tileId, mode);
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change the standby mode on GPU " + std::to_string(this->opts->deviceId) +
                                    " tile " + std::to_string(this->opts->tileId) + ".";
            }
            return json;
        } else if (this->opts->tileId >= 0 && !this->opts->frequencyrange.empty()) {
            std::vector<std::string> paralist = split(this->opts->frequencyrange, ",");
            if (paralist.size() == 2 && !paralist.at(0).empty() && !paralist.at(1).empty()) {
                int val1,val2;
                try {
                    val1 = std::stoi(paralist.at(0));
                    val2 = std::stoi(paralist.at(1));
                } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: frequency range";
                    return json;
                }

                if (val1 <= 0 || val2 <= 0) {
                    (*json)["return"] = "invalid parameter: min/max frequency should bigger than 0.";
                    return json;
                }
                json = this->coreStub->setDeviceFrequencyRange(this->opts->deviceId, this->opts->tileId, val1, val2);
                if ((*json)["status"] == "OK") {
                    (*json)["return"] = "Succeed to change the core frequency range on GPU " + std::to_string(this->opts->deviceId) +
                                        " tile " + std::to_string(this->opts->tileId) + ".";
                }
                return json;
            } else {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }
        } else if (this->opts->tileId >= 0 && !this->opts->performancefactor.empty()) {
            std::vector<std::string> paralist = split(this->opts->performancefactor, ",");
            if (paralist.size() != 2 || paralist.at(1).empty()) {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }
            std::string engine = paralist.at(0);
            std::for_each(engine.begin(), engine.end(), [](char &c) { c = ::tolower(c); });
            xpum_engine_type_flags_t engineType;

            if (engine.compare("compute") == 0) {
                engineType = XPUM_COMPUTE;
            } else if (engine.compare("media") == 0) {
                engineType = XPUM_MEDIA;
            } else {
                (*json)["return"] = "invalid engine";
                return json;
            }
            double val1 = std::stod(paralist.at(1));
            if (val1 < 0.0 || val1 > 100.0) {
                (*json)["return"] = "invalid factor";
                return json;
            }
            json = this->coreStub->setPerformanceFactor(this->opts->deviceId, this->opts->tileId, engineType, val1);
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change the " + engine + " performance factor to " + paralist.at(1) +
                                    " on GPU " + std::to_string(this->opts->deviceId) +
                                    " tile " + std::to_string(this->opts->tileId) + ".";
            }
            return json;
        } else if (this->opts->tileId >= 0 && !this->opts->xelinkportEnable.empty()) {
            std::vector<std::string> paralist = split(this->opts->xelinkportEnable, ",");
            if (paralist.size() != 2 || paralist.at(1).empty()) {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }

            int port;
            int enabled;
            try {
                port = std::stoi(paralist.at(0));
                enabled = std::stoi(paralist.at(1));
            } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: xeLink port";
                    return json;
            }

            if ((enabled != 0 && enabled != 1) || port < 0) {
                (*json)["return"] = "invalid parameter enabled";
                return json;
            }
            json = this->coreStub->setFabricPortEnabled(this->opts->deviceId, this->opts->tileId, port, enabled);
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change Xe Link port " + paralist.at(0) + " to " + (enabled == 1 ? "up" : "down") + " .";
            }
            return json;
        } else if (this->opts->tileId >= 0 && !this->opts->xelinkportBeaconing.empty()) {
            std::vector<std::string> paralist = split(this->opts->xelinkportBeaconing, ",");
            if (paralist.size() != 2 || paralist.at(1).empty()) {
                (*json)["return"] = "invalid parameter: please check help information";
                return json;
            }

            int port;
            int beaconing;
            try {
                port = std::stoi(paralist.at(0));
                beaconing = std::stoi(paralist.at(1));
            } catch (std::invalid_argument const& e) {
                    (*json)["return"] = "invalid parameter: xeLink beaconing";
                    return json;
            }

            if (beaconing != 0 && beaconing != 1) {
                (*json)["return"] = "invalid parameter value: beaconing";
                return json;
            }
            json = this->coreStub->setFabricPortBeaconing(this->opts->deviceId, this->opts->tileId, port, beaconing);
            if ((*json)["status"] == "OK") {
                (*json)["return"] = "Succeed to change Xe Link port " + paralist.at(0) + " beaconing to " + (beaconing == 1 ? "on" : "off") + " .";
            }
            return json;
        }
        else if (!this->opts->setecc.empty()) {
            bool enabled = false;
            int eccVal;
            try {
                eccVal = std::stoi(this->opts->setecc);
            } catch (std::invalid_argument const &e) {
                (*json)["return"]="invalid parameter value";
                return json;     
            }
            if (eccVal == 1) {
                enabled = true;
            } else if (eccVal == 0) {
                enabled = false;
            } else {
                (*json)["return"]="invalid parameter value";
                return json;    
            }
            json = this->coreStub->setMemoryEccState(this->opts->deviceId, enabled);
            if((*json)["status"] == "OK") {
                std::string available = (*json)["memory_ecc_available"];
                std::string configurable = (*json)["memory_ecc_configurable"];
                std::string current = (*json)["memory_ecc_current_state"];
                std::string pending = (*json)["memory_ecc_pending_state"];
                std::string pendingAction = (*json)["memory_ecc_pending_action"];
                (*json)["return"] = "Successfully " + (enabled ? std::string("enable") : std::string("disable")) + " ECC memory on GPU " + std::to_string(this->opts->deviceId)+". Please reset the GPU or reboot the OS for the change to take effect.";
            }
            return json;  
        }
        else if (this->opts->tileId == -1 && this->opts->resetDevice) {
            char confirmed;
            if (this->opts->deviceId >= 0) {
                json = this->coreStub->getDeviceProcessState(this->opts->deviceId);
                std::cout <<"The process(es) below are using this device."<<"\n";

                for(auto it= (*json)["device_process_list"].begin(); it!=(*json)["device_process_list"].end();++it) {
                    std::cout <<"PID: "<<(*it)["process_id"] <<" ,";
                    std::cout <<" Command: "<<(*it)["process_name"];
                    std::cout<<"\n";
                }
                //std::cout << json->dump(4) <<"\n";
                std::cout <<"All process(es) above will be forcibly killed if you reset it. Do you want to continue? (Y/N):";
                std::cin>>confirmed;
                if (std::tolower(confirmed) == 'y') {
                    json = this->coreStub->resetDevice(this->opts->deviceId, true); 
                } else {
                    json->clear();
                    (*json)["status"] = "CANCEL";
                    (*json)["return"] = "Reset is cancelled";
                }
            }
            if((*json)["status"] == "OK") {
                //json->clear();
                (*json)["return"] = "Succeed to reset the GPU "+ std::to_string(this->opts->deviceId);
            }
            return json;
        }
        (*json)["return"] = "unknown or invalid command, parameter or device/tile Id";
        return json;
    }
    (*json)["return"] = "invalid device Id";
    return json;
}

static void showConfigurations(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
    CharTable table1(ComletDeviceConfiguration, *json);
    CharTable table2(ComletTileConfiguration, *json, true);
    table1.show(out);
    table2.show(out);
}

static void showPureCommandOutput(std::ostream &out, std::shared_ptr<nlohmann::json> json) {
}

void ComletConfig::getTableResult(std::ostream &out) {
    auto res = run();
    if (res->contains("return")) {
        out << "Return: " << (*res)["return"].get<std::string>() << std::endl;
        if ((res->contains("status") == false) || 
            ((*res)["status"] != "OK" && (*res)["status"] != "CANCEL")) {
            exit_code = XPUM_CLI_ERROR_BAD_ARGUMENT;
        }
        return;
    } else if (res->contains("error")) {
        out << "Error: " << (*res)["error"].get<std::string>() << std::endl;
        setExitCodeByJson(*res);
        return;
    }
    std::shared_ptr<nlohmann::json> json = std::make_shared<nlohmann::json>();
    *json = *res;

    if (isQuery()) {
        showConfigurations(out, json);
    } else {
        showPureCommandOutput(out, json);
    }
}

} // end namespace xpum::cli
