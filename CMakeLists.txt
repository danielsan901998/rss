cmake_minimum_required(VERSION 3.12)

# set the project name
project(rss)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(CURL REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})


#sources files
set(SOURCES 
    src/parse.cpp
    src/request.cpp
    src/rss.cpp
    )
# add the executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::bsoncxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE curl)
target_link_libraries(${PROJECT_NAME} PRIVATE curlpp)
target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    set(TEST_SOURCES 
        src/parse.cpp
        src/request.cpp
        test/test.cpp
        )
    add_executable(test_rss ${TEST_SOURCES})
    target_include_directories(test_rss PRIVATE "include/")
    target_link_libraries(test_rss PRIVATE mongo::bsoncxx_shared)
    target_link_libraries(test_rss PRIVATE mongo::mongocxx_shared)
    target_link_libraries(test_rss PRIVATE curl)
    target_link_libraries(test_rss PRIVATE curlpp)
    target_link_libraries(test_rss PRIVATE tinyxml2)
    add_test(
        NAME test_rss
        COMMAND $<TARGET_FILE:test_rss>
        )
endif()
