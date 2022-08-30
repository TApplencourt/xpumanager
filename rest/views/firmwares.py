#
# Copyright (C) 2021-2022 Intel Corporation
# SPDX-License-Identifier: MIT
# @file firmwares.py
#

from flask import request, jsonify
import stub
from marshmallow import Schema, fields
from marshmallow import Schema, fields, validate, ValidationError, validates_schema


class FirmwareFlashJobOnAllDevicesSchema(Schema):
    file = fields.Str(
        required=True,
        metadata={"description": "The path of firmware binary file to flash"}
    )
    firmware_name = fields.Str(
        validate=validate.OneOf(["AMC"]),
        metadata={"description": "Firmware name, options are: AMC"}
    )


class FirmwareFlashJobOnSingleDeviceSchema(Schema):
    file = fields.Str(
        required=True,
        metadata={"description": "The path of firmware binary file to flash"}
    )
    firmware_name = fields.Str(
        validate=validate.OneOf(["GSC","GSC_DATA"]),
        metadata={"description": "Firmware name, options are: GSC, GSC_DATA"}
    )


class RunFirmwareFlashJobResultSchema(Schema):
    result = fields.Str(metadata={"description": "The result of the query"})
    error = fields.Str(metadata={"description": "Error message"})


def run_firmware_flash_all():
    """Run firmware flash on all devices
    ---
    post:
        tags:
            - "Firmware Flash"
        description: Run firmware flash on all devices
        consumes:
            - application/json
        parameters:
            - 
                name: Firmware flash job
                in: body
                description: Information needed to flash firmware
                schema: FirmwareFlashJobOnAllDevicesSchema
        produces: 
            - application/json
        responses:
            200:
                description: OK
                schema: RunFirmwareFlashJobResultSchema
            500:
                description: Error

    """
    req = request.get_json()
    try:
        FirmwareFlashJobOnAllDevicesSchema().load(req)
    except ValidationError as err:
        return jsonify(err.messages), 400
    return runFirmwareFlash(1024)


def run_firmware_flash_single(deviceId):
    """Run firmware flash on single device or single card
    ---
    post:
        tags:
            - "Firmware Flash"
        description: Run firmware flash on single device or single card
        consumes:
            - application/json
        parameters:
            - 
                name: Firmware flash job
                in: body
                description: Information needed to flash firmware
                schema: FirmwareFlashJobOnSingleDeviceSchema
            -
                name: deviceId
                in: path
                description: Device id
                type: integer
        produces: 
            - application/json
        responses:
            200:
                description: OK
                schema: RunFirmwareFlashJobResultSchema
            500:
                description: Error
    """
    req = request.get_json()
    try:
        FirmwareFlashJobOnSingleDeviceSchema().load(req)
    except ValidationError as err:
        return jsonify(err.messages), 400
    return runFirmwareFlash(deviceId)


def runFirmwareFlash(deviceId):
    req = request.get_json()
    # validate file path
    filePath = req.get('file')
    if not filePath:
        return jsonify({'error': 'missing arguments'}), 400
    pos = filePath.find("/")
    if pos == -1:
        return jsonify({'error': 'Invalid file path, only full path supported'}), 400
    else:
        filePath = filePath[pos:]    # trunc the file path

    fwType = req.get('firmware_name')
    if fwType == 'GSC' and deviceId == 1024:
        return jsonify({'error': 'Updating GSC firmware on all devices is not supported'}), 400
    if fwType == 'GSC_DATA' and deviceId == 1024:
        return jsonify({'error': 'Updating GSC_DATA firmware on all devices is not supported'}), 400
    if fwType == 'AMC' and deviceId != 1024:
        return jsonify({'error': 'Updating AMC firmware on single device is not supported'}), 400

    code, msg, data = stub.runFirmwareFlash(deviceId, fwType, filePath)
    if code == stub.XpumResult['XPUM_UPDATE_FIRMWARE_IGSC_NOT_FOUND'].value:
        return jsonify({'error': msg}), 500
    elif code != 0:
        return jsonify({'error': msg}), 400
    return jsonify(data)


class FirmwareFlashResultQuerySingleDeviceSchema(Schema):
    firmware_name = fields.Str(
        required=True,
        validate=validate.OneOf(["GSC","GSC_DATA"]),
        metadata={"description": "Firmware name, options are: GSC"}
    )


class FirmwareFlashResultQueryAllDevicesSchema(Schema):
    firmware_name = fields.Str(
        required=True,
        validate=validate.OneOf(["AMC"]),
        metadata={"description": "Firmware name, options are: AMC"}
    )


class FirmwareFlashResultSchema(Schema):
    result = fields.Str(
        metadata={"description": "Firmware flash state, OK/FAILED/ONGOING"})
    error = fields.Str(metadata={"description": "Error message"})


def get_firmware_flash_result_all():
    """Get firmware flash state of all devices
    ---
    get:
        tags:
            - "Firmware Flash"
        description: Get firmware flash state of all devices
        consumes:
            - application/json
        parameters:
            - 
                name: Firmware flash job
                in: body
                description: parameters to get firmware flash state
                schema: FirmwareFlashResultQueryAllDevicesSchema
        produces: 
            - application/json
        responses:
            200:
                description: OK
                schema: FirmwareFlashResultSchema
            400:
                description: Bad Request
            500:
                description: Error
    """
    req = request.get_json()
    try:
        FirmwareFlashResultQueryAllDevicesSchema().load(req)
    except ValidationError as err:
        return jsonify(err.messages), 400
    return get_firmware_flash_result(1024)


def get_firmware_flash_result_single(deviceId):
    """Get firmware flash state of single device
    ---
    get:
        tags:
            - "Firmware Flash"
        description: Get firmware flash state of single device
        consumes:
            - application/json
        parameters:
            - 
                name: Firmware flash job
                in: body
                description: parameters to get firmware flash state
                schema: FirmwareFlashResultQuerySingleDeviceSchema
            -
                name: deviceId
                in: path
                description: Device id
                type: integer
        produces: 
            - application/json
        responses:
            200:
                description: OK
                schema: FirmwareFlashResultSchema
            400:
                description: Bad Request
            500:
                description: Error
    """
    req = request.get_json()
    try:
        FirmwareFlashResultQuerySingleDeviceSchema().load(req)
    except ValidationError as err:
        return jsonify(err.messages), 400
    return get_firmware_flash_result(deviceId)


def get_firmware_flash_result(deviceId):
    req = request.get_json()
    fwType = req.get('firmware_name')
    if fwType == "GSC" and deviceId == 1024:
        return jsonify({'error': 'Updating GSC firmware on all devices is not supported'})
    if fwType == "GSC_DATA" and deviceId == 1024:
        return jsonify({'error': 'Updating GSC_DATA firmware on all devices is not supported'})
    if fwType == "AMC" and deviceId != 1024:
        return jsonify({'error': 'Updating AMC firmware on single device is not supported'})

    code, msg, data = stub.getFirmwareFlashResult(deviceId, fwType)
    if code == 0:
        return jsonify(data)
    else:
        return jsonify({'error': msg})
