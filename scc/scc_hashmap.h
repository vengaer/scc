#ifndef SCC_HASHMAP_H
#define SCC_HASHMAP_H

#include "scc_mem.h"

#include <stddef.h>

/* scc_hashmap_impl_pair
 *
 * Internal use only
 *
 * Expands to a struct appropriate for storing
 * pairs of key-value types
 *
 * keytype
 *      The key type to be stored in the map
 *
 * valuetype
 *      The value type to be stored in the map
 */
#define scc_hashmap_impl_pair(keytype, valuetype)                                       \
    struct { keytype hp_key; valuetype hp_val; }

/* scc_hashmap
 *
 * Expands to a pointer suitable for
 * storing a hash map handle for the given
 * key and value types
 *
 * keytype
 *      The key type to be stored in the map
 *
 * valuetype
 *      The value type to be stored in the map
 */
#define scc_hashmap(keytype, valuetype)                                                 \
    scc_hashmap_impl_pair(keytype, valuetype) *

#define SCC_HASHMAP_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

enum { SCC_HASHMAP_STACKCAP = 32 };

#ifndef SCC_EQ_TYPEDEF
#define SCC_EQ_TYPEDEF
/* scc_eq
 *
 * Signature of function used for equality comparisons
 */
typedef _Bool(*scc_eq)(void const *, void const *);
#endif /* SCC_EQ_TYPEDEF */

#ifndef SCC_HASH_TYPEDEF
#define SCC_HASH_TYPEDEF
/* scc_hash
 *
 * Signature of function used for hashing
 */
typedef unsigned long long(*scc_hash)(void const*, size_t);
#endif /* SCC_HASH_TYPEDEF */

typedef unsigned char scc_hashmap_metatype;

/* struct scc_hashmap_perfevts
 *
 * Internal use only
 *
 * Counters for tracking performance-
 * related events.
 *
 * size_t ev_n_rehashes
 *      Number of times the hash map
 *      has been rehashed
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
 *      Total size of the map, in bytes
 */
struct scc_hashmap_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

/* struct scc_hashmap_base
 *
 * Internal use only
 *
 * Base struct of the hash map. Exposed only indirectly
 * through a fat pointer. Given a struct scc_hashmap_base
 * base, the address of said pointer is obtained by computing
 * &base.hm_fwoff + base.hm_fwoff + 1.
 *
 * scc_eq hm_eq
 *      Pointer to function used for equality comparison
 *
 * scc_hash hm_hash
 *      Pointer to hash function.
 *
 * size_t hm_valoff
 *      Offset of the value array relative the base address.
 *      Used to access the values in the FAM part of the struct
 *
 * size_t hm_mdoff
 *      Offset of metadata array relative the base address. This
 *      is used to access the metadata in the FAM part of the
 *      struct
 *
 * size_t hm_size
 *      Size of the hash map
 *
 * size_t hm_capacity
 *      Capacity of the map. Always a power of 2.
 *
 * size_t hm_pairsize
 *      Size of the key-value pair handle
 *
 * struct scc_hashmap_perfevts hm_perf
 *      Performance counters
 *
 * unsigned short hm_keyalign
 *      Alignment of the key type
 *
 * unsigned short hm_valalign
 *      Alignment of the value type
 *
 * unsigned char hm_dynalloc
 *      Set to 1 if the map was allocated dynamically.
 *      Upon initial construction, the map is allocated on the
 *      stack. This field is set on the first rehash.
 *
 * unsigned char hm_valpad
 *      Tracks the number of padding bytes between the key and value
 *      in the internal pair struct.
 *
 * unsigned char hm_fwoff
 *      Offset of the pointer exposed through the API. The offset
 *      is relative to the field itself.
 *
 * unsigned char hm_buffer[]
 *      FAM hiding type-specific details. For the exact layout,
 *      refer to scc_hashmap_impl_layout.
 */
