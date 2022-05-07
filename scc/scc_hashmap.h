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

//! .. c:macro:: scc_hashmap(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for storing a handle to
//!     a hash map holding instances of the given key- and value types.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
#define scc_hashmap(keytype, valuetype)                                                 \
    scc_hashmap_impl_pair(keytype, valuetype) *

#define SCC_HASHMAP_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

enum { SCC_HASHMAP_STACKCAP = 32 };

//! .. c:type:: _Bool(*scc_hashmap_eq)(void const *, void const *)
//!
//!     Signature of the function used for equality comparisons.
typedef _Bool(*scc_hashmap_eq)(void const *, void const *);

//! .. c:type:: unsigned long long(*scc_hashmap_hash)(void const *, size_t)
//!
//!     Signature of the hash function used.
typedef unsigned long long(*scc_hashmap_hash)(void const *, size_t);

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
 * scc_hashmap_eq hm_eq
 *      Pointer to function used for equality comparison
 *
 * scc_hashmap_hash hm_hash
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
    scc_hashmap_eq hm_eq;
    scc_hashmap_hash hm_hash;
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
        scc_hashmap_eq hm_eq;                                                               \
        scc_hashmap_hash hm_hash;                                                           \
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

//! .. _scc_hashmap_init_with_hash:
//! .. c:function:: void *scc_hashmap_init_with_hash(keytype, valuetype, scc_hashmap_eq eq, scc_hashmap_hash hash)
//!
//!     Initializes a hash map using the specified hash function. The macro expands
//!     to a handle to a hash map with automatic storage duration. See
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes__map>` for an in-depth explanation of how to
//!     safely manage the handle.
//!
//!     The call cannot fail.
//!
//!     The returned pointer must be passed to :ref:`scc_hashmap_free <scc_hashmap_free>` to ensure
//!     allocated memory is reclaimed.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
//!     :param eq: Pointer to function to be used for equality comparison
//!     :param hash: Pointer to function to be used for key hashing
//!     :returns: Handle to a newly created hash map. The map is allocated in the stack
//!               frame of the current function and its lifetime tied to the scope in
//!               which :c:texpr:`scc_hashmap_init_with_hash` is invoked.
//!
//!     .. code-block:: C
//!         :caption: Example usage
//!
//!         extern _Bool eq(void const *l, void const *r);
//!         extern unsigned long long hash(void const *data, size_t size);
//!
//!         scc_hashmap(int, _Bool) map;
//!         {
//!             map = scc_hashmap_init_with_hash(int, _Bool, eq, hash);
//!             /* map is valid */
//!         }
//!         /* map is no longer valid */
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

//! .. _scc_hashmap_init:
//! .. c:function:: void *scc_hashmap_init(keytype, valuetype, scc_hashmap_eq eq)
//!
//!     Initializes a hash map using the default :ref:`Fowler-Noll-Vo <scc_hashmap_fnv1a>`
//!     hash function. Exactly equivalent to calling
//!     :code:`scc_hashmap_init_with_hash(keytype, valuetype, eq, scc_hashmap_fnv1a)`. See
//!     :ref:`its documentation <scc_hashmap_init_with_hash>` for restrictions and common
//!     pitfalls.
//!
//!     The call cannot fail.
//!
//!     The returned pointer must be passed to :ref:`scc_hashmap_free <scc_hashmap_free>` to ensure
//!     allocated memory is reclaimed.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
//!     :param eq: Pointer to function to be used for equality comparison
//!     :returns: Handle to a newly created hash map. The map is allocated in the stack
//!               frame of the current function and its lifetime tied to the scope in
//!               which :c:texpr:`scc_hashmap_init` is invoked.
//!
//!     .. seealso:: :ref:`scc_hashmap_init_with_hash <scc_hashmap_init_with_hash>`
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

//! .. _scc_hashmap_fnv1a:
//! .. c:function:: unsigned long long scc_hashmap_fnv1a(void const *data, size_t size)
//!
//!     Simple `alternative Fowler-Noll-Vo hash
//!     <https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function#FNV-1a_hash>`_
//!     implementation. Used as the default hash function.
//!
//!     :param data: Address of the data to be hashed. The data
//!                  is treated as a consecutive array of bytes.
//!                  Potential padding in structs must therefore
//!                  be explicitly initialized to avoid erratic
//!                  hashing behavior.
//!     :param size: The size of the data to be hashed, in bytes
//!     :returns: The alternative Fowler-Noll-Vo hash of the given :c:texpr:`data`
unsigned long long scc_hashmap_fnv1a(void const *data, size_t size);

//! .. _scc_hashmap_free:
//! .. c:function:: void scc_hashmap_free(void *map)
//!
//!     Reclaim memory used by the given hash map. The :c:texpr:`map`
//!     parameter must refer to a valid hash map handle returned by either
//!     :ref:`scc_hashmap_init <scc_hashmap_init>` or
//!     :ref:`scc_hashmap_init_with_hash <scc_hashmap_init_with_hash>`.
//!
//!     :param map: Handle to the hash map to free
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

