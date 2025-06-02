/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "SensorNotifier.h"

class RawLightNotifier : public SensorNotifier {
  public:
    RawLightNotifier(sp<ISensorManager> manager);
    ~RawLightNotifier();

  protected:
    void notify();

    bool isEnable;
};
