#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define CLEAR_SCREEN() system("cls")
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR_SCREEN() system("clear")
#endif

void trim_newline(char* str);
void format_time(char* buffer, time_t time);
void create_directory(const char* path);
int file_exists(const char* path);

#endif