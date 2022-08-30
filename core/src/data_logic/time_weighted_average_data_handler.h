/* 
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file time_weighted_average_data_handler.h
 */

#pragma once

#include "metric_statistics_data_handler.h"

namespace xpum {

class TimeWeightedAverageDataHandler : public MetricStatisticsDataHandler {
   public:
    TimeWeightedAverageDataHandler(MeasurementType type, std::shared_ptr<Persistency> &p_persistency);

    virtual ~TimeWeightedAverageDataHandler();

    virtual void handleData(std::shared_ptr<SharedData> &p_data) noexcept;

    void calculateData(std::shared_ptr<SharedData> &p_data);
};
} // end namespace xpum