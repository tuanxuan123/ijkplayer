//
// Created by zhazhachen on 2023/9/20.
//
#include "ijkcacheutil.h"
#include "ijkavutil/ijkstl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif // _WIN32

extern IjkURLProtocol ijkio_cache_protocol;
extern IjkURLProtocol ijkio_ffio_protocol;

#define CONFIG_MAX_LINE 1024
#define CACHE_READ_SIZE 1024 * 32

static int ijkio_manager_context_alloc(IjkIOManagerContext **ph)
{
    return ijkio_manager_create(ph, NULL);
}

IjkIOManagerContext* ijkcache_init_context(const char* url, const char* cache_file_path, const char* cache_map_path) {
    IjkIOManagerContext *h = NULL;
    ijkio_manager_context_alloc(&h);

    IjkAVDictionary *options = NULL;
    ijk_av_dict_set(&options, "cache_file_path", cache_file_path, 0);
    ijk_av_dict_set(&options, "cache_map_path", cache_map_path, 0);
    ijk_av_dict_set(&options, "parse_cache_map", "1", 0);

    ijkio_manager_io_open(h, url, 0, &options);

    return h;
}

void ijkcache_destroy_context(IjkIOManagerContext *h) {
    ijkio_manager_destroy(h);
}

int ijkcache_is_cache_complete(IjkIOManagerContext *h) {
    int tree_index = 0;
    IjkCacheTreeInfo *cur_tree_info = ijk_map_get(h->ijkio_app_ctx->cache_info_map, tree_index);
    if (cur_tree_info == NULL) return 0;

    int is_cache_complete = (cur_tree_info->physical_size == cur_tree_info->file_size ? 1 : 0);
    return is_cache_complete;
}

int ijkcache_save_to_mp4(IjkIOManagerContext *h, const char* cache_file_path, const char* output_path) {
    int is_cache_complete = ijkcache_is_cache_complete(h);
    if (is_cache_complete) {
        int tree_index = 0;
        IjkCacheTreeInfo* cur_tree_info = ijk_map_get(h->ijkio_app_ctx->cache_info_map, tree_index);

        if (cur_tree_info)
        {
            //if mp4 exist and valid
            struct stat file_info;
            if (stat(output_path, &file_info) == 0) {
                off_t file_size = file_info.st_size;
                if (file_size == cur_tree_info->file_size) {
                    return 1;
                }
            }

            //create mp4 file
            char *tmp_cache = malloc(CACHE_READ_SIZE);
            int read_bytes = 0;
            FILE* fp = fopen(output_path, "wb");
            while (tmp_cache && fp && read_bytes < cur_tree_info->file_size) {
                int tmp_read_size = ijkio_manager_io_read(h, tmp_cache, CACHE_READ_SIZE);
                if (tmp_read_size > 0) {
                    read_bytes += tmp_read_size;
                    int write_count = fwrite(tmp_cache, tmp_read_size, 1, fp);
                    if (1 != write_count) {
                        break;
                    }
                }
                else break;
            }

            if (tmp_cache) {
                free(tmp_cache);
                tmp_cache = NULL;
            }

            if (fp) {
                fclose(fp);
            }
            return read_bytes > 0 ? 1 : 0;
        }
    }
    return 0;
}
