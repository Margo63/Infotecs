set(LIBRARY_NAME logger)

add_library(${LIBRARY_NAME}_static STATIC
        logger.cpp
)

add_library(${LIBRARY_NAME}_shared SHARED
        logger.cpp
)

target_include_directories(${LIBRARY_NAME}_static PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${LIBRARY_NAME}_shared PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(${LIBRARY_NAME}_static PROPERTIES
        OUTPUT_NAME ${LIBRARY_NAME}
)

set_target_properties(${LIBRARY_NAME}_shared PROPERTIES
        OUTPUT_NAME ${LIBRARY_NAME}
        VERSION 1.0.0
        SO VERSION 1
)