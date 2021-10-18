#include "ijksdl_windows_log.h"
#include "zlog.h"
#include <stdio.h>
#include <Windows.h>
#include <sys/stat.h>
#include<io.h>
#include <time.h>
#define LOG_BUF_SIZE	1024

#define LIMIT_NUM_OF_LOG 3

#define IJK_LOG_UNKNOWN     0
#define IJK_LOG_DEFAULT     1

#define IJK_LOG_VERBOSE     2
#define IJK_LOG_DEBUG       3
#define IJK_LOG_INFO        4
#define IJK_LOG_WARN        5
#define IJK_LOG_ERROR       6
#define IJK_LOG_FATAL       7
#define IJK_LOG_SILENT      8

//限制log文件大小为16MB
const long max_log_size = 16*1024*1024;

static int is_zlog_init = 1;
static int is_memory_limit = 0;
static char log_path[128] = "";
static char log_filename[128] = "";

static int log_mkdirs(char* muldir);
static void log_getpath();
static void check_and_delete_history_log();
static int check_memory();


void ffp_log_windows_print(int level, const char* tag, const char* fmt, ...) {
    int rc;
    zlog_category_t* c;
    //初始化:log文件地址，文件名，zlog初始化，删除历史多余文件。
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
        check_and_delete_history_log();
    }

    //判断log大小是否超限
    if (is_memory_limit) {
        return;
    }
    else if (check_memory()) {
        is_memory_limit = 1;
        return;
    }

    zlog_put_mdc("path", log_path);
    zlog_put_mdc("filename", log_filename);
    c = zlog_get_category(tag);
    if (!c) {
        zlog_fini();
        return;
    }

    va_list ap;
    char log_buffer[LOG_BUF_SIZE] = { 0 };

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

void ffp_log_windows_vprint(int level, const char* tag, const char* fmt, va_list ap)
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
        check_and_delete_history_log();
    }


    if (is_memory_limit) {
        return;
    }
    else if (check_memory()) {
        is_memory_limit = 1;
        return;
    }

    zlog_put_mdc("path", log_path);
    zlog_put_mdc("filename", log_filename);
    c = zlog_get_category(tag);
    if (!c) {
        zlog_fini();
        return;
    }

    char log_buffer[LOG_BUF_SIZE] = { 0 };

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
    wchar_t wc_username[100];
    int len = 100;
    GetUserName(wc_username, &len);
    char username[100];
    wcstombs(username, wc_username, 100);
    sprintf(log_path, "C:\\Users\\%s\\AppData\\Local\\PVideoPlayer", username);
}

//zlog 日志生成路径不能新建文件夹，需要自行生成
static int log_mkdirs(char* muldir) {
    int i, len;
    wchar_t str[512];
    strncpy(str, muldir, strlen(muldir) + 1);
    len = strlen(str);
    int flag = 0;
    for (i = 0; i <= len; i++) {
        if (str[i] == '/' || str[i] == '\0') {
            str[i] = '\0';
            if (access(str, 0) != 0) {
                int ret = mkdir(str, 0x777);
                flag = -1;
            }
            str[i] = '/';
        }

    }
    return flag;
}

//只保留最新的三个日志文件。检查当前是否超过两个，若超过则删除最老的文件。
#define NUM_OF_FILES 10
static char files[NUM_OF_FILES][256];
static void check_and_delete_history_log() {
    int num_of_files = getFiles(log_path, files);
    if (num_of_files < LIMIT_NUM_OF_LOG) {
        return;
    }
    time_t min_time = MAXINT64;
    int flag = 0;
    for (int i = 0; i < num_of_files; i++) {
        struct stat st;
        stat(files[i], &st);
        if (st.st_mtime < min_time) {
            min_time = st.st_mtime;
            flag = i;
        }
    }

    char command[300];
    sprintf(command, "del /q  %s", files[flag]);
    system(command);
}

//检查日志文件大小是否超过了限制。max_log_size 16MB
static int check_memory() {
    static c_init = 0;
    static char path_filename[256];
    if (!c_init) {
        c_init = 1;
        sprintf(path_filename, "%s\\%s.log", log_path, log_filename);
    }
    struct stat st;
    stat(path_filename, &st);
    if (st.st_size >= max_log_size) {
        return 1;
    }
    return 0;
}


static int getFiles(char* path) {
    int num_of_img = 0;
    long long hFile = 0;
    struct _finddata_t fileinfo;
    char p[256] = { 0 };
    strcpy(p, path);
    strcat(p, "\\*");
    printf("%s\n", p);

    if ((hFile = _findfirst(p, &fileinfo)) != -1) {
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    continue;
            }
            else {
                strcpy(files[num_of_img], path);
                strcat(files[num_of_img], "\\");
                strcat(files[num_of_img++], fileinfo.name);
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }

    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(log_filename, "IJKMEDIA_%02d%02d_%d", timeinfo->tm_mon + 1, timeinfo->tm_mday, getpid());

    return num_of_img;
}