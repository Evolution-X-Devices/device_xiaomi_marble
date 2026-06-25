#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/xiaomi/sm8450-common',
    'hardware/qcom-caf/sm8450',
    'hardware/google/interfaces',
    'hardware/google/pixel',
    'hardware/google/pixel/pixelstats',
    'hardware/google/pixel/power-libperfmgr',
    'hardware/lineage/interfaces/power-libperfmgr',
    'hardware/qcom-caf/common/libqti-perfd-client',
    'hardware/xiaomi',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/xiaomi/sm8450-common',
]

blob_fixups: blob_fixups_user_type = {
    (
        'vendor/etc/camera/marble_enhance_motiontuning.xml',
        'vendor/etc/camera/marble_motiontuning.xml',
    ): blob_fixup().regex_replace('xml=version', 'xml version'),
    'vendor/etc/camera/pureView_parameter.xml': blob_fixup().regex_replace(
        r'=([0-9]+)>', r'="\1">'
    ),
    (
        'vendor/bin/hw/vendor.qti.camera.provider@2.7-service_64',
        'vendor/lib64/camera/plugins/com.xiaomi.plugin.mialgosnsc.so',
        'vendor/lib64/camx.device@3.4-ext-impl.so',
        'vendor/lib64/camx.device@3.5-ext-impl.so',
        'vendor/lib64/camx.device@3.6-ext-impl.so',
        'vendor/lib64/camx.provider@2.4-external.so',
        'vendor/lib64/camx.provider@2.4-impl.so',
        'vendor/lib64/camx.provider@2.4-legacy.so',
        'vendor/lib64/camx.provider@2.5-external.so',
        'vendor/lib64/camx.provider@2.5-legacy.so',
        'vendor/lib64/camx.provider@2.6-legacy.so',
        'vendor/lib64/camx.provider@2.7-legacy.so',
        'vendor/lib64/com.qti.feature2.anchorsync.so',
    ): blob_fixup().replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),
    (
        'vendor/lib64/hw/com.qti.chi.override.so',
        'vendor/lib64/libcamxcommonutils.so',
        'vendor/lib64/libmialgoengine.so',
    ): blob_fixup().add_needed('libprocessgroup_shim.so'),
    'vendor/lib64/libcamximageformatutils.so': blob_fixup().replace_needed(
        'vendor.qti.hardware.display.config-V2-ndk_platform.so',
        'vendor.qti.hardware.display.config-V2-ndk.so',
    ),
    'vendor/lib64/libgf_hal.so': blob_fixup().binary_regex_replace(
        rb'\[%s\] openat: %s xiaomi_sysfs_fd,failed:\[fingerdown\]',
        b'[%s] openat: xiaomi_sysfs_fd,failed:[fingerdown]\x00\x00\x00',
    ),
    (
        'vendor/lib64/libTrueSight.so',
        'vendor/lib64/libalLDC.so',
        'vendor/lib64/libalhLDC.so',
    ): blob_fixup()
    .clear_symbol_version('AHardwareBuffer_allocate')
    .clear_symbol_version('AHardwareBuffer_describe')
    .clear_symbol_version('AHardwareBuffer_lock')
    .clear_symbol_version('AHardwareBuffer_lockPlanes')
    .clear_symbol_version('AHardwareBuffer_release')
    .clear_symbol_version('AHardwareBuffer_unlock'),
}

module = ExtractUtilsModule(
    'marble',
    'xiaomi',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    add_firmware_proprietary_file=True,
)

if __name__ == '__main__':
    utils = ExtractUtils.device_with_common(
        module, 'sm8450-common', module.vendor
    )
    utils.run()
