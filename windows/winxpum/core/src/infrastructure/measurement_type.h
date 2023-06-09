/*
 *  Copyright (C) 2021-2023 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file measurement_type.h
 */

#pragma once

namespace xpum {

    enum MeasurementType {
        METRIC_POWER = 0,
        METRIC_ENERGY,
        METRIC_FREQUENCY,
        METRIC_TEMPERATURE,
        METRIC_MEMORY_USED,
        METRIC_MEMORY_UTILIZATION,
        METRIC_MEMORY_BANDWIDTH,
        METRIC_MEMORY_READ,
        METRIC_MEMORY_WRITE,
        METRIC_MEMORY_READ_THROUGHPUT,
        METRIC_MEMORY_WRITE_THROUGHPUT,
        METRIC_COMPUTATION,
        METRIC_ENGINE_GROUP_COMPUTE_ALL_UTILIZATION,
        METRIC_ENGINE_GROUP_MEDIA_ALL_UTILIZATION,
        METRIC_ENGINE_GROUP_COPY_ALL_UTILIZATION,
        METRIC_ENGINE_GROUP_RENDER_ALL_UTILIZATION,
        METRIC_ENGINE_GROUP_3D_ALL_UTILIZATION,
        METRIC_EU_ACTIVE,
        METRIC_EU_STALL,
        METRIC_EU_IDLE,
        METRIC_RAS_ERROR_CAT_RESET,
        METRIC_RAS_ERROR_CAT_PROGRAMMING_ERRORS,
        METRIC_RAS_ERROR_CAT_DRIVER_ERRORS,
        METRIC_RAS_ERROR_CAT_CACHE_ERRORS_CORRECTABLE,
        METRIC_RAS_ERROR_CAT_CACHE_ERRORS_UNCORRECTABLE,
        METRIC_RAS_ERROR_CAT_DISPLAY_ERRORS_CORRECTABLE,
        METRIC_RAS_ERROR_CAT_DISPLAY_ERRORS_UNCORRECTABLE,
        METRIC_RAS_ERROR_CAT_NON_COMPUTE_ERRORS_CORRECTABLE,
        METRIC_RAS_ERROR_CAT_NON_COMPUTE_ERRORS_UNCORRECTABLE,
        METRIC_REQUEST_FREQUENCY,
        METRIC_MEMORY_TEMPERATURE,
        METRIC_FREQUENCY_THROTTLE,
        METRIC_PCIE_READ_THROUGHPUT,
        METRIC_PCIE_WRITE_THROUGHPUT,
        METRIC_PCIE_READ,
        METRIC_PCIE_WRITE,
        METRIC_ENGINE_UTILIZATION,
        METRIC_FABRIC_THROUGHPUT,
        METRIC_PERF,
        METRIC_FREQUENCY_THROTTLE_REASON_GPU,

        METRIC_MAX,
    };

} // end namespace xpum