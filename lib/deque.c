#include <scc/deque.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

size_t scc_deque_impl_npad(void const *deque);
size_t scc_deque_capacity(void const *deque);
size_t scc_deque_size(void const *deque);
bool scc_deque_empty(void const *deque);
size_t scc_deque_impl_push_back_index(void *deque);
size_t scc_deque_impl_push_front_index(void *deque);
size_t scc_deque_impl_pop_back_index(void *deque);
size_t scc_deque_impl_pop_front_index(void *deque);
size_t scc_deque_impl_back_index(void const *deque);
void scc_deque_clear(void *deque);
void *scc_deque_impl_iter_start(void *deque, size_t elemsize);
void *scc_deque_impl_iter_end(void *deque, size_t elemsize);

static inline bool scc_deque_get_dynalloc(void const *deque) {
    return ((unsigned char const *)deque)[-1];
}

static inline void scc_deque_set_dynalloc(void *deque) {
    ((unsigned char *)deque)[-1] = 1;
}

static inline size_t scc_deque_bytesize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_deque_base) + npad;
}

static struct scc_deque_base *scc_deque_alloc(size_t capacity, size_t size, size_t elemsize, size_t npad) {
    size_t const nbytes = scc_deque_bytesize(capacity, elemsize, npad);
    struct scc_deque_base *base = malloc(nbytes);
    if (!base) {
        return 0;
    }
    assert((unsigned char *)base + nbytes == &base->rd_buffer[npad] + capacity * elemsize);
    base->rd_size = size;
    base->rd_capacity = capacity;
    base->rd_buffer[npad - 2u] = npad - 2 * sizeof(unsigned char);
    scc_deque_set_dynalloc(&base->rd_buffer[npad]);
    return base;
}

static bool scc_deque_grow(void **dequeaddr, size_t newcap, size_t elemsize) {
    size_t const npad = scc_deque_impl_npad(*dequeaddr);
    struct scc_deque_base *prev = scc_deque_impl_base(*dequeaddr);
    struct scc_deque_base *base = scc_deque_alloc(newcap, prev->rd_size, elemsize, npad);
    if (!base) {
        return false;
    }

    assert(prev->rd_begin == prev->rd_end);
    unsigned char *data = base->rd_buffer + npad;
    size_t befwrap = prev->rd_capacity - prev->rd_begin;
    size_t first = befwrap * elemsize;
    unsigned char *firstsrc = (unsigned char *)*dequeaddr + prev->rd_begin * elemsize;
    scc_memcpy(data, firstsrc, first);
    if (prev->rd_begin) {
        scc_memcpy(data + first, *dequeaddr, (prev->rd_size - befwrap) * elemsize);
    }
    base->rd_begin = 0;
    base->rd_end = base->rd_size;

    if (scc_deque_get_dynalloc(*dequeaddr)) {
        free(prev);
    }

    *dequeaddr = data;
    return true;
}

void *scc_deque_impl_new(struct scc_deque_base *base, size_t offset, size_t capacity) {
    base->rd_capacity = capacity;
    unsigned char *handle = (unsigned char *)base + offset;
    handle[-2] = offset - sizeof(*base) - 2 * sizeof(*handle);
    return handle;
}

void *scc_deque_impl_new_dyn(size_t dequesz, size_t offset, size_t capacity) {
    struct scc_deque_base *base = calloc(dequesz, sizeof(unsigned char));
    if (!base) {
        return 0;
    }

    void *deque = scc_deque_impl_new(base, offset, capacity);
    scc_deque_set_dynalloc(deque);
    return deque;
}

void scc_deque_free(void *deque) {
    if (scc_deque_get_dynalloc(deque)) {
        free(scc_deque_impl_base(deque));
    }
}

bool scc_deque_impl_prepare_push(void *dequeaddr, size_t elemsize) {
    struct scc_deque_base *base = scc_deque_impl_base(*(void **)dequeaddr);
    if (base->rd_size < base->rd_capacity) {
        return true;
    }
    size_t newcap = base->rd_capacity << 1u;
    return scc_deque_grow(dequeaddr, newcap, elemsize);
}

bool scc_deque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize) {
    struct scc_deque_base *base = scc_deque_impl_base(*(void **)dequeaddr);
    if (base->rd_capacity >= capacity) {
        return true;
    }
    if (!scc_bits_is_power_of_2(capacity)) {
        unsigned shifts;
        for (shifts = 0u; capacity; capacity >>= 1u, ++shifts);
        capacity = 1u << shifts;
    }

    assert(scc_bits_is_power_of_2(capacity));
    return scc_deque_grow(dequeaddr, capacity, elemsize);
}

void *scc_deque_impl_clone(void const *deque, size_t elemsize) {
    struct scc_deque_base const *obase = scc_deque_impl_base_qual(deque, const);
    size_t const basesz = (unsigned char const *)deque - (unsigned char const *)obase;
    size_t const bytesz = obase->rd_capacity * elemsize + basesz;
    scc_when_mutating(assert(bytesz > obase->rd_capacity * elemsize));
    struct scc_deque_base *nbase = malloc(bytesz);
    if (!nbase) {
        return 0;
    }

    scc_memcpy(nbase, obase, bytesz);

    unsigned char *ndeque = (unsigned char *)nbase + basesz;
    scc_deque_set_dynalloc(ndeque);
    return ndeque;
}

void *scc_deque_impl_iter_next(void *it, void *deque, size_t elemsize) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    size_t off = (unsigned char const *)it - (unsigned char const *)deque;
    off /= elemsize;
    off = (off + 1u) & (base->rd_capacity - 1u);
    return (unsigned char *)deque + off * elemsize;
}
