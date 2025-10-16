/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libinit_dalvik_heap.h>
#include <libinit_variant.h>
#include <libinit_utils.h>

#include "vendor_init.h"

static const variant_info_t marble_global_info = {
    .hwc_value = "GL",
    .sku_value = "",

    .brand = "POCO",
    .device = "marble",
    .marketname = "POCO F5",
    .model = "23049PCD8G",
    .mod_device = "marble_global",
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
