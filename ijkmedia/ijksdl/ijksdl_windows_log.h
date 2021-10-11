//
// Created by zacxu on 2021/10/8.
//

#ifndef PLAYERCODE_IJKDL_WINDOWS_LOG_H
#define PLAYERCODE_IJKDL_WINDOWS_LOG_H
#include <stdio.h>

void ffp_log_windows_print(int level, const char* tag, const char* fmt, ...);
void ffp_log_windows_vprint(int level, const char* tag, const char* fmt, va_list ap);

#endif //PLAYERCODE_IJKDL_WINDOWS_LOG_H
