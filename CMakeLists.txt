cmake_minimum_required(VERSION 4.0)
project(project2 C)

set(CMAKE_C_STANDARD 11)

# 创建可执行文件（去掉 STATIC）
add_executable(project2
        library.c
        project1.origin.c
        gamestate.h
        undostack.h
        file_io.h
        uimanager.h
        game_logic.h
        utils.h
        main.c
        undostack.c
        game_logic.c
        file_io.c
        uimanager.c
        utils.c)