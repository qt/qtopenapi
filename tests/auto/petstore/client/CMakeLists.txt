# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_openapi_client(PetStoreClient
       SPEC_FILE
              ${CMAKE_CURRENT_SOURCE_DIR}/petstore.yaml
       GENERATE_COMMON_LIBRARY
       COMMON_LIBRARY_NAME CommonPetStoreClient
)
qt_autogen_tools_initial_setup(PetStoreClient)
qt_autogen_tools_initial_setup(CommonPetStoreClient)

add_subdirectory(PetApiTest)
add_subdirectory(StoreApiTest)
add_subdirectory(UserApiTest)
