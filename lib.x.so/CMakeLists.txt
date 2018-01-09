cmake_minimum_required(VERSION 2.8)
project(devent)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall  -std=c++11 -fpermissive -fPIC")


SET(DEV_ENV "linux_x64")


# 头文件搜索路径
INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/util/inc
        ${PROJECT_SOURCE_DIR}/test
        ${PROJECT_SOURCE_DIR}/temp
)


# 动态库/静态库搜索路径
LINK_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/util/lib
)

# mac平台
IF(${DEV_ENV} STREQUAL "mac")
    INCLUDE_DIRECTORIES(
            ${INCLUDE_DIRECTORIES}
            /usr/local/include/
    )

    LINK_DIRECTORIES(
            ${LINK_DIRECTORIES}
            /usr/local/lib
    )
ENDIF()

# linux平台
IF(${DEV_ENV} STREQUAL "linux_x64")
    INCLUDE_DIRECTORIES(
            ${INCLUDE_DIRECTORIES}
            /usr/local/include
    )

    LINK_DIRECTORIES(
            ${LINK_DIRECTORIES}
            /usr/local/lib
    )
ENDIF()

set(SOURCE_FILES test/boost_test.cpp)
add_executable(boost_test ${SOURCE_FILES})


set(SOURCE_FILES test/except_test.cpp)
add_executable(except_test ${SOURCE_FILES})
target_link_libraries(except_test
    base
)

set(SOURCE_FILES test/gettid_test.cpp)
add_executable(gettid_test ${SOURCE_FILES})
target_link_libraries(gettid_test
        pthread
)