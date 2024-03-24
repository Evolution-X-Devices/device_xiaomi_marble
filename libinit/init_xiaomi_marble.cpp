/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libinit_dalvik_heap.h>
#include <libinit_variant.h>
#include <libinit_utils.h>

#include "vendor_init.h"

#define FINGERPRINT_GL "POCO/marble_global/marble:12/SKQ1.230401.001/V816.0.3.0.UMRMIXM:user/release-keys"
#define FINGERPRINT_CN "Redmi/marble/marble:12/SKQ1.230401.001/V816.0.2.0.UMRCNXM:user/release-keys"
#define FINGERPRINT_IN "POCO/marblein/marblein:12/SKQ1.230401.001/V816.0.5.0.UMRINXM:user/release-keys"

static const variant_info_t marble_global_info = {
    .hwc_value = "GL",
    .sku_value = "",

    .brand = "POCO",
    .device = "marble",
    .marketname = "POCO F5",
    .model = "23049PCD8G",
    .mod_device = "marble_global",
    .build_fingerprint = FINGERPRINT_GL,
};

static const variant_info_t marblein_info = {
    .hwc_value = "IN",
    .sku_value = "",

    .brand = "POCO",
    .device = "marblein",
    .marketname = "POCO F5",
    .model = "23049PCD8I",
//  .mod_device = "marble_in_global",
    .mod_device = "marble_global", // Fixed miuicamera crash
    .build_fingerprint = FINGERPRINT_IN,
};

static const variant_info_t marble_info = {
    .hwc_value = "CN",
    .sku_value = "",

    .brand = "Redmi",
    .device = "marble",
    .marketname = "Redmi Note 12 Turbo",
    .model = "23049RAD8C",
//  .mod_device = "marble",
    .mod_device = "marble_global", // Fixed miuicamera crash
    .build_fingerprint = FINGERPRINT_CN,
};

static const std::vector<variant_info_t> variants = {
    marble_global_info,
    marblein_info,
    marble_info,
};

void vendor_load_properties() {
    set_dalvik_heap();
    search_variant(variants);

    // SafetyNet workaround
    property_override("ro.boot.verifiedbootstate", "green");
}
