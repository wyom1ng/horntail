cmake_minimum_required(VERSION 3.0)
project(horntail)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        horntail/link.controller.cpp
        horntail/link.controller.h
        horntail/logging.cpp
        horntail/logging.h
        horntail/config.plugin.cpp
        horntail/config.plugin.h
        horntail/database.plugin.cpp
        horntail/database.plugin.h
        horntail/credential.filter.cpp
        horntail/credential.filter.h)

add_executable(${PROJECT_NAME} horntail/main.cpp ${SOURCES})

add_executable(${PROJECT_NAME}_log horntail_log/main.cpp)

add_executable(${PROJECT_NAME}_test horntail_test/main.cpp ${SOURCES}
        horntail_test/guard.cpp
        horntail_test/guard.h
        horntail_test/test/link.cpp
        horntail_test/test/spec.cpp)

### drogon ###

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
add_subdirectory(third_party/drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)

target_link_libraries(${PROJECT_NAME}_test PRIVATE drogon)
ParseAndAddDrogonTests(${PROJECT_NAME}_test)

##############
### spdlog ###

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=0) # set this to 0 so none of the log statements get compiled out
add_subdirectory(third_party/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
target_link_libraries(${PROJECT_NAME}_log PRIVATE spdlog)
target_link_libraries(${PROJECT_NAME}_test PRIVATE spdlog)

##############
### json #####

target_include_directories(${PROJECT_NAME}_log PRIVATE third_party/json/include)
target_include_directories(${PROJECT_NAME}_test PRIVATE third_party/json/include)

##############
### csprng ###

target_include_directories(${PROJECT_NAME} PRIVATE third_party/CSPRNG/source)
target_sources(${PROJECT_NAME} PRIVATE third_party/CSPRNG/source/csprng.cpp)
target_include_directories(${PROJECT_NAME}_test PRIVATE third_party/CSPRNG/source)
target_sources(${PROJECT_NAME}_test PRIVATE third_party/CSPRNG/source/csprng.cpp)

##############
### nanoid ###

add_subdirectory(third_party/nanoid_cpp)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/nanoid_cpp/inc)
target_link_libraries(${PROJECT_NAME} PRIVATE nanoid)
target_include_directories(${PROJECT_NAME}_test PRIVATE third_party/nanoid_cpp/inc)
target_link_libraries(${PROJECT_NAME}_test PRIVATE nanoid)

##############
### date #####

target_include_directories(${PROJECT_NAME}_test PRIVATE third_party/date/include)

##############