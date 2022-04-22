#ifndef _ELIST_H_
#define _ELIST_H_

#include <sys/types.h>

struct elist {
    size_t capacity;         /*!< Storage space allocated for list items */
    size_t size;             /*!< The actual number of items in the list */
    void **element_storage;  /*!< Pointer to the beginning of the array  */
};

ssize_t elist_add(struct elist *list, void *item);
size_t elist_capacity(struct elist *list);
void elist_clear(struct elist *list);
void elist_clear_mem(struct elist *list);
struct elist *elist_create(size_t list_sz);
void elist_destroy(struct elist *list);
void *elist_get(struct elist *list, size_t idx);
struct elist *elist_get_sub(struct elist *list, size_t start_idx, size_t end_idx);
ssize_t elist_index_of(struct elist *list, void *item, size_t item_sz);
int elist_remove(struct elist *list, size_t idx);
int elist_set(struct elist *list, size_t idx, void *item);
int elist_set_capacity(struct elist *list, size_t capacity);
size_t elist_size(struct elist *list);
void elist_sort(struct elist *list, int (*comparator)(const void *, const void *));

#endif
