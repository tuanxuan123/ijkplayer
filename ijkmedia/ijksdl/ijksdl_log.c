/*****************************************************************************
 * ijksdl_error.c
 *****************************************************************************
 *
 * Copyright (c) 2013 Bilibili
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ijksdl_log.h"

#ifdef _WIN32

#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <stdarg.h>
#include <time.h>


#define LOG_PATH            "C:/Users/%s/AppData/Local/PVideoPlayer/"
#define LOG_NAME            "PVideo.log"
#define LOG_OLD_NAME        "PVideo_old.log"


#define MAX_LOG_SIZE        8 * 1024 * 1024
#define USER_NAME_SIZE      128
#define LOG_PATH_SIZE       1024

static int s_log_inited = 0;
static const char* s_log_type[] = {"[UNKNOWN] ", "[DEFAULT] ", "[VERBOSE] ", "[DEBUG] ", "[INFO] ", "[WARN] ", "[ERROR] ", "[FATAL] ", "[SILENT] "};
static char s_log_path[LOG_PATH_SIZE] = { 0 };
static char s_log_path_name[LOG_PATH_SIZE] = { 0 };
static FILE* s_file = NULL;


#endif // __WIN32


static int s_log_level = 0;

void pixvideo_set_log_level(int level)
{
    s_log_level = level;
}


int pixvideo_get_log_level()
{
	return s_log_level;
}

#ifdef _WIN32


static int log_mkdirs(char* path) 
{

    size_t len = strlen(path);
    int ret = 0;

    for (size_t i = 0; i <= len; i++)
    {
        if (path[i] == '/' || path[i] == '\0') 
        {
            path[i] = '\0';
            if (_access(path, 0) != 0) 
            {
                ret = _mkdir(path);

            }
            path[i] = '/';

            if (ret != 0)
                break;
        }

    }

    return ret;

}



static void get_log_path() 
{
    wchar_t wc_user_name[USER_NAME_SIZE] = { 0 };
    int len = USER_NAME_SIZE;
    GetUserName(wc_user_name, &len);
    char user_name[USER_NAME_SIZE] = { 0 };
    wcstombs(user_name, wc_user_name, USER_NAME_SIZE);
    sprintf(s_log_path, LOG_PATH, user_name);
}



void pixvideo_ffp_log_windows_print(int level, const char* tag, const char* msg)
{
    if (!s_log_inited)
    {
        get_log_path();
        log_mkdirs(s_log_path);

        memcpy(s_log_path_name, s_log_path, strlen(s_log_path));

        strcat(s_log_path_name, LOG_NAME);

        

        s_file = fopen(s_log_path_name, "a+");
        s_log_inited = 1;

        if (s_file)
        {
            fseek(s_file, 0, SEEK_END);
            long size = ftell(s_file);
            if (size >= MAX_LOG_SIZE)
            {
                fclose(s_file);

                size_t path_len = strlen(s_log_path);
                strcat(s_log_path, LOG_OLD_NAME);

                remove(s_log_path);
                if (rename(s_log_path_name, s_log_path) != 0)
                {
                    printf("rename failed !!");
                }

                s_log_path[path_len] = '\0';

                s_file = fopen(s_log_path_name, "a+");

            }
            
            fputs("\n===============================================================================\n", s_file);
        }
        else
        {
            return;
        }

    }

    char log_content[LOG_PATH_SIZE] = {0};
	size_t len = strlen(msg);

    if (!s_file || len > LOG_PATH_SIZE)
        return;


	if (msg[len - 1] == '\n')
    {
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);

        sprintf(log_content, " %d-%d-%d-%d:%d:%d", wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond);
        strcat(log_content, s_log_type[level]);
    }
    
    strcat(log_content, msg);
    fputs(log_content, s_file);
    fflush(s_file);
}

#endif // _WIN32

