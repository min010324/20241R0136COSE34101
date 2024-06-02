cmake_minimum_required(VERSION 3.28)
project(20241R0136COSE4101 C)

set(CMAKE_C_STANDARD 11)
set(SOURCES
    main.c
    process.c
    scheduler.c
)

add_executable(CPUScheduler ${SOURCES})
