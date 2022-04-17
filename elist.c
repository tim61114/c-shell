#include <stdio.h> // delete this later
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "elist.h"

#define DEFAULT_INIT_SZ 10
#define RESIZE_MULTIPLIER 2

struct elist {
    size_t capacity;         /*!< Storage space allocated for list items */
    size_t size;             /*!< The actual number of items in the list */
    void **element_storage;  /*!< Pointer to the beginning of the array  */
};

bool idx_is_valid(struct elist *list, size_t idx);

struct elist *elist_create(size_t list_sz)
{
    struct elist *list = malloc(sizeof(struct elist));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = list_sz == 0 ? DEFAULT_INIT_SZ : list_sz;
    list->element_storage = (malloc(sizeof(void*) * list->capacity));
    return list;
}

void elist_destroy(struct elist *list)
{
    free(list->element_storage);
    free(list);
}

int elist_set_capacity(struct elist *list, size_t capacity)
{
    if (capacity == 0) {
        list->size = 0;
        list->capacity = DEFAULT_INIT_SZ;
        return 0;
    }

    void *new_elements = realloc(list->element_storage, sizeof(void*) * capacity);
    if (new_elements == NULL) {
        return -1;
    }
    list->element_storage = new_elements;
    list->capacity = capacity;

    if (list->capacity < list->size) {
        list->size = list->capacity;
    }

    return 0;
}

size_t elist_capacity(struct elist *list)
{
    return list->capacity;
}

ssize_t elist_add(struct elist *list, void *item)
{
    if (list->size >= list->capacity) {
        if (elist_set_capacity(list, list->capacity * RESIZE_MULTIPLIER) == -1) {
            return -1;
        }
    }

    size_t index = list->size++;
    list->element_storage[index] = item;

    return index;
}

int elist_set(struct elist *list, size_t idx, void *item)
{
    if (idx >= list->size) {
        return -1;
    }

    list->element_storage[idx] = item;
    return 0;
}

void *elist_get(struct elist *list, size_t idx)
{
    return idx >= list->size ? NULL : list->element_storage[idx];
}

size_t elist_size(struct elist *list)
{
    return list->size;
}

int elist_remove(struct elist *list, size_t idx)
{
    if (idx >= list->size) {
        return -1;
    }

    for (int i = idx + 1; i < list->size; i++) {
        list->element_storage[i - 1] = list->element_storage[i];
    }
    //memmove(list->element_storage + idx, list->element_storage + idx + 1, sizeof(list->element_storage) * (list->size - idx - 1));
    list->size--;
    return 0;
}

void elist_clear(struct elist *list)
{
    list->size = 0;
}

void elist_clear_mem(struct elist *list)
{
    memset(list->element_storage, 0, list->size * sizeof(void*));
    list->size = 0;
}

ssize_t elist_index_of(struct elist *list, void *item, size_t item_sz)
{
    for (int i = 0; i < list->size; i++) {
        if (!memcmp(item, list->element_storage[i], item_sz)) {
            return i;
        }
    }
    return -1;
}

void elist_sort(struct elist *list, int (*comparator)(const void *, const void *))
{
    qsort(list->element_storage, list->size, sizeof(void *), comparator);
}

bool idx_is_valid(struct elist *list, size_t idx)
{
    return idx >= 0 && idx < list->size;
}
