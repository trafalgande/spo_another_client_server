cmake_minimum_required(VERSION 3.16)
project(spo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pthread")

find_package (bson-1.0 1.7 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(JSONC REQUIRED IMPORTED_TARGET json-c)

add_executable(client client.c cmd_parser.h util.c mt.h)
add_executable(server server.c cmd_parser.c api.c util.c mt.h)
target_link_libraries(client PRIVATE PkgConfig::JSONC)
target_link_libraries(server PRIVATE PkgConfig::JSONC)
target_link_libraries(server PRIVATE mongo::bson_shared)
target_link_libraries(client PRIVATE mongo::bson_shared)