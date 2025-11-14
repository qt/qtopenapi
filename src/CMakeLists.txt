# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# We need to evaluate the features early, to decide whether to build and install the
# OpenAPI module at all. Otherwise if not all dependencies are met and the generator is not built
# in the tools subdir, we still end up installing the macros and extra targets files, which is
# undesirable.
add_subdirectory(global)

if(QT_FEATURE_openapi_generator)
    add_subdirectory(openapi)
endif()

add_subdirectory(tools)
