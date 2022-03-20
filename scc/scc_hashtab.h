#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include "scc_mem.h"

#include <stddef.h>

/* scc_hashtab
 *
 * Expands to a pointer suitable for
 * storing a hash table handle for the given
 * type
 *
 * type
 *      The type stored in the table
 */
#define scc_hashtab(type) type *

#define SCC_HASHTAB_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

enum { SCC_HASHTAB_STACKCAP = 32 };

/* scc_eq
 *
 * Signature of function used for equality comparisons
 */
typedef _Bool(*scc_eq)(void const *, void const *);

/* scc_hash
 *
 * Signature of function used for hashing
 */
typedef unsigned long long(*scc_hash)(void const*, size_t);

/* scc_hashtab_metatype
 *
 * Internal use only
 *
 * Type used for storing hash table metadata
 */
typedef unsigned char scc_hashtab_metatype;

/* struct scc_hashtab_perfevts
 *
 * Internal use only
 *
 * Counters for tracking performance-
 * related events.
 *
 * size_t ev_n_rehashes
 *      Number of times the hash table
 *      hash been rehashed
 *
 * size_t ev_n_eqs
 *      Number of calls to eq performed
 *
 * size_t ev_n_hash
 *      Number of calls to hash performed
 *
 * size_t ev_n_inserts
 *      Number of successful insertions performed
 *
 * size_t ev_bytesz
 *      Total size of the table, in bytes
 */
struct scc_hashtab_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

/* struct scc_hashtab_base
 *
 * Internal use only
 *
 * Base struct of the hash table. Never exposed directly
 * through the API. Instead, all "public" functions operate
 * on a fat pointer referred to as a handle. Given a struct
 * scc_hashtab_base base, the address of said pointer is
 * obtained by computing &base.ht_fwoff + base.ht_fwoff - 1.
 *
 * scc_eq ht_eq;
 *      Pointer to function used for equality comparison
 *
 * scc_hash ht_hash;
 *      Pointer to hash function.
 *
 * size_t ht_mdoff;
 *      Offset of metadata array relative base address. This
 *      is used to access the metadata in the FAM part of the
 *      struct.
 *
 * size_t ht_size;
 *      Size of the hash table
 *
 * size_t ht_capacity;
 *      Capacity of the hash table. Always a power of two to
 *      allow for efficient modulo computation.
 *
 * struct scc_hashtab_perfevts ht_perf;
 *      Performance counters.
 *
 * unsigned char ht_dynalloc;
 *      Set to 1 if the current table was allocated dynamically.
 *      Upon initial construction, the table is allocated on the
 *      stack. This field is set on the first rehash.
 *
 * unsigned char ht_fwoff;
 *      Offset of the pointer exposed through the API. The offset
 *      is relative to the field itself.
 *
 * unsigned char ht_buffer[];
 *      FAM hiding type-specific details. For the exact layout,
 *      refer to scc_hashtab_impl_layout.
 */
struct scc_hashtab_base {
    scc_eq ht_eq;
    scc_hash ht_hash;
    size_t ht_mdoff;
    size_t ht_size;
    size_t ht_capacity;
#ifdef SCC_PERFEVTS
    struct scc_hashtab_perfevts ht_perf;
#endif
    unsigned char ht_dynalloc;
    unsigned char ht_fwoff;
    unsigned char ht_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHTAB_INJECT_PERFEVTS(name)                                   \
    struct scc_hashtab_perfevts name;
#else
#define SCC_HASHTAB_INJECT_PERFEVTS(name)
#endif

/* scc_hashtab_impl_layout(type)
 *
 * Internal use only
 *
 * The actual layout of the hash table instantiated for a given
 * type. The ht_eq through ht_fwoff members are the same as for
 * struct scc_hashtab_base.
 *
 * unsigned char ht_bkoff;
 *      Field used for tracking the padding between ht_fwoff and
 *      ht_curr. Just as ht_fwoff is used to compute the address
 *      of ht_curr given a base address, ht_bkoff is used to
 *      compute the base address given the address of ht_curr.
 *
 *      The primary purpose of this field is to force injection of
 *      padding bytes between ht_fwoff and ht_curr. Whether the offset
 *      is actually stored in this field depends on the alignment of
 *      the type parameter. In practice, this occurs only for 1-byte
 *      aligned types. For types with stricter alignment requirements,
 *      the last padding byte is used instead.
 *
 * type ht_curr;
 *      Temporary instance of the type stored in the hash table.
 *      Values to be inserted or used in probing are stored here to
 *      provide rvalue support. The address of this element is the
 *      one exposed through the API.
 *
 * type ht_data[SCC_HASHTAB_STACKCAP];
 *      The data array in which elements in the table are stored.

 * scc_hashtab_metatype ht_meta[SCC_HASHTAB_STACKCAP];
 *      Metadata array used for tracking vacant slots. Each slot n
 *      in the metadata array corresponds to slot n in the data array.
 *
 *      A slot is unused if the value of its metadata entry is 0. If the
 *      byte has a non-zero value with the MSB unset, the slot was previously
 *      occupied but has since been vacated. Such slots can be reused during
 *      insertion but cannot be used as probing stops.
 *
 *      An entry with the MSB set signifies that the slot is occupied. In
 *      this case, the remaining bits are the CHAR_BIT - 1 most
 *      significant bytes of the hash computed for the value in the
 *      corresponding slot in the data array. This is used for
 *      avoiding unnecessary calls to eq.
 *
 * scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ];
 *      Guard to allow for unaligned vector loads without risking
 *      reads from potential guard pages. The SCC_HASHTAB_GUARDSZ
 *      low bytes of ht_meta are mirrored in the guard.
 */
#define scc_hashtab_impl_layout(type)                                       \
    struct {                                                                \
        scc_eq ht_eq;                                                       \
        scc_hash ht_hash;                                                   \
        size_t ht_mdoff;                                                    \
        size_t ht_size;                                                     \
        size_t ht_capacity;                                                 \
        SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                                \
        unsigned char ht_dynalloc;                                          \
        unsigned char ht_fwoff;                                             \
        unsigned char ht_bkoff;                                             \
        type ht_curr;                                                       \
        type ht_data[SCC_HASHTAB_STACKCAP];                                 \
        scc_hashtab_metatype ht_meta[SCC_HASHTAB_STACKCAP];                 \
        scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ];                 \
    }

