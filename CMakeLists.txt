cmake_minimum_required(VERSION 3.16)
# set the project name
project(rss)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(CURL REQUIRED)
find_package(PythonLibs 3 REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories("$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

add_library(librequest STATIC src/request.cpp)
target_link_libraries(librequest PRIVATE curl)
target_link_libraries(librequest PRIVATE curlpp)

add_library(libparse STATIC src/parse.cpp)
target_link_libraries(libparse PRIVATE tinyxml2)
target_link_libraries(libparse PRIVATE mongo::bsoncxx_shared)
target_link_libraries(libparse PRIVATE ${PYTHON_LIBRARIES})

# add the executable
add_executable(${PROJECT_NAME} src/rss.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(libparse PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::bsoncxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE librequest)
target_link_libraries(${PROJECT_NAME} PRIVATE libparse)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ~/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug" AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Coverage")
    SET(BUILD_TESTING OFF CACHE BOOL "Turn off tests" FORCE)
    message(STATUS "Not building tests, tests are supported in Debug build only.")
endif ()
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()

