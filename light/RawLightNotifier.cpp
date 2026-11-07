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

static const std::string kDisplayPrimary = "/sys/devices/platform/soc/soc:qcom,dsi-display-primary";

static const std::vector<std::pair<std::string, notify_t>> kDisplayNodes = {
        {"panel_power_state", POWER_STATE},
        {"dynamic_fps", DISPLAY_FREQUENCY},
        {"dbv", BRIGHTNESS},
};

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

RawLightNotifier::RawLightNotifier(sp<ISensorManager> manager) : SensorNotifier(manager),
                     isEnable(false) {
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

    // Enable the sensor initially
    res = mQueue->enableSensor(mSensorHandle, 20000 /* sample period */, 0 /* latency */);
    if (res != Result::OK) {
        LOG(ERROR) << "failed to enable sensor";
    } else isEnable = true;

    std::vector<android::base::unique_fd> fds;
    std::vector<struct pollfd> pollfds;
    std::vector<notify_t> targets;
    for (const auto& [node, notifyType] : kDisplayNodes) {
        std::string nodePath = kDisplayPrimary + "/" + node;
        android::base::unique_fd fd(open(nodePath.c_str(), O_RDONLY));
        if (fd.get() == -1) {
            PLOG(ERROR) << "failed to open " << nodePath;
            continue;
        }

        pollfds.push_back({.fd = fd.get(), .events = POLLERR | POLLPRI, .revents = 0});
        targets.push_back(notifyType);
        fds.push_back(std::move(fd));
    }

    if (pollfds.empty()) {
        mActive = false;
        return;
    }

    _oem_msg msg;

    // Read once so that the next poll only returns on changes
    for (size_t i = 0; i < pollfds.size(); i++) {
        pollfds[i].revents = POLLPRI;
    }

    while (mActive) {
        for (size_t i = 0; i < pollfds.size(); i++) {
            if (!(pollfds[i].revents & (POLLERR | POLLPRI))) {
                continue;
            }

            int value;
            if (!readInt(pollfds[i].fd, &value)) {
                continue;
            }

            if (targets[i] == POWER_STATE) {
                if (value == kPanelPowerOn) {
                    if (!isEnable) {
                        res = mQueue->enableSensor(mSensorHandle, 20000 /* sample period */,
                                                   0 /* latency */);
                        if (res != Result::OK) {
                            LOG(ERROR) << "failed to enable sensor";
                        } else isEnable = true;
                    }
                } else if (isEnable) {
                    res = mQueue->disableSensor(mSensorHandle);
                    if (res != Result::OK) {
                        LOG(ERROR) << "failed to disable sensor";
                    } else isEnable = false;
                }
            }

            msg.sensorType = kSensorTypeAmbientlightRaw;
            msg.notifyType = targets[i];
            msg.notifyTypeFloat = targets[i];
            msg.value = value;
            msg.unknown1 = 1;
            msg.unknown2 = 5;

            SscCalApiWrapper::getInstance().processMsg(&msg);
        }

        int rc = poll(pollfds.data(), pollfds.size(), -1);
        if (rc < 0) {
            PLOG(ERROR) << "failed to poll display nodes";
        }
    }
}
