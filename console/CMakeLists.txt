add_executable(app main.cpp)

target_link_libraries(app
        logger_static
)

target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/liblogger/include
)