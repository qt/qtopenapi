# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Check for both existence of the built Qt generator plugin and the
# upstream generator.
# It's possible to have java + maven available, so the qt plugin will be built,
# but for the upstream generator to be missing, in which case we can't build the tests.
if(TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar
        AND TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
    add_subdirectory(colorpalette)
    add_subdirectory(compression)
    add_subdirectory(linking_several_clients)
    if(TARGET Qt::Gui)
        add_subdirectory(mediatype)
    endif()
    add_subdirectory(openapi2.0)
    add_subdirectory(operation-parameters)
    add_subdirectory(petstore)
    add_subdirectory(responses)
endif()
