# ============================================================
# MODULE: CMakeLists.txt
# PURPOSE: Build configuration for Sri Mart.
# WHY: CMake configures how source files are compiled and linked.
# INPUT: Source files (.cpp), dependencies (Drogon, libpq).
# OUTPUT: Executable named sri_mart.
# ARCHITECTURE: Build configuration
# ============================================================

cmake_minimum_required(VERSION 3.14)
project(sri_mart VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

add_executable(sri_mart
    sri.cpp
    controllers/ProductController.cc
    controllers/AuthController.cc
    services/ProductService.cc
    services/AuthService.cc
)

target_link_libraries(sri_mart PRIVATE
    Drogon::Drogon
    pq
)

target_include_directories(sri_mart PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/services
)

add_custom_command(TARGET sri_mart POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/config $<TARGET_FILE_DIR:sri_mart>/config
    COMMENT "Copying config files to build directory"
)
