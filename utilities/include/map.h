/**
 * Copyright (c) 2021 Maarten Thomassen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Map
 */
typedef struct map_s* map;

typedef struct map_iter_s* map_iter;

/**
 * @brief Create a new Map
 * 
 * @param[out] m Pointer where to store the new map
 */
void map_new(map* m);

/**
 * @brief Copies the map and all its items
 * 
 * @param[in] m The map to be copied
 * @param[out] copy A pointer to where to store the copied map
 */
void map_copy(const map m, map* copy);

/**
 * @brief Add a value with the specified key to the map
 * 
 * @param[in] m The map
 * @param[in] key Key of the value to add
 * @param[in] value Value to add
 */
void map_add(map m, const void * key, const void * value);

/**
 * @brief Gets the value with the specified key from the map
 * 
 * @param[in] m The map
 * @param[in] key The key of the value to get
 * @param[out] value The
 */
void map_get(map m, const void* key, const void ** value);

/**
 * @brief Removes the value with the specified key from the map
 * 
 * @param m The map
 * @param key The key of the value to remove
 */
void map_remove(map m, const void* key);

/**
 * @brief Gets the number of entries in the map
 * 
 * @param[in] m The map 
 * 
 * @return size_t The number of entries in the map
 */
size_t map_count(const map m);

void   map_iter_new(const map map, map_iter* iter);
void   map_iter_delete(map_iter iter);
void   map_iter_next(map_iter iter, void** key, void** value);

#define TYPED_MAP(T)                                 \
    typedef struct map_##T* map_##T;                 \
    inline void map_##T##_new(map_##T* map)          \
    {                                                \
        map_new((map*)map);                          \
    }                                                \
                                                     \
    inline void map_##T##_add(map_##T map, T* item)  \
    {                                                \
        map_add((map)map, (void*)item);              \
    }                                                \
                                                     \
    inline size_t map_##T##_count(const map_##T map) \
    {                                                \
        return map_count((map)map);                  \
    }

#endif // MAP_H