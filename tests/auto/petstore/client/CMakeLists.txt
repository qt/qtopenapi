# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(cpp-qt-petstore)

add_subdirectory(client)
add_subdirectory(PetApiTest)
add_subdirectory(StoreApiTest)
add_subdirectory(UserApiTest)
