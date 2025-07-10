//
// Created by zhazhachen on 2023/9/20.
//

#ifndef VIDEO_IJKIOCACHEUTIL_H
#define VIDEO_IJKIOCACHEUTIL_H
#include "ijkiomanager.h"

IjkIOManagerContext* ijkcache_init_context(const char* url, const char* cache_file_path, const char* cache_map_path);
void ijkcache_destroy_context(IjkIOManagerContext *h);
int ijkcache_is_cache_complete(IjkIOManagerContext *h);
int ijkcache_save_to_mp4(IjkIOManagerContext *h, const char* cache_file_path, const char* output_path);

#endif //VIDEO_IJKIOCACHEUTIL_H
