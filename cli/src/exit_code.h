/*
 *  Copyright (C) 2021-2022 Intel Corporation
 *  SPDX-License-Identifier: MIT
 *  @file exit_code.h
 */

#pragma once

#define XPUM_CLI_SUCCESS                                                    0   // Success
#define XPUM_CLI_ERROR_GENERIC_ERROR                                        1   // Generic error
#define XPUM_CLI_ERROR_BAD_ARGUMENT                                         2   // Bad argument
#define XPUM_CLI_ERROR_BUFFER_TOO_SMALL                                     3   // The buffer pass to function is too small
#define XPUM_CLI_ERROR_DEVICE_NOT_FOUND                                     4   // Device not found
#define XPUM_CLI_ERROR_TILE_NOT_FOUND                                       5   // Tile not found
#define XPUM_CLI_ERROR_GROUP_NOT_FOUND                                      6   // Group not found
#define XPUM_CLI_ERROR_POLICY_TYPE_INVALID                                  7   // Policy type is invalid
#define XPUM_CLI_ERROR_POLICY_ACTION_TYPE_INVALID                           8   // Policy action type is invalid
#define XPUM_CLI_ERROR_POLICY_CONDITION_TYPE_INVALID                        9   // Policy condtion type is invalid
#define XPUM_CLI_ERROR_POLICY_TYPE_ACTION_NOT_SUPPORT                       10  // Policy type and policy action not match
#define XPUM_CLI_ERROR_POLICY_TYPE_CONDITION_NOT_SUPPORT                    11  // Policy type and condition type not match
#define XPUM_CLI_ERROR_POLICY_INVALID_THRESHOLD                             12  // Policy threshold invalid
#define XPUM_CLI_ERROR_POLICY_INVALID_FREQUENCY                             13  // Policy frequency invalid
#define XPUM_CLI_ERROR_POLICY_NOT_EXIST                                     14  // Policy not exist
#define XPUM_CLI_ERROR_DIAGNOSTIC_TASK_NOT_COMPLETE                         15  
#define XPUM_CLI_ERROR_GROUP_DEVICE_DUPLICATED                              16  
#define XPUM_CLI_ERROR_GROUP_CHANGE_NOT_ALLOWED                             17  
#define XPUM_CLI_ERROR_NOT_INITIALIZED                                      18  // XPUM is not initialized.
#define XPUM_CLI_ERROR_DUMP_RAW_DATA_TASK_NOT_EXIST                         19  // Dump raw data task not exists
#define XPUM_CLI_ERROR_DUMP_RAW_DATA_ILLEGAL_DUMP_FILE_PATH                 20  // Dump file path provide is illegal
#define XPUM_CLI_ERROR_UNKNOWN_AGENT_CONFIG_KEY                             21  // The the key for agent setting is unknown
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_ILLEGAL_FILENAME                     22  // File name can not contain following characters: {}()><&*'|=?;[]$-#~!"%:+,`
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_IMAGE_FILE_NOT_FOUND                 23  // Firmware image not found
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_UNSUPPORTED_AMC                      24  // Update AMC firmware on target device not supported
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_UNSUPPORTED_AMC_SINGLE               25  // Update AMC firmware on single device not supported
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_UNSUPPORTED_GFX_ALL                  26  // Update GFX firmware on all device not supported
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_MODEL_INCONSISTENCE                  27  // Devices models are inconsistent
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_IGSC_NOT_FOUND                       28  // "/usr/bin/igsc" not found
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_TASK_RUNNING                         29  // Firmware update task is already running
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_INVALID_FW_IMAGE                     30  // The image file is not a valid FW image file
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_FW_IMAGE_NOT_COMPATIBLE_WITH_DEVICE  31  // The image file is not compatible with device
#define XPUM_CLI_ERROR_DUMP_METRICS_TYPE_NOT_SUPPORT                        32  // Dump metrics not supported
#define XPUM_CLI_ERROR_METRIC_NOT_SUPPORTED                                 33  // Unsupported metric
#define XPUM_CLI_ERROR_METRIC_NOT_ENABLED                                   34  // Unenabled metric
#define XPUM_CLI_ERROR_HEALTH_INVALID_TYPE                                  35  
#define XPUM_CLI_ERROR_HEALTH_INVALID_CONIG_TYPE                            36  
#define XPUM_CLI_ERROR_HEALTH_INVALID_THRESHOLD                             37  
#define XPUM_CLI_ERROR_DIAGNOSTIC_INVALID_LEVEL                             38  
#define XPUM_CLI_ERROR_AGENT_SET_INVALID_VALUE                              39  // Agent set value is invalid
#define XPUM_CLI_ERROR_LEVEL_ZERO_INITIALIZATION_ERROR                      40  // Level Zero initialization error.
#define XPUM_CLI_ERROR_UNSUPPORTED_SESSIONID                                41  // Unsupported session id
#define XPUM_CLI_ERROR_UPDATE_FIRMWARE_FAIL                                 42  // Fail to update firmware
#define XPUM_CLI_ERROR_DIAGNOSTIC_TASK_TIMEOUT                              43
#define XPUM_CLI_ERROR_OPEN_FILE                                            44  // Fail to open a file
#define XPUM_CLI_ERROR_EMPTY_XML                                            45  // xpumExportTopology2XML returns an error, and then the exported xml file is empty
#define XPUM_CLI_ERROR_DIAGNOSTIC_TASK_FAILED                               46
#define XPUM_CLI_ERROR_FIRMWARE_VERSION_ERROR                               47
#define XPUM_CLI_ERROR_MEMORY_ECC_LIB_NOT_SUPPORT                           48  

int errorNumTranslate(int coreErrNo);
