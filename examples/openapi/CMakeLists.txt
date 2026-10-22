# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Quick)
    qt_internal_add_example(chatgptclient)
    qt_internal_add_example(colorpalette)
endif()

if(TARGET Qt::Widgets AND TARGET Qt::NetworkAuth AND QT_FEATURE_process)
    qt_internal_add_example(oauth2client)
endif()
