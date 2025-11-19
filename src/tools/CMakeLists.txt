# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_openapi_generator)
    add_subdirectory(qtopenapi-generator)
    if(QT_FEATURE_openapi_common_library)
        add_subdirectory(qtopenapicommon)
    endif()
endif()
