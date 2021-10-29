cmake_minimum_required(VERSION 3.21)
project(horntail)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} horntail/main.cpp horntail/link.controller.cpp horntail/link.controller.h horntail/logging.cpp horntail/logging.h)
add_executable(horntail_log horntail_log/main.cpp)

### drogon ###

set(BUILD_EXAMPLES off)
add_subdirectory(third_party/drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)

##############
### spdlog ###

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=0) # set this to 0 so none of the log statements get compiled out
add_subdirectory(third_party/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
target_link_libraries(horntail_log PRIVATE spdlog)

##############
### json #####

target_include_directories(horntail_log PRIVATE third_party/json/include)

##############