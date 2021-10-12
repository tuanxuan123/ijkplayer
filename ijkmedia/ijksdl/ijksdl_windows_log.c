#include "ijksdl_windows_log.h"
#include "zlog.h"
#include <stdio.h>
#include <Windows.h>

#define LOG_BUF_SIZE	1024

#define IJK_LOG_UNKNOWN     0
#define IJK_LOG_DEFAULT     1

#define IJK_LOG_VERBOSE     2
#define IJK_LOG_DEBUG       3
#define IJK_LOG_INFO        4
#define IJK_LOG_WARN        5
#define IJK_LOG_ERROR       6
#define IJK_LOG_FATAL       7
#define IJK_LOG_SILENT      8

static int is_zlog_init = 1;
static char log_path[128] = "";

static int log_mkdirs(char* muldir);
static void log_getpath();
void ffp_log_windows_print(int level, const char *tag, const char *fmt, ...)
{
    
    int rc;
    zlog_category_t* c;
    if (is_zlog_init) {
        log_getpath();
        rc = zlog_init("..\\..\\ijkmedia\\ijksdl\\ijksdl_windows_zlog.conf");
        if (access(log_path, 0)) {
            log_mkdirs(log_path);
        }
        if (rc) {
            printf("init failed\n");
            return;
        }
        is_zlog_init = 0;
    }
    zlog_put_mdc("path", log_path);
    c = zlog_get_category(tag);
    if (!c) {
        zlog_fini();
        return;
    }

    va_list ap;
    char log_buffer[LOG_BUF_SIZE] = {0};

    va_start(ap, fmt);
    vsnprintf(log_buffer, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    switch (level) {
        case IJK_LOG_UNKNOWN:
        case IJK_LOG_DEFAULT:
        case IJK_LOG_VERBOSE:
        case IJK_LOG_DEBUG:
            zlog_debug(c, log_buffer);
            break;
        case IJK_LOG_INFO:
            zlog_info(c, log_buffer);
            break;
        case IJK_LOG_WARN:
            zlog_warn(c, log_buffer);
            break;
        
        case IJK_LOG_ERROR:
            zlog_error(c, log_buffer);
            break;
        case IJK_LOG_FATAL:
        case IJK_LOG_SILENT:
            zlog_fatal(c, log_buffer);
            break;
        default:
            break;
    }
    //zlog_fini();
}

void ffp_log_windows_vprint(int level, const char *tag, const char *fmt, va_list ap)
{
    int rc;
    zlog_category_t* c;
    if (is_zlog_init) {
        log_getpath();
        rc = zlog_init("..\\..\\ijkmedia\\ijksdl\\ijksdl_windows_zlog.conf");
        if (access(log_path, 0)) {
            log_mkdirs(log_path);
        }
        if (rc) {
            printf("init failed\n");
            return;
        }
        is_zlog_init = 0;
    }
    zlog_put_mdc("path", log_path);
    c = zlog_get_category(tag);
    if (!c) {
        zlog_fini();
        return;
    }

    char log_buffer[LOG_BUF_SIZE] = {0};

    vsnprintf(log_buffer, LOG_BUF_SIZE, fmt, ap);

    switch (level) {
        case IJK_LOG_UNKNOWN:
        case IJK_LOG_DEFAULT:
        case IJK_LOG_VERBOSE:
        case IJK_LOG_DEBUG:
            zlog_debug(c, log_buffer);
            break;
        case IJK_LOG_INFO:
            zlog_info(c, log_buffer);
            break;
        case IJK_LOG_WARN:
            zlog_warn(c, log_buffer);
            break;

        case IJK_LOG_ERROR:
            zlog_error(c, log_buffer);
            break;
        case IJK_LOG_FATAL:
        case IJK_LOG_SILENT:
            zlog_fatal(c, log_buffer);
            break;
        default:
            break;
    }
    //zlog_fini();
}
static void log_getpath() {
    wchar_t szBuffer[100];
    int len = 100;
    GetUserName(szBuffer, &len);
    char szbu[100];
    wcstombs(szbu, szBuffer, 100);
    strcat(log_path, "C:\\Users\\");
    strcat(log_path, szbu);
    strcat(log_path, "\\AppData\\Local\\PVideoPlayer");
}

//zlog 日志生成路径不能新建文件夹，需要自行生成
static int log_mkdirs(char* muldir)
{
    int i, len;
    wchar_t str[512];
    strncpy(str, muldir, strlen(muldir) + 1);
    len = strlen(str);
    int flag = 0;
    for (i = 0; i <= len; i++)
    {
        if (str[i] == '/' || str[i] == '\0')
        {
            str[i] = '\0';
            if (access(str, 0) != 0)
            {
                int ret = mkdir(str, 0x777);
                flag = -1;
            }
            str[i] = '/';
        }

    }
    return flag;
}