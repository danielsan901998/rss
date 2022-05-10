set(TEST_SOURCES 
    ../src/parse.cpp
    ../src/request.cpp
    ../test/test.cpp
    )
add_executable(test_rss ${TEST_SOURCES})
target_include_directories(test_rss PRIVATE "../include/")
target_link_libraries(test_rss PRIVATE mongo::bsoncxx_shared)
target_link_libraries(test_rss PRIVATE mongo::mongocxx_shared)
target_link_libraries(test_rss PRIVATE curl)
target_link_libraries(test_rss PRIVATE curlpp)
target_link_libraries(test_rss PRIVATE tinyxml2)
add_test(
    NAME test_rss
    COMMAND $<TARGET_FILE:test_rss>
    )