struct scc_hashmap_base {
    scc_eq hm_eq;
    scc_hash hm_hash;
    size_t hm_valoff;
    size_t hm_mdoff;
    size_t hm_size;
    size_t hm_capacity;
    size_t hm_pairsize;
#ifdef SCC_PERFEVTS
    struct scc_hashmap_perfevts hm_perf;
#endif
    unsigned short hm_keyalign;
    unsigned short hm_valalign;
    unsigned char hm_dynalloc;
    unsigned char hm_valpad;
    unsigned char hm_fwoff;
    unsigned char hm_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHMAP_INJECT_PERFEVTS(name)                                   \
    struct scc_hashmap_perfevts name;
#else
#define SCC_HASHMAP_INJECT_PERFEVTS(name)
#endif

/* scc_hashmap_impl_layout
 *
 * Internal use only
 *
 * The actual layout of the hash map instantiated for a given
 * key-value type pair. The hm_eq through hm_fwoff members are
 * the same as for struct scc_hashmap_base.
 *
 * unsigned char hm_bkoff;
 *      Field used for tracking the padding between hm_fwoff and
 *      hm_curr. Just as hm_fwoff is used to compute the address
 *      of hm_curr given a base address, hm_bkoff is used to
 *      compute the base address given the address of hm_curr.
 *
 *      The primary purpose of this field is to force injection of
 *      padding bytes between hm_fwoff and hm_curr. Whether the offset
 *      is actually stored in this field depends on the alignment of
 *      the type parameter. In practice, this occurs only for 1-byte
 *      aligned types. For types with stricter alignment requirements,
 *      the last padding byte is used instead.
 *
 * scc_hashmap_impl_pair(keytype, valuetype) hm_curr
 *      Volatile instance of the pair stored in the hash map. Used
 *      for temporary storage of values to be inserted or probed
 *      for in order to provide rvalue support.
 *
 * keytype hm_keys[SCC_HASHMAP_STACKCAP]
 *      The keys array
 *
 * valuetype hm_vals[SCC_HASHMAP_STACKCAP]
 *      The value array
 *
 * scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP]
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
 * scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ];
 *      Guard to allow for unaligned vector loads without risking
 *      reads from potential guard pages. The SCC_HASHMAP_GUARDSZ
 *      low bytes of hm_meta are mirrored in the guard.
 */
#define scc_hashmap_impl_layout(keytype, valuetype)                                         \
    struct {                                                                                \
        scc_eq hm_eq;                                                                       \
        scc_hash hm_hash;                                                                   \
        size_t hm_valoff;                                                                   \
        size_t hm_mdoff;                                                                    \
        size_t hm_size;                                                                     \
        size_t hm_capacity;                                                                 \
        size_t hm_pairsize;                                                                 \
        SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                                \
        unsigned short hm_keyalign;                                                         \
        unsigned short hm_valalign;                                                         \
        unsigned char hm_dynalloc;                                                          \
        unsigned char hm_valpad;                                                            \
        unsigned char hm_fwoff;                                                             \
        unsigned char hm_bkoff;                                                             \
        scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                                  \
        keytype hm_keys[SCC_HASHMAP_STACKCAP];                                              \
        valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                            \
        scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP];                                 \
        scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ];                                 \
    }


/* scc_hashmap_impl_init
 *
 * Internal use only
 *
 * Initialize an empty hash map and return a handle to it.
 *
 * struct scc_hashmap_base *base
 *      Address of hash map base. The handle returned by the function
 *      refers to the hm_curr entry in this map
 *
 * size_t coff
 *      Offset of hm_curr relative the address of *base
 *
 * size_t valoff
 *      Offset of the value type in the key-value pair
 *
 * size_t keysize
 *      Size of the key type
 */
void *scc_hashmap_impl_init(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize);

/* scc_hashmap_init_with_hash
 *
 * Initialize hash map with a specified hash function.
 *
 * keytype
 *      The key type the hash map is to store
 *
 * valuetype
 *      The value type the hash map is to store
 *
 * scc_eq eq
 *      Pointer to function to use for equality comparisons
 *
 * scc_hash hash
 *      Pointer to function to use for hashing
 */
#define scc_hashmap_init_with_hash(keytype, valuetype, eq, hash)                            \
    scc_hashmap_impl_init(                                                                  \
        (void *)&(scc_hashmap_impl_layout(keytype, valuetype)){                             \
            .hm_eq = eq,                                                                    \
            .hm_hash = hash,                                                                \
            .hm_valoff = offsetof(scc_hashmap_impl_layout(keytype, valuetype), hm_vals),    \
            .hm_mdoff = offsetof(scc_hashmap_impl_layout(keytype, valuetype), hm_meta),     \
            .hm_capacity = SCC_HASHMAP_STACKCAP,                                            \
            .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),               \
            .hm_keyalign = scc_alignof(keytype),                                            \
            .hm_valalign = scc_alignof(valuetype)                                           \
        },                                                                                  \
        offsetof(scc_hashmap_impl_layout(keytype, valuetype), hm_curr),                     \
        offsetof(scc_hashmap_impl_pair(keytype, valuetype), hp_val),                        \
        sizeof(keytype)                                                                     \
    )

/* scc_hashmap_init
 *
 * Initialize a hash map. The hash function is set to an fnv1a.
 *
 * keytype
 *      The key type the hash map is to store
 *
 * valuetype
 *      The value type the hash map is to store
 *
 * scc_eq eq
 *      Pointer to function to use for equality comparisons
 */
#define scc_hashmap_init(keytype, valuetype, eq)                                            \
    scc_hashmap_init_with_hash(keytype, valuetype, eq, scc_hashmap_fnv1a)

/* scc_hashmap_impl_bkpad
 *
 * Internal use only
 *
 * Compute number of padding bytes between hm_curr and hm_fwoff
 */
