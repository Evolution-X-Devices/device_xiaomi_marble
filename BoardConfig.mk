#
# Copyright (C) 2022-2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Security patch level
VENDOR_SECURITY_PATCH := 2026-02-01

# Inherit from xiaomi sm8450-common
include device/xiaomi/sm8450-common/BoardConfigCommon.mk

# Inherit from the proprietary version
include vendor/xiaomi/marble/BoardConfigVendor.mk

# Inherit from proprietary files for miuicamera
-include device/xiaomi/miuicamera-marble/BoardConfig.mk

DEVICE_PATH := device/xiaomi/marble

# Assert
TARGET_OTA_ASSERT_DEVICE := marble,marblein

# Init
TARGET_RECOVERY_DEVICE_MODULES ?= init_xiaomi_marble

# Properties
TARGET_ODM_PROP += $(DEVICE_PATH)/properties/odm.prop
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/properties/system.prop
TARGET_VENDOR_PROP += $(DEVICE_PATH)/properties/vendor.prop

# The SFPS display IDs differ per kernel (DRM port = 0x80 | encoder index):
# 6.6 creates the writeback encoder first (DSI = 2), 5.10 creates DSI first (DSI = 1).
# Only the pair matching the kernel is set, as all four exceed the prop size limit.
ifeq ($(TARGET_KERNEL_VERSION),6.6)
TARGET_VENDOR_PROP += $(DEVICE_PATH)/properties/vendor_sfps_6.6.prop
else
TARGET_VENDOR_PROP += $(DEVICE_PATH)/properties/vendor_sfps_5.10.prop
endif

# Screen density
TARGET_SCREEN_DENSITY := 440
