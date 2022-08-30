/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file statistics_stub.cpp
 */

#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

#include "core.grpc.pb.h"
#include "core.pb.h"
#include "grpc_core_stub.h"
#include "xpum_structs.h"

namespace xpum::cli {

std::shared_ptr<std::map<int, std::map<int, int>>> GrpcCoreStub::getEngineCount(const char *bdf) {
    std::map<int, std::map<int, int>> m;
    return std::make_shared<std::map<int, std::map<int, int>>>(m);
}

std::shared_ptr<std::map<int, std::map<int, int>>> GrpcCoreStub::getEngineCount(int deviceId) {
    grpc::ClientContext context;
    GetEngineCountRequest request;
    GetEngineCountResponse response;
    request.set_deviceid(deviceId);
    grpc::Status status = stub->getEngineCount(&context, request, &response);

    std::map<int, std::map<int, int>> m;

    for (auto &tileInfo : response.enginecountlist()) {
        std::map<int, int> mm;
        for (auto &countInfo : tileInfo.datalist()) {
            int engineType = countInfo.enginetype();
            int count = countInfo.count();
            mm[engineType] = count;
        }
        int tileId = tileInfo.istilelevel() ? tileInfo.tileid() : -1;
        m[tileId] = mm;
    }
    return std::make_shared<std::map<int, std::map<int, int>>>(m);
}

std::shared_ptr<nlohmann::json> GrpcCoreStub::getFabricCount(const char *bdf) {
    nlohmann::json json;
    return std::make_shared<nlohmann::json>(json);
}

std::shared_ptr<nlohmann::json> GrpcCoreStub::getFabricCount(int deviceId) {
    grpc::ClientContext context;
    GetFabricCountRequest request;
    GetFabricCountResponse response;
    request.set_deviceid(deviceId);
    grpc::Status status = stub->getFabricCount(&context, request, &response);


    nlohmann::json json;
    if (response.errormsg().length() == 0) {
        for (auto &tileInfo : response.fabriccountlist()) {
            std::string tileId = tileInfo.istilelevel() ? std::to_string(tileInfo.tileid()) : "device";
            nlohmann::json obj;
            for (auto &countInfo : tileInfo.datalist()) {
                obj["tile_id"] = countInfo.tileid();
                obj["remote_device_id"] = countInfo.remotedeviceid();
                obj["remote_tile_id"] = countInfo.remotetileid();
            }
            json[tileId].push_back(obj);
        }
    }
    else {
        json["error"] = response.errormsg();
    }
    return std::make_shared<nlohmann::json>(json);
}

std::shared_ptr<nlohmann::json> GrpcCoreStub::getEngineStatistics(int deviceId) {
    grpc::ClientContext engineContext;
    XpumGetEngineStatsRequest engineRequest;
    XpumGetEngineStatsResponse engineResponse;
    engineRequest.set_deviceid(deviceId);
    engineRequest.set_sessionid(0);
    grpc::Status status = stub->getEngineStatistics(&engineContext, engineRequest, &engineResponse);

    // std::shared_ptr<nlohmann::json> json;
    nlohmann::json json;

    if (!status.ok()) {
        json["error"] = status.error_message();
        return std::make_shared<nlohmann::json>(json);
    }

    if (engineResponse.errormsg().length() != 0) {
        if (engineResponse.status() == XPUM_METRIC_NOT_SUPPORTED || engineResponse.status() == XPUM_METRIC_NOT_ENABLED)
            return std::make_shared<nlohmann::json>();
        json["error"] = engineResponse.errormsg();
        return std::make_shared<nlohmann::json>(json);
    }

    // engine data
    for (auto &engineInfo : engineResponse.datalist()) {
        xpum_engine_type_t engineType = (xpum_engine_type_t)engineInfo.enginetype();
        nlohmann::json obj;
        int32_t scale = engineInfo.scale();
        if (scale == 1) {
            obj["value"] = engineInfo.value();
            obj["min"] = engineInfo.min();
            obj["max"] = engineInfo.max();
            obj["avg"] = engineInfo.avg();
        } else {
            obj["value"] = (double)engineInfo.value() / scale;
            obj["min"] = (double)engineInfo.min() / scale;
            obj["max"] = (double)engineInfo.max() / scale;
            obj["avg"] = (double)engineInfo.avg() / scale;
        }
        obj["engine_id"] = engineInfo.engineid();
        std::string tileId = engineInfo.istiledata() ? std::to_string(engineInfo.tileid()) : "device";
        switch (engineType) {
            case XPUM_ENGINE_TYPE_COMPUTE:
                json[tileId]["compute"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_RENDER:
                json[tileId]["render"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_DECODE:
                json[tileId]["decoder"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_ENCODE:
                json[tileId]["encoder"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_COPY:
                json[tileId]["copy"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_MEDIA_ENHANCEMENT:
                json[tileId]["media_enhancement"].push_back(obj);
                break;
            case XPUM_ENGINE_TYPE_3D:
                json[tileId]["3d"].push_back(obj);
                break;
            default:
                break;
        }
    }
    for (auto &item : json.items()) {
        auto key = item.key();
        auto &obj = json[key];
        if (!obj.contains("compute")) {
            obj["compute"] = nlohmann::json::array();
        }
        if (!obj.contains("render")) {
            obj["render"] = nlohmann::json::array();
        }
        if (!obj.contains("decoder")) {
            obj["decoder"] = nlohmann::json::array();
        }
        if (!obj.contains("encoder")) {
            obj["encoder"] = nlohmann::json::array();
        }
        if (!obj.contains("copy")) {
            obj["copy"] = nlohmann::json::array();
        }
        if (!obj.contains("media_enhancement")) {
            obj["media_enhancement"] = nlohmann::json::array();
        }
        if (!obj.contains("3d")) {
            obj["3d"] = nlohmann::json::array();
        }
    }

    return std::make_shared<nlohmann::json>(json);
}

std::shared_ptr<nlohmann::json> GrpcCoreStub::getFabricStatistics(int deviceId) {
    nlohmann::json json;

    grpc::ClientContext context;
    GetFabricStatsRequest request;
    GetFabricStatsResponse response;
    request.set_deviceid(deviceId);
    request.set_sessionid(0);
    grpc::Status status = stub->getFabricStatistics(&context, request, &response);
    if (!status.ok()) {
        json["error"] = status.error_message();
        return std::make_shared<nlohmann::json>(json);
    }
    if (response.errormsg().length() != 0) {
        if (response.status() == XPUM_METRIC_NOT_SUPPORTED || response.status() == XPUM_METRIC_NOT_ENABLED)
            return std::make_shared<nlohmann::json>();
        json["error"] = response.errormsg();
        return std::make_shared<nlohmann::json>(json);
    }

    // fabric data
    json["fabric_throughput"] = nlohmann::json::array();
    for (auto &fabricInfo : response.datalist()) {
        nlohmann::json obj;

        std::stringstream ss;
        if (fabricInfo.type() == XPUM_FABRIC_THROUGHPUT_TYPE_TRANSMITTED) {
            ss << deviceId << "/" << fabricInfo.tileid() << "->" << fabricInfo.remote_device_id() << "/" << fabricInfo.remote_device_tile_id();
        } else if (fabricInfo.type() == XPUM_FABRIC_THROUGHPUT_TYPE_RECEIVED) {
            ss << fabricInfo.remote_device_id() << "/" << fabricInfo.remote_device_tile_id() << "->" << deviceId << "/" << fabricInfo.tileid();
        } else {
            continue;
        }

        int32_t scale = fabricInfo.scale();
        if (scale == 1) {
            obj["value"] = fabricInfo.value();
            obj["min"] = fabricInfo.min();
            obj["max"] = fabricInfo.max();
            obj["avg"] = fabricInfo.avg();
        } else {
            obj["value"] = (double)fabricInfo.value() / scale;
            obj["min"] = (double)fabricInfo.min() / scale;
            obj["max"] = (double)fabricInfo.max() / scale;
            obj["avg"] = (double)fabricInfo.avg() / scale;
        }
        obj["name"] = ss.str();
        obj["tile_id"] = fabricInfo.tileid();
        json["fabric_throughput"].push_back(obj);
    }
    return std::make_shared<nlohmann::json>(json);
}

std::unique_ptr<nlohmann::json> GrpcCoreStub::getStatistics(const char *bdf, bool enableFilter) {
    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());
    return json;
}

std::unique_ptr<nlohmann::json> GrpcCoreStub::getStatistics(int deviceId, bool enableFilter) {
    assert(this->stub != nullptr);

    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());

    grpc::ClientContext context;
    XpumGetStatsResponse response;
    XpumGetStatsRequest request;
    request.set_deviceid(deviceId);
    request.set_sessionid(0);
    request.set_enablefilter(enableFilter);
    grpc::Status status = stub->getStatisticsNotForPrometheus(&context, request, &response);

    if (!status.ok()) {
        (*json)["error"] = status.error_message();
        return json;
    }

    if (response.errormsg().length() != 0) {
        (*json)["error"] = response.errormsg();
        return json;
    }

    // get engine stats
    auto engineStatsJson = getEngineStatistics(deviceId);
    if (engineStatsJson->contains("error")) {
        return std::make_unique<nlohmann::json>(*engineStatsJson);
    }

    // get fabric stats
    auto fabricStatsJson = getFabricStatistics(deviceId);
    if (!fabricStatsJson->contains("error") && !fabricStatsJson->empty()) {
        json->update(*fabricStatsJson);
        // return std::make_unique<nlohmann::json>(*fabricStatsJson);
    }

    std::vector<nlohmann::json> deviceJsonList;
    uint64_t begin = response.begin();
    uint64_t end = response.end();
    (*json)["begin"] = isotimestamp(begin);
    (*json)["end"] = isotimestamp(end);

    (*json)["elapsed_time"] = (end - begin) / 1000;

    std::vector<nlohmann::json> deviceLevelStatsDataList;
    std::vector<nlohmann::json> tileLevelStatsDataList;

    for (int i{0}; i < response.datalist_size(); i++) {
        auto &stats_info = response.datalist(i);
        std::vector<nlohmann::json> dataList;
        for (int j = 0; j < stats_info.datalist_size(); j++) {
            auto &stats_data = stats_info.datalist(j);
            auto tmp = nlohmann::json();
            xpum_stats_type_t metricsType = (xpum_stats_type_t)stats_data.metricstype().value();
            tmp["metrics_type"] = metricsTypeToString(metricsType);
            int32_t scale = stats_data.scale();
            if (scale == 1) {
                tmp["value"] = stats_data.value();
                if (!stats_data.iscounter()) {
                    tmp["avg"] = stats_data.avg();
                    tmp["min"] = stats_data.min();
                    tmp["max"] = stats_data.max();
                } else {
                    tmp["total"] = stats_data.accumulated();
                }
            } else {
                tmp["value"] = (double)stats_data.value() / scale;
                if (!stats_data.iscounter()) {
                    tmp["avg"] = (double)stats_data.avg() / scale;
                    tmp["min"] = (double)stats_data.min() / scale;
                    tmp["max"] = (double)stats_data.max() / scale;
                } else {
                    tmp["total"] = (double)stats_data.accumulated() / scale;
                }
            }
            dataList.push_back(tmp);
        }
        if (stats_info.istiledata()) {
            auto tmp = nlohmann::json();
            tmp["tile_id"] = stats_info.tileid();
            tmp["data_list"] = dataList;
            auto strTileId = std::to_string(stats_info.tileid());
            if (engineStatsJson->contains(strTileId)) {
                tmp["engine_util"] = (*engineStatsJson)[strTileId];
            }
            tileLevelStatsDataList.push_back(tmp);
        } else {
            deviceLevelStatsDataList.insert(deviceLevelStatsDataList.end(), dataList.begin(), dataList.end());
        }
    }
    if (engineStatsJson->contains("device")) {
        (*json)["engine_util"] = (*engineStatsJson)["device"];
    }
    (*json)["device_level"] = deviceLevelStatsDataList;
    if (tileLevelStatsDataList.size() > 0)
        (*json)["tile_level"] = tileLevelStatsDataList;

    (*json)["device_id"] = deviceId;

    return json;
}

std::unique_ptr<nlohmann::json> GrpcCoreStub::getStatisticsByGroup(uint32_t groupId, bool enableFilter) {
    assert(this->stub != nullptr);

    auto json = std::unique_ptr<nlohmann::json>(new nlohmann::json());

    grpc::ClientContext context;
    XpumGetStatsResponse response;
    XpumGetStatsByGroupRequest request;
    request.set_groupid(groupId);
    request.set_sessionid(0);
    request.set_enablefilter(enableFilter);
    grpc::Status status = stub->getStatisticsByGroupNotForPrometheus(&context, request, &response);

    if (!status.ok()) {
        (*json)["error"] = status.error_message();
    }

    if (response.errormsg().length() != 0) {
        (*json)["error"] = response.errormsg();
        return json;
    }

    auto deviceMap = nlohmann::json();

    for (int i{0}; i < response.datalist_size(); i++) {
        auto &stats_info = response.datalist(i);

        std::string key = std::to_string(stats_info.deviceid());

        if (!deviceMap.contains(key)) {
            auto tmp = nlohmann::json();
            tmp["device_level"] = std::vector<nlohmann::json>();
            tmp["tile_level"] = std::vector<nlohmann::json>();
            tmp["device_id"] = stats_info.deviceid();
            deviceMap[key] = tmp;
        }

        std::vector<nlohmann::json> dataList;

        for (int j = 0; j < stats_info.datalist_size(); j++) {
            auto &stats_data = stats_info.datalist(j);
            auto tmp = nlohmann::json();
            xpum_stats_type_t metricsType = (xpum_stats_type_t)stats_data.metricstype().value();
            tmp["metrics_type"] = metricsTypeToString(metricsType);
            int32_t scale = stats_data.scale();
            if (scale == 1) {
                tmp["value"] = stats_data.value();
                if (!stats_data.iscounter()) {
                    tmp["avg"] = stats_data.avg();
                    tmp["min"] = stats_data.min();
                    tmp["max"] = stats_data.max();
                } else {
                    tmp["total"] = stats_data.accumulated();
                }
            } else {
                tmp["value"] = (double)stats_data.value() / scale;
                if (!stats_data.iscounter()) {
                    tmp["avg"] = (double)stats_data.avg() / scale;
                    tmp["min"] = (double)stats_data.min() / scale;
                    tmp["max"] = (double)stats_data.max() / scale;
                } else {
                    tmp["total"] = (double)stats_data.accumulated() / scale;
                }
            }
            dataList.push_back(tmp);
        }

        if (stats_info.istiledata()) {
            auto tmp = nlohmann::json();
            tmp["tile_id"] = stats_info.tileid();
            tmp["data_list"] = dataList;

            deviceMap[key]["tile_level"].push_back(tmp);
        } else {
            deviceMap[key]["device_level"] = dataList;
        }
    }

    uint64_t begin = response.begin();
    uint64_t end = response.end();
    uint32_t elapsed_time = (end - begin) / 1000;
    std::string beginTimestamp = isotimestamp(begin);
    std::string endTimestamp = isotimestamp(end);

    auto datas = std::vector<nlohmann::json>();
    for (auto &item : deviceMap.items()) {
        nlohmann::json data;
        int deviceId = item.value()["device_id"].get<int>();
        data["begin"] = beginTimestamp;
        data["end"] = endTimestamp;
        data["elapsed_time"] = elapsed_time;
        data["device_id"] = item.value()["device_id"];
        data["device_level"] = item.value()["device_level"];
        // get engine stats
        auto engineStatsJson = getEngineStatistics(deviceId);
        if (engineStatsJson->contains("error")) {
            return std::make_unique<nlohmann::json>(*engineStatsJson);
        }
        if (engineStatsJson->contains("device")) {
            data["engine_util"] = (*engineStatsJson)["device"];
        }
        if (item.value().contains("tile_level")) {
            auto &tileLevelObj = item.value()["tile_level"];
            for (auto &tileObj : tileLevelObj) {
                std::string strTileId = std::to_string(tileObj["tile_id"].get<int>());
                if (engineStatsJson->contains(strTileId)) {
                    tileObj["engine_util"] = (*engineStatsJson)[strTileId];
                }
            }
            data["tile_level"] = tileLevelObj;
        }
        // get fabric stats
        auto fabricStatsJson = getFabricStatistics(deviceId);
        if (!fabricStatsJson->contains("error") && !fabricStatsJson->empty()) {
            // return std::make_unique<nlohmann::json>(*fabricStatsJson);
            data.update(*fabricStatsJson);
        }
        datas.push_back(data);
    }

    (*json)["datas"] = datas;
    (*json)["group_id"] = groupId;

    return json;
}
} // end namespace xpum::cli
