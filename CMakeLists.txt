CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(SKIPLIST)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BUILD_TYPE "Debug")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设定源码位置.cpp
SET(SRC_FILE ${PROJECT_SOURCE_DIR}/src/main.cpp)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src SRC_DIR)

# 生成可执行文件
ADD_EXECUTABLE(skiplist ${SRC_FILE} ${SRC_DIR})