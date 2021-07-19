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
#include "include/slist.h"

#include <assert.h>

typedef struct slist_node_s
{
    struct slist_node_s* next;
    void*                item;
} * slist_node;

struct slist_s
{
    slist_node first;
    slist_node last;
    size_t     count;
    slist_conf conf;
};

struct slist_iter_s
{
    slist      list;
    slist_node current;
    slist_node next;
    size_t     index;
};

/*
 * Private Methods
 */
static void remove_node(slist list, slist_node node, slist_node prev)
{
    assert(prev->next == node);

    if(prev == NULL)
    {
        list->first = node->next;
    }
    else
    {
        prev->next = node->next;
    }

    if(node->next == NULL)
    {
        list->last = prev;
    }
    
    list->conf.mem_free(node);
    list->count--;
}

/*
 * Public Methods
 */
void slist_conf_init(slist_conf* conf)
{
    conf->mem_alloc = malloc;
    conf->mem_calloc = calloc;
    conf->mem_free = free;
}

util_err_t slist_new(slist* list)
{
    slist_conf conf;
    slist_conf_init(&conf);
    return slist_new_conf(&conf, list);
}

util_err_t slist_new_conf(const slist_conf* const conf, slist* list)
{
    slist newList = (slist)conf->mem_calloc(1, sizeof(*newList));

    if(newList == NULL)
    {
        *list = NULL;
        return UTIL_ERR_ALLOC;
    }

    newList->conf.mem_alloc = conf->mem_alloc;
    newList->conf.mem_calloc = conf->mem_calloc;
    newList->conf.mem_free = conf->mem_free;

    *list = newList;
    return UTIL_OK;
}

void slist_delete(slist list)
{
    // Delete all nodes
    slist_clear(list);

    // Delete the list itself
    list->conf.mem_free(list);
}

util_err_t slist_copy_shallow(const slist list, slist* copy)
{
    slist newList = (slist)list->conf.mem_calloc(1, sizeof(*newList));

    if(newList == NULL)
    {
        *copy = NULL;
        return UTIL_ERR_ALLOC;
    }

    util_err_t err;
    slist_node node = list->first;
    while (node != NULL)
    {
        err = slist_add(newList, node->item);

        if(err != UTIL_OK)
        {
            slist_delete(newList);
            *copy = NULL;
            return err;
        }

        node = node->next;
    }

    *copy = newList;
    return UTIL_OK;
}

util_err_t slist_copy_deep(const slist list, void (copyFn)(const void* item, void** copy), slist* copy)
{
    slist newList = (slist)list->conf.mem_calloc(1, sizeof(*newList));

    if(newList == NULL)
    {
        *copy = NULL;
        return UTIL_ERR_ALLOC;
    }

    util_err_t err;
    slist_node node = list->first;
    while (node != NULL)
    {
        void* newItem;
        copyFn(node->item, &newItem);

        err = slist_add(newList, newItem);

        if(err != UTIL_OK)
        {
            // TODO: Call delete variant that also frees resources from items
            slist_delete(newList);
            *copy = NULL;
            return err;
        }

        node = node->next;
    }

    *copy = newList;
    return UTIL_OK;
}

util_err_t slist_add(slist list, void* item)
{
    slist_node newNode = (slist_node)list->conf.mem_calloc(1, sizeof(*newNode));

    if(newNode == NULL)
    {
        return UTIL_ERR_ALLOC;
    }

    newNode->item = item;

    // Add the new node to the list
    if(list->first == NULL)
    {
        // List is empty, set first and last node
        list->first = newNode;
        list->last = newNode;
    }
    else
    {
        // List has atleast 1 entry, set next node of last node and update list last node
        list->last->next = newNode;
        list->last = newNode;
    }
    
    list->count++;

    return UTIL_OK;
}

util_err_t slist_remove(slist list, void* item)
{
    slist_node prev = NULL;
    slist_node node = list->first;

    // Find node with the item
    while (node != NULL && node->item != item)
    {
        prev = node;
        node = node->next;
    }

    if(node == NULL)
    {
        return UTIL_ERR_NOT_FOUND;
    }

    // Remove it from the list
    remove_node(list, node, prev);

    return UTIL_OK;
}

util_err_t slist_remove_at(slist list, size_t index)
{
    slist_node prev = NULL;
    slist_node node = list->first;

    // Find the node at the specified index
    for(size_t i = 0; node != NULL && i < index; i++)
    {
        prev = node;
        node = node->next;
    }

    if(node == NULL)
    {
        return UTIL_ERR_OUT_OF_RANGE;
    }

    // Remove it from the list
    remove_node(list, node, prev);

    return UTIL_OK;
}

void slist_clear(slist list)
{
    slist_node node = list->first;
    slist_node next;
    while (node != NULL)
    {
        next = node->next;
        list->conf.mem_free(node);
        node = next;
    }
}

size_t slist_count(const slist list)
{
    return list->count;
}

util_err_t slist_iter_new(const slist list, slist_iter* iter)
{
    slist_iter newIter = (slist_iter)list->conf.mem_alloc(sizeof(*newIter));

    if(newIter == NULL)
    {
        *iter = NULL;
        return UTIL_ERR_ALLOC;
    }

    newIter->list = list;
    newIter->current = NULL;
    newIter->next = list->first;
    newIter->index = 0;

    *iter = newIter;

    return UTIL_OK;
}

util_err_t slist_iter_delete(slist_iter iter)
{
    iter->list->conf.mem_free(iter);

    return UTIL_OK;
}

util_err_t slist_iter_next(slist_iter iter, void** item)
{
    if (iter->next == NULL)
    {
        *item = NULL;
        return UTIL_ITER_END;
    }

    slist_node nextNode = iter->next;

    iter->current = nextNode;
    iter->next = nextNode->next;
    iter->index++;

    *item = nextNode->item;

    return UTIL_OK;
}

size_t slist_iter_index(const slist_iter iter)
{
    // Return index - 1 since the 0th iter created by iter_new does not point to an element
    return iter->index - 1;
}