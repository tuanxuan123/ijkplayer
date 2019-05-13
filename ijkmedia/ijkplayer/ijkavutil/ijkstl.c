/*
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

#include <stdlib.h>
#include <memory.h>
#include "ijkstl.h"

#define IjkMapDefualtSize       256


void ijk_map_realloc(IjkMap *data)
{
    if(!data)
        return;

    data->size = data->size * 2; 
    data->keys = (int64_t*)realloc(data->keys, data->size * sizeof(int64_t));
    data->values = (void**)realloc(data->values, data->size * sizeof(void*));

    memset(data->keys + data->usedIdx, 0, sizeof(int64_t) * data->size / 2);
    memset(data->values + data->usedIdx, 0, sizeof(void*) * data->size / 2);
}

IjkMap* ijk_map_create() {
    
    IjkMap *data = (IjkMap*)malloc(sizeof(IjkMap));
    data->size = IjkMapDefualtSize;
    data->usedIdx = 0;
    data->keys = (int64_t*)calloc(data->size, sizeof(int64_t));
    data->values = (void**)calloc(data->size, sizeof(void*));

    memset(data->keys, 0, sizeof(int64_t) * data->size);
    memset(data->values, 0, sizeof(void*) * data->size);
    return data;

}

void ijk_map_put(IjkMap *data, int64_t key, void *value) {

    if(!data)
        return;

    

    for(int i = 0; i < data->usedIdx; ++i)
    {
        if(data->keys[i] == key)
        {
            data->values[i] = value;
            return;
        }
    }


    if(data->usedIdx >= data->size)
        ijk_map_realloc(data);

    data->keys[data->usedIdx] = key;
    data->values[data->usedIdx] = value;
    ++data->usedIdx;
    
}

void* ijk_map_get(IjkMap *data, int64_t key) {
    
    if(!data)
        return NULL;

    for(int i = 0; i < data->usedIdx; ++i)
    {
        if(data->keys[i] == key)
        {
            return data->values[i];
        }
    }

    return NULL;
}

int ijk_map_remove(IjkMap *data, int64_t key) {

    if(!data)
        return -1;

    if(data->usedIdx <= 0)
        return 0;

    int idx = -1;
    for(int i = 0; i < data->usedIdx; ++i)
    {
        if(data->keys[i] == key)
        {
            idx = i;
            break;
        }
    }

    if(idx >= 0)
    {
        for(int j = idx; j < data->usedIdx - 1; ++j)
        {
            data->keys[j] = data->keys[j+1];
            data->values[j] = data->values[j+1];
        }

        data->keys[data->usedIdx - 1] = 0;
        data->values[data->usedIdx - 1] = NULL;
        --data->usedIdx;

    }

    return 0;
}

int ijk_map_size(IjkMap *data) {

    if(!data)
        return 0;

    return data->usedIdx;
}



void* ijk_map_index_get(IjkMap *data, int index) {

    if(!data)
        return NULL;

    if(index >= data->usedIdx)
        return NULL;

    return data->values[index];
}

void ijk_map_traversal_handle(IjkMap *data, void *parm, int(*enu)(void *parm, int64_t key, void *elem)) {

    if(!data)
        return;

    for(int i = 0; i < data->usedIdx; ++i)
        enu(parm, data->keys[i], data->values[i]);
}

int64_t ijk_map_get_min_key(IjkMap *data) {


    if(!data || data->usedIdx <= 0)
        return -1;

    int64_t minKey = data->keys[0];

    for(int i = 1; i < data->usedIdx; ++i)
    {
        if(minKey > data->keys[i])
            minKey = data->keys[i];
    }

    return minKey;
}

void ijk_map_clear(IjkMap *data) {

    if(!data || data->usedIdx <= 0)
        return;

    memset(data->keys, 0, sizeof(int64_t) * data->size);
    memset(data->values, 0, sizeof(void*) * data->size);
    data->usedIdx = 0;
}

void ijk_map_destroy(IjkMap *data) {

    if(!data)
        return;

    free(data->keys);
    free(data->values);
    free(data);

}