/* scc_hashtab_impl_init
 *
 * Internal use only
 *
 * Initialize an empty hash table and return a handle to it.
 *
 * struct scc_hashtab_base *base
 *      Address of hash table base. The handle returned by the function
 *      refers to the ht_curr entry in this table
 *
 * scc_eq eq
 *      Pointer to the equality function to use
 *
 * scc_hash
 *      Pointer to the hash function to use
 *
 * size_t coff
 *      Offset of ht_curr relative the address of *base
 *
 * size_t mdoff
 *      Offset of ht_meta relative the address of *base
 *
 * size_t cap
 *      The capacity allocated for *base
 */
void *scc_hashtab_impl_init(struct scc_hashtab_base *base, scc_eq eq, scc_hash hash, size_t coff, size_t mdoff, size_t cap);

/* scc_hashtab_init_with_hash
 *
 * Initialize hash table with a specified hash function.
 *
 * type
 *      Type the hash table is to store
 *
 * scc_eq eq
 *      Pointer to function to use for equality comparisons
 *
 * scc_hash hash
 *      Pointer to function to use for hashing
 */
#define scc_hashtab_init_with_hash(type, eq, hash)                          \
    scc_hashtab_impl_init(                                                  \
        (void *)&(scc_hashtab_impl_layout(type)){ 0 },                      \
        eq,                                                                 \
        hash,                                                               \
        offsetof(scc_hashtab_impl_layout(type), ht_curr),                   \
        offsetof(scc_hashtab_impl_layout(type), ht_meta),                   \
        SCC_HASHTAB_STACKCAP                                                \
    )

/* scc_hashtab_init
 *
 * Initialize a hash table. The hash function is set to an fnv1a.
 *
 * type
 *      Type the hash table is to store
 *
 * scc_eq eq
 *      Pointer to function to use for equality comparisons
 */
#define scc_hashtab_init(type, eq)                                          \
    scc_hashtab_init_with_hash(type, eq, scc_hashtab_fnv1a)

/* scc_hashtab_impl_bkpad
 *
 * Internal use only
 *
 * Compute number of padding bytes between ht_curr and ht_fwoff
 */
inline size_t scc_hashtab_impl_bkpad(void const *handle) {
    return ((unsigned char const *)handle)[-1] + sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
}

/* scc_hashtab_impl_base_qual
 *
 * Internal use only
 *
 * Obtain qualified pointer to the struct scc_hashtab_base
 * corresponding to the given handle
 */
#define scc_hashtab_impl_base_qual(handle, qual)                            \
    scc_container_qual(                                                     \
        (unsigned char qual *)(handle) - scc_hashtab_impl_bkpad(handle),    \
        struct scc_hashtab_base,                                            \
        ht_fwoff,                                                           \
        qual                                                                \
    )

/* scc_hashtab_impl_base
 *
 * Internal use only
 *
 * Obtain unqualified pointer to the struct scc_hashtab_base
 * corresponding to the given handle
 */
#define scc_hashtab_impl_base(handle)                                       \
    scc_hashtab_impl_base_qual(handle,)

/* scc_hashtab_fnv1a
 *
 * Simple alternative Fowler-Voll-No implementation
 *
 * void const *data
 *      Address of data to be hashed. The data is treated as a
 *      consecutive array of bytes. Potential padding in structs
 *      must be explicitly initialized to avoid hash mismatches.
 *
 * size_t size
 *      Size of data, in bytes
 */
