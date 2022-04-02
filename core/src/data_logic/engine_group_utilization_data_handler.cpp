/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file engine_group_utilization_data_handler.cpp
 */

#include "engine_group_utilization_data_handler.h"

#include <algorithm>
#include <iostream>

#include "core/core.h"
#include "infrastructure/configuration.h"

namespace xpum {

EngineGroupUtilizationDataHandler::EngineGroupUtilizationDataHandler(MeasurementType type,
                                                                     std::shared_ptr<Persistency>& p_persistency)
    : MetricStatisticsDataHandler(type, p_persistency) {
}

EngineGroupUtilizationDataHandler::~EngineGroupUtilizationDataHandler() {
    close();
}

uint32_t EngineGroupUtilizationDataHandler::getAverage(std::vector<uint32_t>& datas) {
    uint32_t sum = 0;
    for (auto& data : datas) {
        sum += data;
    }
    if (datas.size() != 0) {
        return sum / datas.size();
    } else {
        return 0;
    }
}

void EngineGroupUtilizationDataHandler::calculateData(std::shared_ptr<SharedData>& p_data) {
    std::unique_lock<std::mutex> lock(this->mutex);

    std::map<std::string, std::shared_ptr<MeasurementData>>::iterator iter = p_data->getData().begin();
    while (iter != p_data->getData().end()) {
        Property prop;
        Core::instance().getDeviceManager()->getDevice(iter->first)->getProperty(XPUM_DEVICE_PROPERTY_INTERNAL_DEVICE_NAME, prop);
        bool b_ats_device = Utility::isATSPlatform(prop.getValue());

        if (b_ats_device) {
            std::map<uint32_t, std::vector<uint32_t>> group_utilizations;
            auto extended_data = iter->second->getExtendedDatas()->begin();
            while (extended_data != iter->second->getExtendedDatas()->end()) {
                if (extended_data->second.type == ZES_ENGINE_GROUP_COMPUTE_ALL || extended_data->second.type == ZES_ENGINE_GROUP_RENDER_ALL || extended_data->second.type == ZES_ENGINE_GROUP_MEDIA_ALL || extended_data->second.type == ZES_ENGINE_GROUP_COPY_ALL || extended_data->second.type == ZES_ENGINE_GROUP_3D_ALL) {
                    ++extended_data;
                    continue;
                }
                auto pre_data = p_preData->getData().find(iter->first);
                if (pre_data != p_preData->getData().end()) {
                    auto pre_extended = pre_data->second->getExtendedDatas()->find(extended_data->first);
                    if (pre_extended != pre_data->second->getExtendedDatas()->end()) {
                        uint64_t val = Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100 * (extended_data->second.active_time - pre_extended->second.active_time) / (extended_data->second.timestamp - pre_extended->second.timestamp);
                        if (val > Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100) {
                            val = Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100;
                        }
                        group_utilizations[(extended_data->second.on_subdevice ? extended_data->second.subdevice_id : 0)].push_back(val);
                    }
                }
                ++extended_data;
            }

            std::map<uint32_t, std::vector<uint64_t>> utilizations;
            uint32_t i = 0;
            uint32_t num_subdevice = p_data->getData()[iter->first]->getNumSubdevices();
            do {
                utilizations[i].push_back(getAverage(group_utilizations[i]));
                i++;
            } while (i < num_subdevice);
            if (num_subdevice != 0) {
                for (uint32_t i = 0; i < num_subdevice; ++i) {
                    p_data->getData()[iter->first]->setSubdeviceDataCurrent(i, *std::max_element(utilizations[i].begin(), utilizations[i].end()));
                    p_data->getData()[iter->first]->setScale(Configuration::DEFAULT_MEASUREMENT_DATA_SCALE);
                }
            } else {
                p_data->getData()[iter->first]->setCurrent(*std::max_element(utilizations[0].begin(), utilizations[0].end()));
                p_data->getData()[iter->first]->setScale(Configuration::DEFAULT_MEASUREMENT_DATA_SCALE);
            }
        } else {
            auto extended_data = iter->second->getExtendedDatas()->begin();
            while (extended_data != iter->second->getExtendedDatas()->end()) {
                auto pre_data = p_preData->getData().find(iter->first);
                if (pre_data != p_preData->getData().end()) {
                    auto pre_extended = pre_data->second->getExtendedDatas()->find(extended_data->first);
                    if (pre_extended != pre_data->second->getExtendedDatas()->end()) {
                        if (extended_data->second.type == ZES_ENGINE_GROUP_COMPUTE_ALL || extended_data->second.type == ZES_ENGINE_GROUP_RENDER_ALL || extended_data->second.type == ZES_ENGINE_GROUP_MEDIA_ALL || extended_data->second.type == ZES_ENGINE_GROUP_COPY_ALL || extended_data->second.type == ZES_ENGINE_GROUP_3D_ALL) {
                            uint64_t val = Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100 * (extended_data->second.active_time - pre_extended->second.active_time) / (extended_data->second.timestamp - pre_extended->second.timestamp);
                            if (val > Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100) {
                                val = Configuration::DEFAULT_MEASUREMENT_DATA_SCALE * 100;
                            }
                            p_data->getData()[iter->first]->setScale(Configuration::DEFAULT_MEASUREMENT_DATA_SCALE);
                            if (extended_data->second.on_subdevice) {
                                p_data->getData()[iter->first]->setSubdeviceDataCurrent(extended_data->second.subdevice_id, val);
                            } else {
                                p_data->getData()[iter->first]->setCurrent(val);
                            }
                        }
                    }
                }
                ++extended_data;
            }
        }

        ++iter;
    }
}

void EngineGroupUtilizationDataHandler::handleData(std::shared_ptr<SharedData>& p_data) noexcept {
    if (p_preData == nullptr || p_data == nullptr) {
        return;
    }

    calculateData(p_data);
    updateStatistics(p_data);
}

} // end namespace xpum