//! .. c:function:: _Bool scc_hashmap_insert(void *mapaddr, keytype key, valuetype value)
//!
//!     Insert a key-value pair in the hash map. If :c:texpr:`key` is already present
//!     in the hash map, its associated value is replaced with the given
//!     :c:texpr:`value`. If :c:texpr:`key` is not found in the table, the pair
//!     is inserted.
//!
//!     The call may result in the hash map being reallocated.
//!
//!     The return value does not indicate whether the given :c:texpr:`key` was already
//!     present in the table or not. For determining whether a key is present in the
//!     map, refer to :ref:`scc_hashmap_find <scc_hashmap_find>`.
//!
//!     :param mapaddr: Address of the handle used for referring to the hash map. Should
//!                     the map have to be reallocated to accomodate the insertion, :c:texpr:`*mapaddr`
//!                     is changed to refer to the newly allocated hash map.
//!     :param key: The key used for comparison with ones existing in the map. Must be implicitly
//!                 convertible to the key type for which the table was instantiated.
//!     :param value: The associated value to insert a the slot computed from the :c:texpr:`key`.
//!     :returns: A :c:texpr:`_Bool` indicating whether the insertion was successful.
//!     :retval true: The key-value pair was successfully inserted.
//!     :retval false: Memory allocation failure
//!
//!     .. code-block:: C
//!         :caption: Insert the value 38 with key 1 in a map
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashmap(int, int) map = scc_hashmap_init(int, int, eq);
//!
//!         if(!scc_hashmap_insert(&map, 1, 38)) {
//!             fputs("Insertion failure\n", stderr);
//!             exit(1);
//!         }
//!
//!         /* Use map */
//!
//!         scc_hashmap_free(map);
#define scc_hashmap_insert(mapaddr, key, value)                             \
    scc_hashmap_impl_insert((                                               \
            (*(mapaddr))->hp_key = key,                                     \
            (*(mapaddr))->hp_val = value,                                   \
            (mapaddr)                                                       \
        ),                                                                  \
        sizeof((*(mapaddr))->hp_key),                                       \
        sizeof((*(mapaddr))->hp_val)                                        \
    )

//! .. c:function:: size_t scc_hashmap_capacity(void const *map)
//!
//!     Return the current capacity of the given hash map.
//!
//!     :param map: Handle to the hash map
//!     :returns: The number of elements the hash map is able to
//!               store at the time of the call. Note that the hash
//!               map *will* be reallocated before the capacity is
//!               reached.
inline size_t scc_hashmap_capacity(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_capacity;
}

//! .. c:function:: size_t scc_hashmap_size(void const *map)
//!
//!     Return the current size of the given hash map.
//!     :param map: Handle to the hash map
//!     :returns: The number of elements stored in the hash map at the time of the call.
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

//! .. _scc_hashmap_find:
//! .. c:function:: void *scc_hashmap_find(void *map, keytype key)
//!
//!     Probe for value identified by the given :c:texpr:`key` in the hash map.
//!
//!     :param map: Handle for identifying the hash map
//!     :param key: The :c:texpr:`key` to probe for
//!     :returns: Address of the value associated with the :c:texpr:`key`, if any
//!     :retval NULL: If :c:texpr:`key` was not found in :c:texpr:`map`.
//!     :retval Address of associated value: The :c:texpr:`map` contains :c:texpr:`key`. The
//!             returned element may be modified provided that the :c:texpr:`valuetype` the
//!             map was instantiated with was const :code:`const`-qualified.
//!
//!     .. code-block:: C
//!         :caption: Find and modify value in hash map
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashmap(int, short) map = scc_hashmap_init(int, short, eq);
//!         assert(scc_hashmap_insert(&map, 12, 2345));
//!
//!         short *val = scc_hashmap_find(map, 12);
//!         assert(val);
//!
//!         /* Modify value in hash map */
//!         *val = 1111;
//!
//!         assert(1111 == *scc_hashmap_find(map, 12));
//!
//!         scc_hashmap_free(map);
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

//! .. c:function:: _Bool scc_hashmap_remove(void *map, keytype key)
//!
//!     Remove a key-value pair from the given hash map. Return :c:texpr:`true` if
//!     the key was found while probing.
//!
//!     :param map: Handle for identifying the hash map
//!     :param key: The :c:texpr:`key` to probe for and remove
//!     :returns: Whether the operation was successful
//!     :retval true: :c:texpr:`map` did contain :c:texpr:`key` before the call and
//!                   it has now been removed along with its associated value.
//!     :retval false: :c:texpr:`map` did not contain :c:texpr:`key` at the time of the
//!                    call.
//!
//!     .. code-block:: C
//!         :caption: Insert and immediately remove a key-value pair
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashmap(int, short) map = scc_hashmap_init(int, short, eq);
//!
//!         assert(scc_hashmap_insert(&map, 12, 2345));
//!         assert(scc_hashmap_remove(map, 12));
//!         assert(!scc_hashmap_find(map, 12));
//!
//!         scc_hashmap_free(map);
#define scc_hashmap_remove(map, key)                                    \
    scc_hashmap_impl_remove(((map)->hp_key = (key), (map)), sizeof((map)->hp_key))

//! .. c:function:: void scc_hashmap_clear(void *map)
//!
//!     Clear all entries in the given hash map.
//!
//!     :param map: Handle for identifying the hash map
void scc_hashmap_clear(void *map);

#endif /* SCC_HASHMAP_H */
