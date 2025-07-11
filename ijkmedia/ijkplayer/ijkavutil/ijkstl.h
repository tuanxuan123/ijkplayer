/*
 * Copyright (c) 2016 Bilibili
 * Copyright (c) 2016 Raymond Zheng <raymondzheng1412@gmail.com>
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
#ifndef IJKAVUTIL_IJKSTL_H
#define IJKAVUTIL_IJKSTL_H

#include <stdint.h>

 typedef struct IjkMap
 {
    int         size;
    int64_t     *keys;
    void        **values;
    int         usedIdx;

 }IjkMap;

IjkMap* ijk_map_create();
void ijk_map_put(IjkMap *data, int64_t key, void *value);
void* ijk_map_get(IjkMap *data, int64_t key);
int ijk_map_remove(IjkMap *data, int64_t key);
int ijk_map_size(IjkMap *data);
void* ijk_map_index_get(IjkMap *data, int index);
void ijk_map_traversal_handle(IjkMap *data, void *parm, int(*enu)(void *parm, int64_t key, void *elem));
int64_t ijk_map_get_min_key(IjkMap *data);
void ijk_map_clear(IjkMap *data);
void ijk_map_destroy(IjkMap *data);

#endif /* IJKAVUTIL_IJKSTL_H */
