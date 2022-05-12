project(test_rss)
add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::bsoncxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE librequest)
target_link_libraries(${PROJECT_NAME} PRIVATE libparse)
add_test(
    NAME ${PROJECT_NAME}
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
    )
