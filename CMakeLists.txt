cmake_minimum_required(VERSION 3.16)
project(spo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pthread")

find_package (bson-1.0 1.7 REQUIRED)

add_executable(client client.c cmd_parser.c mt.h)
add_executable(server server.c cmd_parser.c api.c util.c mt.h)
target_link_libraries(client PRIVATE json-c)
target_link_libraries(server PRIVATE json-c)
target_link_libraries(server PRIVATE mongo::bson_shared)