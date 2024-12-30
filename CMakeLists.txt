# CMake boilerplate
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(modbus_master)

# Specifica la tabella di partizioni custom
set(PARTITION_TABLE_PATH ${CMAKE_SOURCE_DIR}/partitions.csv)

# Impostazione per utilizzare la partizione custom durante la compilazione
set(IDF_EXTRA_PARTITION_TABLE ${PARTITION_TABLE_PATH})
