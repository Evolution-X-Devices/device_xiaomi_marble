/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "RawLightNotifier"

#include "RawLightNotifier.h"

#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <display/drm/mi_disp.h>
#include <poll.h>
#include <sys/ioctl.h>

#include "SensorNotifierUtils.h"
#include "SscCalApi.h"

static const std::string kDispFeatureDevice = "/dev/mi_display/disp_feature";

// sensor: xiaomi.sensor.ambientlight.raw
static const uint32_t kSensorTypeAmbientlightRaw = 33171111;

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::sensors::V1_0::Event;

namespace {

class RawLightSensorCallback : public IEventQueueCallback {
  public:
    Return<void> onEvent(const Event& e) {
        _oem_msg msg;
        msg.notifyType = REPORT_VALUE;
        msg.value = e.u.vec4.y;
        msg.notifyTypeFloat = msg.notifyType;
        msg.unknown1 = 2;
        msg.unknown2 = 5;
        msg.sensorType = kSensorTypeAmbientlightRaw;

        SscCalApiWrapper::getInstance().processMsg(&msg);

        return Void();
    }
};

}  // namespace

RawLightNotifier::RawLightNotifier(sp<ISensorManager> manager) : SensorNotifier(manager) {
    initializeSensorQueue("xiaomi.sensor.ambientlight.factory", false,
                          new RawLightSensorCallback());
}

RawLightNotifier::~RawLightNotifier() {
    deactivate();
}

void RawLightNotifier::notify() {
    Result res;

    if (mQueue == nullptr) {
        LOG(ERROR) << "failed to initialize sensor queue";
        mActive = false;
        return;
    }

    android::base::unique_fd disp_fd_ =
            android::base::unique_fd(open(kDispFeatureDevice.c_str(), O_RDWR));
    if (disp_fd_.get() == -1) {
        LOG(ERROR) << "failed to open " << kDispFeatureDevice;
    }

    // Enable the sensor initially
    res = mQueue->enableSensor(mSensorHandle, 20000 /* sample period */, 0 /* latency */);
    if (res != Result::OK) {
        LOG(ERROR) << "failed to enable sensor";
    }

    // Register for power events
    disp_event_req req;
    req.base.flag = 0;
    req.base.disp_id = MI_DISP_PRIMARY;
    req.type = MI_DISP_EVENT_POWER;
    ioctl(disp_fd_.get(), MI_DISP_IOCTL_REGISTER_EVENT, &req);

    struct pollfd dispEventPoll = {
            .fd = disp_fd_.get(),
            .events = POLLIN,
    };

    while (mActive) {
        int rc = poll(&dispEventPoll, 1, -1);
        if (rc < 0) {
            LOG(ERROR) << "failed to poll " << kDispFeatureDevice << ", err: " << rc;
            continue;
        }

        std::shared_ptr<disp_event_resp> response = parseDispEvent(disp_fd_.get());
        if (response == nullptr) {
            continue;
        }

        if (response->base.type != MI_DISP_EVENT_POWER) {
            LOG(ERROR) << "unexpected display event: " << response->base.type;
            continue;
        }

        int value = response->data[0];
        LOG(VERBOSE) << "received data: " << std::bitset<8>(value);

        switch (response->data[0]) {
            case MI_DISP_POWER_ON:
                res = mQueue->enableSensor(mSensorHandle, 20000 /* sample period */,
                                           0 /* latency */);
                if (res != Result::OK) {
                    LOG(ERROR) << "failed to enable sensor";
                }
                break;
            default:
                res = mQueue->disableSensor(mSensorHandle);
                if (res != Result::OK) {
                    LOG(ERROR) << "failed to disable sensor";
                }
                break;
        }
    }
}