unsigned long long scc_hashtab_fnv1a(void const *data, size_t size);

/* scc_hashtab_free
 *
 * Reclaim memory used by the given hash table
 */
void scc_hashtab_free(void *handle);

/* scc_hashtab_impl_insert
 *
 * Internal use only
 *
 * Insert the value in ht_curr in the table. Return true
 * on success.
 *
 * void *handleaddr
 *      Address of the handle used to refer to the hash table
 *
 * size_t elemsize
 *      Size of the elements stored in the hash table
 */
_Bool scc_hashtab_impl_insert(void *handleaddr, size_t elemsize);

/* scc_hashtab_insert
 *
 * Insert a value in the hash table. May result in a reallocation.
 *
 * Expands to true if the value was successfully inserted
 *
 * type **handleaddr
 *      Address of the handle used for referring to the hash table.
 *
 * type' value
 *      The value to insert. Subject to implicit conversion should type
 *      and type' not be the same.
 */
#define scc_hashtab_insert(handleaddr, value)                               \
    scc_hashtab_impl_insert((**(handleaddr) = (value), (handleaddr)), sizeof(**(handleaddr)))

/* scc_hashtab_capacity
 *
 * Return the current capacity of the hash table
 *
 * void const *handle
 *      Handle to the hash table in question
 */
inline size_t scc_hashtab_capacity(void const *handle) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(handle, const);
    return base->ht_capacity;
}

/* scc_hashtab_size
 *
 * Return the current size of the hash table
 *
 * void const *handle
 *      Handle to the hash table in question
 */
inline size_t scc_hashtab_size(void const *handle) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(handle, const);
    return base->ht_size;
}

/* scc_hashtab_impl_find
 *
 * Internal use only
 *
 * Probe for the value in *handle in the table. Return a pointer to
 * the element if found, NULL otherwise
 *
 * void const *handle
 *      Handle to the hash table in question
 *
 * size_t elemsize
 *      Size of the elements stored in the hash table
 */
void const *scc_hashtab_impl_find(void const *handle, size_t elemsize);

/* scc_hashtab_find
 *
 * Probe for value in the hash table
 *
 * Expands to the address of the found element on success, NULL otherwise
 *
 * type *handleaddr
 *      Address of the handle used for referring to the hash table.
 *
 * type' value
 *      The value to probe for. Subject to implicit conversion should type
 *      and type' not be the same.
 */
#define scc_hashtab_find(handle, value)                                     \
    scc_hashtab_impl_find((*(handle) = (value), (handle)), sizeof(*(handle)))

/* scc_hashtab_impl_reserve
 *
 * Internal use only
 *
 * Reserve storage for specified number of slots. Return true on success,
 * otherwise false. On failure, *(void **)handleaddr remains untouched.
 *
 * type *handleaddr
 *      Address of the handle used for referring to the hash table
 *
 * size_t capacity
 *      The desired capacity. If it is not a power of 2, it is
 *      rounded up to the next such power.
 */
_Bool scc_hashtab_impl_reserve(void *handleaddr, size_t capacity, size_t elemsize);

/* scc_hashtab_reserve
 *
 * Reserve storage for at least the specified number of slots.
 * If the table could be successfully reallocated, the macro
 * expands to true. Otherwise, the macro expands to false and
 * **handleaddr remains untouched.
 *
 * type *handleaddr
 *      Address of the handle used for referring to the hash table
 *
 * size_t capacity
 *      The desired capacity
 */
#define scc_hashtab_reserve(handleaddr, capacity)                           \
    scc_hashtab_impl_reserve(handleaddr, capacity, sizeof(**(handleaddr)))

/* scc_hashtab_impl_remove
 *
 * Internal use only
 *
 * Remove the value stored in the handle from the hash table
 *
 * void *handle
 *      Handle used to refer to the hash table
 *
 * size_t elemsize
 *      Size of each value stored in the table
 */
_Bool scc_hashtab_impl_remove(void *handle, size_t elemsize);

/* scc_hashtab_remove
 *
 * Remove an element from the hash table. Return true if the
 * operation was successfully performed
 *
 * type *handle
 *      Handle used for referring to the hash table in question
 *
 * type' value
 *      Value to remove. Subject to implicit conversion should type and
 *      type' not be the same
 */
#define scc_hashtab_remove(handle, value)                                   \
    scc_hashtab_impl_remove((*(handle) = (value), (handle)), sizeof(*(handle)))

/* scc_hashtab_clear
 *
 * Remove all entries in a hash table
 *
 * void *handle
 *      Handle used for referring to the hash table
 */
void scc_hashtab_clear(void *handle);

#endif /* SCC_HASHTAB_H */