inline size_t scc_hashmap_impl_bkpad(void const *map) {
    return ((unsigned char const *)map)[-1] + sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
}

/* scc_hashmap_impl_base_qual
 *
 * Internal use only
 *
 * Obtain qualified pointer to the struct scc_hashmap_base
 * corresponding to the given map
 */
#define scc_hashmap_impl_base_qual(map, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(map) - scc_hashmap_impl_bkpad(map),          \
        struct scc_hashmap_base,                                            \
        hm_fwoff,                                                           \
        qual                                                                \
    )

/* scc_hashmap_impl_base
 *
 * Internal use only
 *
 * Obtain unqualified pointer to the struct scc_hashmap_base
 * corresponding to the given map
 */
#define scc_hashmap_impl_base(map)                                          \
    scc_hashmap_impl_base_qual(map,)

/* scc_hashmap_fnv1a
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
unsigned long long scc_hashmap_fnv1a(void const *data, size_t size);


/* scc_hashmap_free
 *
 * Reclaim memory used by the given hash map
 */
void scc_hashmap_free(void *map);

/* scc_hashmap_impl_insert
 *
 * Internal use only
 *
 * Insert the key-value pair in ht_curr in the map. Return true
 * on success.
 *
 * void *mapaddr
 *      Address of the map handle
 *
 * size_t keysize
 *      Size of the key type
 *
 * size_t valsize
 *      Size of the value type
 */
_Bool scc_hashmap_impl_insert(void *mapaddr, size_t keysize, size_t valsize);


/* scc_hashmap_insert
 *
 * Insert a key-value pair in the hash map, replacing existing
 * values if present. May result in reallocation.
 *
 * Expands to true if the pair was successfully inserted
 *
 * scc_hashmap(keytype, valuetype) *mapaddr
 *      Address of the map handle
 *
 * keytype' key
 *      The key for which the value is to be inserted. Subject to
 *      implicit conversion should keytype and keytype' not be the same.
 *
 * valuetype' value
 *      The value to insert. Subject to implicit conversion should valuetype
 *      and valuetype' not be the same.
 */
#define scc_hashmap_insert(mapaddr, key, value)                             \
    scc_hashmap_impl_insert((                                               \
            (*(mapaddr))->hp_key = key,                                     \
            (*(mapaddr))->hp_val = value,                                   \
            (mapaddr)                                                       \
        ),                                                                  \
        sizeof((*(mapaddr))->hp_key),                                       \
        sizeof((*(mapaddr))->hp_val)                                        \
    )

/* scc_hashmap_capacity
 *
 * Return the current capacity of the hash map
 *
 * void const *map
 *      Handle to the hash map in question
 */
inline size_t scc_hashmap_capacity(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_capacity;
}

/* scc_hashmap_size
 *
 * Return the current size of the hash map
 *
 * void const *map
 *      Handle to the hash map in question
 */
inline size_t scc_hashmap_size(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_size;
}

/* scc_hashmap_impl_find
 *
 * Internal use only
 *
 * Probe for the value in *handle in the map. Return a pointer to
 * the address of the value slot corresponding to the given key
 * if found, NULL otherwise
 *
 * void const *map
 *      Handle to the hash map in question
 *
 * size_t keysize
 *      Size of the key type
 *
 * size_t valsize
 *      Size of the value type
 */
void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize);

/* scc_hashmap_find
 *
 * Probe for value in the hash map
 *
 * Expands to the address of the value slot corresponding to the
 * given key if found, otherwise NULL
 *
 * scc_hashmap(keytype, valuetype) map
 *      The map handle
 *
 * keytype' key
 *      The key to probe for. Subject to implicit conversion should keytype
 *      and keytype' not be the same.
 */
#define scc_hashmap_find(map, key)                                      \
    scc_hashmap_impl_find(                                              \
        ((map)->hp_key = (key), (map)),                                 \
        sizeof((map)->hp_key),                                          \
        sizeof((map)->hp_val)                                           \
    )

/* scc_hashmap_impl_remove
 *
 * Internal use only
 *
 * Remove the key-value stored in the handle from the hash
 * map. Return true if a key was found, otherwise false
 *
 * void *map
 *      The map handle
 */
_Bool scc_hashmap_impl_remove(void *map, size_t keysize);

/* scc_hashmap_remove
 *
 * Remove a key-value pair from the hash map. Return true if the
 * key was found in the map
 *
 * scc_hashmap(keytype, valuetype) map
 *      The map handle
 *
 * keytype' key
 *      Key to remove. Subject to implicit conversion should keytype and
 *      keytype' not be the same
 */
#define scc_hashmap_remove(map, value)                                      \
    scc_hashmap_impl_remove(((map)->hp_key = (value), (map)), sizeof((map)->hp_key))

#endif /* SCC_HASHMAP_H */
