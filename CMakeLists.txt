cmake_minimum_required(VERSION 3.20)
project(pc_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pc_sim
    main-simpler.cpp
    storage.h
)

add_custom_command(TARGET pc_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/program.asm"
        "$<TARGET_FILE_DIR:pc_sim>/program.asm"
)
