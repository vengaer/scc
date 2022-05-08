#include <scc/scc_ringdeque.h>

#include <stdbool.h>
#include <stdlib.h>

size_t scc_ringdeque_impl_npad(void const *deque);
size_t scc_ringdeque_capacity(void const *deque);
size_t scc_ringdeque_size(void const *deque);

/* scc_ringdeque_is_allocd
 *
 * Read the rd_dynalloc field
 *
 * void const *deque
 *      Handle to the ringdeque in question
 */
static inline bool scc_ringdeque_is_allocd(void const *deque) {
    return ((unsigned char const *)deque)[-1];
}

void *scc_ringdeque_impl_init(void *deque, size_t offset, size_t capacity) {
    struct scc_ringdeque_base *base = deque;
    base->rd_capacity = capacity;
    unsigned char *handle = (unsigned char *)base + offset;
    handle[-2] = offset - sizeof(*base) - 2 * sizeof(*handle);
    return handle;
}

void scc_ringdeque_free(void *deque) {
    if(scc_ringdeque_is_allocd(deque)) {
        free(scc_ringdeque_impl_base(deque));
    }
}
