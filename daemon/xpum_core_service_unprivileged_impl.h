#pragma once

#include "xpum_core_service_impl.h"

namespace xpum::daemon {
class XpumCoreServiceUnprivilegedImpl : public XpumCoreServiceImpl {
public:
    XpumCoreServiceUnprivilegedImpl(void) {}
    virtual ~XpumCoreServiceUnprivilegedImpl() {}

    virtual ::grpc::Status groupCreate(::grpc::ServerContext* context, const ::GroupName* request, ::GroupInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status groupDestory(::grpc::ServerContext* context, const ::GroupId* request, ::GroupInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status groupAddDevice(::grpc::ServerContext* context, const ::GroupAddRemoveDevice* request, ::GroupInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status groupRemoveDevice(::grpc::ServerContext* context, const ::GroupAddRemoveDevice* request, ::GroupInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status runDiagnostics(::grpc::ServerContext* context, const ::RunDiagnosticsRequest* request, ::DiagnosticsTaskInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status runDiagnosticsByGroup(::grpc::ServerContext* context, const ::RunDiagnosticsByGroupRequest* request, ::DiagnosticsGroupTaskInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status getDiagnosticsResult(::grpc::ServerContext* context, const ::DeviceId* request, ::DiagnosticsTaskInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status getDiagnosticsResultByGroup(::grpc::ServerContext* context, const ::GroupId* request, ::DiagnosticsGroupTaskInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status setHealthConfig(::grpc::ServerContext* context, const ::HealthConfigRequest* request, ::HealthConfigInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status setHealthConfigByGroup(::grpc::ServerContext* context, const ::HealthConfigByGroupRequest* request, ::HealthConfigByGroupInfo* response) override {
        return PD;
    }
    virtual ::grpc::Status getStatisticsNotForPrometheus(::grpc::ServerContext* context, const ::XpumGetStatsRequest* request, ::XpumGetStatsResponse* response) {
        return PD;
    }
    virtual ::grpc::Status getStatisticsByGroupNotForPrometheus(::grpc::ServerContext* context, const ::XpumGetStatsByGroupRequest* request, ::XpumGetStatsResponse* response) {
        return PD;
    }
    virtual ::grpc::Status runFirmwareFlash(::grpc::ServerContext* context, const ::XpumFirmwareFlashJob* request, ::XpumFirmwareFlashJobResponse* response) override {
        return PD;
    }
    virtual ::grpc::Status getFirmwareFlashResult(::grpc::ServerContext* context, const ::XpumFirmwareFlashTaskRequest* request, ::XpumFirmwareFlashTaskResult* response) override {
        return PD;
    }
    virtual ::grpc::Status setPolicy(::grpc::ServerContext* context, const ::SetPolicyRequest* request, ::SetPolicyResponse* response) override {
        return PD;
    }
    virtual ::grpc::Status handleErrorForGetPolicy(xpum_result_t res, ::GetPolicyResponse* response) {
        return PD;
    }
    virtual ::grpc::Status setDeviceSchedulerMode(::grpc::ServerContext* context, const ::ConfigDeviceSchdeulerModeRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setDevicePowerLimit(::grpc::ServerContext* context, const ::ConfigDevicePowerLimitRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setDeviceFrequencyRange(::grpc::ServerContext* context, const ::ConfigDeviceFrequencyRangeRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setDeviceStandbyMode(::grpc::ServerContext* context, const ::ConfigDeviceStandbyRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status resetDevice(::grpc::ServerContext* context, const ::ResetDeviceRequest* request, ::ResetDeviceResponse* response) override {
        return PD;
    }
    virtual ::grpc::Status setPerformanceFactor(::grpc::ServerContext* context, const ::PerformanceFactor* request, ::DevicePerformanceFactorSettingResponse* response) override {
        return PD;
    }
    virtual ::grpc::Status setDeviceFabricPortEnabled(::grpc::ServerContext* context, const ::ConfigDeviceFabricPortEnabledRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setDeviceFabricPortBeaconing(::grpc::ServerContext* context, const ::ConfigDeviceFabricPortBeconingRequest* request, ::ConfigDeviceResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setDeviceMemoryEccState(::grpc::ServerContext* context, const ::ConfigDeviceMemoryEccStateRequest* request, ::ConfigDeviceMemoryEccStateResultData* response) override {
        return PD;
    }
    virtual ::grpc::Status setAgentConfig(::grpc::ServerContext* context, const ::SetAgentConfigRequest* request, ::SetAgentConfigResponse* response) override {
        return PD;
    }

private:
    static const grpc::Status PD;
};


const grpc::Status XpumCoreServiceUnprivilegedImpl::PD( grpc::PERMISSION_DENIED, grpc::string{"You don't have permission to run this command"});
}