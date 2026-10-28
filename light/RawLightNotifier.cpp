/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "RawLightNotifier"

#include "RawLightNotifier.h"

#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <fcntl.h>
#include <poll.h>

#include "SensorNotifierUtils.h"
#include "SscCalApi.h"

static const std::string kPanelPowerState =
        "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/panel_power_state";

// SDE_MODE_DPMS_ON
static const int kPanelPowerOn = 0;

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

    android::base::unique_fd fd(open(kPanelPowerState.c_str(), O_RDONLY));
    if (fd.get() == -1) {
        PLOG(ERROR) << "failed to open " << kPanelPowerState;
        mActive = false;
        return;
    }

    struct pollfd powerStatePoll = {
            .fd = fd.get(),
            .events = POLLERR | POLLPRI,
            .revents = POLLPRI,
    };

    while (mActive) {
        int value;
        if (readInt(fd.get(), &value)) {
            if (value == kPanelPowerOn) {
                res = mQueue->enableSensor(mSensorHandle, 20000 /* sample period */,
                                           0 /* latency */);
                if (res != Result::OK) {
                    LOG(ERROR) << "failed to enable sensor";
                }
            } else {
                res = mQueue->disableSensor(mSensorHandle);
                if (res != Result::OK) {
                    LOG(ERROR) << "failed to disable sensor";
                }
            }
        }

        if (poll(&powerStatePoll, 1, -1) < 0) {
            PLOG(ERROR) << "failed to poll " << kPanelPowerState;
        }
    }
}
