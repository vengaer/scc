#ifndef SCC_HASHMAP_H
#define SCC_HASHMAP_H

#include "arch.h"
#include "bits.h"
#include "bug.h"
#include "canary.h"
#include "config.h"
#include "hash.h"
#include "mem.h"
#include "perf.h"

#include <stddef.h>

//? .. _hashmap_impl_pair:
//? .. c:macro:: scc_hashmap_impl_pair(keytype, valuetype)
//?
//?     Expands to a struct appropriate for storing pairs of
//?     the supplied key-value types
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The key type of the hashmap
//?     :param valuetype: The value type of the hashmap
#define scc_hashmap_impl_pair(keytype, valuetype)                                       \
    struct { keytype hp_key; valuetype hp_val; }

//? .. c:macro:: scc_hashmap_impl_pair_valoff(keytype, valuetype)
//?
//?     Compute offset of the value field in the :ref:`pair struct <hashmap_impl_pair>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The key type of the hashmap
//?     :param valuetype: The value type of the hashmap
#define scc_hashmap_impl_pair_valoff(keytype, valuetype)                                \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

//! .. c:macro:: scc_hashmap(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for storing a handle to
//!     a hash map holding instances of the given key- and value types.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
#define scc_hashmap(keytype, valuetype)                                                 \
    scc_hashmap_impl_pair(keytype, valuetype) *

//? .. c:macro:: SCC_HASHMAP_GUARDSZ
//?
//?     Size of the vector guard placed after the
//?     :ref:`metadata array <scc_hashmap_metatype_hm_meta>`.
//?
//?     .. note::
//?
//?         Internal use only
#define SCC_HASHMAP_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

//? .. c:macro:: SCC_HASHMAP_CANARYSZ
//?
//?     Size of the stack canary placed after the
//?     metadata guard
//?
//?     .. note::
//?
//?         Internal use only
#define SCC_HASHMAP_CANARYSZ 32u

//? .. c:macro:: SCC_HASHMAP_DUPLICATE
//?
//?     Bit set when encountering an existing key during
//?     insertion
//?
//?     .. note::
//?
//?         Internal use only
#define SCC_HASHMAP_DUPLICATE (~(~0ull >> 1u))

#ifndef SCC_HASHMAP_STACKCAP
//! .. c:enumerator:: SCC_HASHMAP_STACKCAP
//!
//!     Capacity of the on-stack buffer used initially by the
//!     hash map. The value may be overridden by defining it before
//!     including the header
//!
//!     Must be a power of 2 and >= 32
//!
//!     .. note::
//!
//!         Internal use only
enum { SCC_HASHMAP_STACKCAP = 32 };
#endif

scc_static_assert(SCC_HASHMAP_STACKCAP >= 32);
scc_static_assert(scc_bits_is_power_of_2(SCC_HASHMAP_STACKCAP));

//! .. c:type:: _Bool(*scc_hashmap_eq)(void const *, void const *)
//!
//!     Signature of the function used for equality comparisons.
typedef _Bool(*scc_hashmap_eq)(void const *, void const *);

//! .. c:type:: unsigned long long(*scc_hashmap_hash)(void const *, size_t)
//!
//!     Signature of the hash function used.
typedef unsigned long long(*scc_hashmap_hash)(void const *, size_t);

//? .. c:type:: unsigned char scc_hashmap_metatype
//?
//?     Type used for storing hash map metadata
//?
//?     .. note::
//?         Internal use only
typedef unsigned char scc_hashmap_metatype;

//? .. c:struct:: scc_hashmap_perfevts
//?
//?     Counters for tracking performance-related events
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:var:: size_t ev_n_rehashes
//?
//?         Number of times the hash map has been rehashed
//?
//?     .. c:var:: size_t ev_n_eqs
//?
//?         Number of calls to eq performed
//?
//?     .. c:var:: size_t ev_n_hash
//?
//?         Number of calls to hash performed
//?
//?     .. c:var:: size_t ev_n_inserts
//?
//?         Number of successful insertions performed
//?
//?     .. c:var:: size_t ev_bytesz
//?
//?         Total size of the map, in bytes
struct scc_hashmap_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

//? .. _scc_hashmap_base:
//? .. c:struct:: scc_hashmap_base
//?
//?     Base struct of the hash map. Exposed only indirectly
//?     through fat pointers. Given a :code:`struct scc_hashmap_base`,
//?     the address of said pointer is obtained by computing
//?     :c:texpr:`&base.hm_fwoff + base.hw_fwoff + sizeof(base.hw_fwoff)`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _scc_hashmap_eq_hm_eq:
//?     .. c:var:: scc_hashmap_eq hm_eq
//?
//?         Pointer to function used for equalify comparison
//?
//?     .. _scc_hashmap_hash_hm_hash:
//?     .. c:var:: scc_hashmap_hash hm_hash
//?
//?         Pointer to hash function
//?
//?     .. _size_t_hm_valoff:
//?     .. c:var:: size_t hm_valoff
//?
//?         Offset of value array relative the base address. Used
//?         to access the values in the FAM
//?
//?     .. _size_t_hm_mdoff:
//?     .. c:var:: size_t hm_mdoff
//?
//?         Offset of metadata array relative the base address. Used
//?         to access the metadata in the FAM.
//?
//?     .. _size_t_hm_size:
//?     .. c:var:: size_t hm_size
//?
//?         Size of the hash map
//?
//?     .. _size_t_hm_capacity:
//?     .. c:var:: size_t hm_capacity
//?
//?         Capacity of the map. Always a power of 2.
//?
//?     .. _size_t_hm_pairsize:
//?     .. c:var:: size_t hm_pairsize
//?
//?         Size of the key-value pair at the address of the handle
//?
//?     .. _struct_scc_hashmap_perfevts_hm_perf:
//?     .. c:var:: struct scc_hashmap_perfevts hm_perf
//?
//?         Performance counters. Present only if CONFIG_PERFEVENTS
//?         is defined
//?
//?     .. _unsigned_short_hm_keyalign:
//?     .. c:var:: unsigned short hm_keyalign
//?
//?         Key type alignment
//?
//?     .. _unsigned_short_hm_valalign:
//?     .. c:var:: unsigned short hm_valalign
//?
//?         Value type alignment
//?
//?     .. _unsigned_char_hm_dynalloc:
//?     .. c:var:: unsigned char hm_dynalloc
//?
//?         Holds the value 1 if the map was allocated dynamically.
//?         Set at first rehash as the map is initially allocated on the
//?         stack
//?
//?     .. _unsigned_char_hm_valpad:
//?     .. c:var:: unsigned char hm_valpad
//?
//?         Tracks the number of padding bytes between the key and value in the
//?         internal pair struct
//?
//?     .. _unsigned_char_hm_fwoff:
//?     .. c:var:: unsigned char hm_fwoff
//?
//?         Offset of the pointer exposed through the API relative the field
//?         itself
//?
//?     .. _unsigned_char_hm_buffer:
//?     .. c:var:: unsigned char hm_buffer[]
//?
//?         FAM hiding type-specific details. For exact layout, refer to
//?         :ref:`scc_hashmap_impl_layout <scc_hashmap_impl_layout>`.
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

//? .. _scc_hashmap_impl_layout:
//? .. c:macro:: scc_hashmap_impl_layout(keytype, valuetype)
//?
//?     The actual layout of the hash map instantiated for a given
//?     key-value type pair. The hm_eq through hm_fwoff members are
//?     the same as for :ref:`struct scc_hashmap_base <scc_hashmap_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: Key type of the hash map
//?     :param valuetype: Value type of the hash map
//?
//?     .. c:struct:: @layout
//?
//?         .. c:var:: scc_hashmap_eq hm_eq
//?
//?             See :ref:`hm_eq <scc_hashmap_eq_hm_eq>`.
//?
//?         .. c:var:: scc_hashmap_hash hm_hash
//?
//?             See :ref:`hm_hash <scc_hashmap_hash_hm_hash>`.
//?
//?         .. c:var:: size_t hm_valoff
//?
//?             See :ref:`hm_valoff <size_t_hm_valoff>`.
//?
//?         .. c:var:: size_t hm_mdoff
//?
//?             See :ref:`hm_mdoff <size_t_hm_mdoff>`.
//?
//?         .. c:var:: size_t hm_size
//?
//?             See :ref:`hm_size <size_t_hm_size>`.
//?
//?         .. c:var:: size_t hm_capacity
//?
//?             See :ref:`hm_capacity <size_t_hm_capacity>`.
//?
//?         .. c:var:: size_t hm_pairsize
//?
//?             See :ref:`hm_pairsize <size_t_hm_pairsize>`.
//?
//?         .. c:var:: struct scc_hashmap_perfevts hm_perf
//?
//?             See :ref:`hm_perf <struct_scc_hashmap_perfevts_hm_perf>`.
//?
//?         .. c:var:: unsigned short hm_keyalign
//?
//?             See :ref:`hm_keyalign <unsigned_short_hm_keyalign>`..
//?
//?         .. c:var:: unsigned short hm_valalign
//?
//?             See :ref:`hm_valalign <unsigned_short_hm_valalign>`.
//?
//?         .. c:var:: unsigned char hm_dynalloc
//?
//?             See :ref:`hm_dynalloc <unsigned_char_hm_dynalloc>`.
//?
//?         .. c:var:: unsigned char hm_valpad
//?
//?             See :ref:`hm_valpad <unsigned_char_hm_valpad>`
//?
//?         .. c:var:: unsigned char hm_fwoff
//?
//?             See :ref:`hm_fwoff <unsigned_char_hm_fwoff>`.
//?
//?         .. _unsigned_char_hm_bkoff:
//?         .. c:var:: unsigned char hm_bkoff
//?
//?             Field used for tracking the padding between :code:`hm_fwoff`
//?             and :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>`. Just as :code:`hm_fwoff`
//?             is used to compute the address of :code:`hm_curr` given the
//?             base address, :code:`hm_bkoff` is used to compute the base
//?             address given the address of :code:`hm_curr`:
//?
//?             The primary purpose of this field is to force injection of
//?             padding bytes between :code:`hm_fwoff` and :code:`hm_curr`.
//?             Whether the offset is actually stored in this field depends
//?             on the alignment of the :code:`type` parameter. In practice,
//?             this occurs only for 1-byte aligned types. For types with
//?             stricter alignment requirements, the last padding byte is
//?             used instead.
//?
//?
//?         .. _scc_hashmap_impl_pair_hm_curr:
//?         .. c:var:: kvpair hm_curr
//?
//?             Where :code:`struct kvpair` is defined as
//?             :code:`typedef scc_hashmap_impl_pair(keytype, valuetype) kvpair`.
//?
//?             Volatile instance of the pair stored in the hash map. Used
//?             for temporary storage of values to be inserted or probed for
//?             in order to provide rvalue support.
//?
//?         .. c:var:: keytype hm_keys[SCC_HASHMAP_STACKCAP]
//?
//?             The key array.
//?
//?         .. c:var:: valuetype hm_vals[SCC_HASHMAP_STACKCAP]
//?
//?             The value array.
//?
//?         .. _scc_hashmap_metatype_hm_meta:
//?         .. c:var:: scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP]
//?
//?             Metadata array used for tracking vacant slots. A slot at index :code:`n`
//?             in the metadata array corresponds to slot :code:`n` in the key and value
//?             arrays.
//?
//?             A slot is unused if the value of its metadata entry is 0. If the byte has
//?             a non-zero value with the MSB unset, the slot was previously
//?             occupied but has since been vacated. Such slots are reused during insertion
//?             but must not serve as probing stops.
//?
//?             An entry with the MSB set signifies that the slot is occupied. In this case,
//?             the remaining bites are the :c:texpr:`CHAR_BIT - 1` most significant bits
//?             of the hash computed for the value in the corresponding slot in the key array.
//?             This is used for optimizing away unnecessary calls to
//?             :ref:`eq <scc_hashmap_eq_hm_eq>`
//?
//?         .. c:var:: scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ]
//?
//?             Guard to allow for unaligned vector loads without risking reads from
//?             potential guard pages. The :code:`SCC_HASHMAP_GUARDSZ` low bytes of the
//?             :code:`hm_meta` field are mirrored in the guard.
#define scc_hashmap_impl_layout(keytype, valuetype)                                         \
    struct {                                                                                \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                        \
        } hm2;                                                                              \
        scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP];                                 \
        scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ];                                 \
        SCC_CANARY_INJECT(SCC_HASHMAP_CANARYSZ)                                             \
    }

//? .. c:macro:: scc_hashmap_impl_curroff(keytype, valuetype)
//?
//?     Compute offset of :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: Key type of the hash map
//?     :param valuetype: Value type of the hash map
#define scc_hashmap_impl_curroff(keytype, valuetype)                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                scc_hashmap_eq hm_eq;                                                       \
                scc_hashmap_hash hm_hash;                                                   \
                size_t hm_valoff;                                                           \
                size_t hm_mdoff;                                                            \
                size_t hm_size;                                                             \
                size_t hm_capacity;                                                         \
                size_t hm_pairsize;                                                         \
                SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                        \
                unsigned short hm_keyalign;                                                 \
                unsigned short hm_valalign;                                                 \
                unsigned char hm_dynalloc;                                                  \
                unsigned char hm_valpad;                                                    \
                unsigned char hm_fwoff;                                                     \
                unsigned char hm_bkoff;                                                     \
            } hm0;                                                                          \
            scc_hashmap_impl_pair(keytype, valuetype) hm_curr[];                            \
        }                                                                                   \
    )

//? .. c:macro:: scc_hashmap_impl_valoff(keytype, valuetype)
//?
//?     Compute offset of the value array.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: Key type of the hash map
//?     :param valuetype: Value type of the hash map
#define scc_hashmap_impl_valoff(keytype, valuetype)                                         \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[];                                                            \
        }                                                                                   \
    )

//? .. c:macro:: scc_hashmap_impl_mdoff(keytype, valuetype)
//?
//?     Compute offset of the metadata array.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: Key type of the hash map
//?     :param valuetype: Value type of the hash map
#define scc_hashmap_impl_mdoff(keytype, valuetype)                                          \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    struct {                                                                \
                        scc_hashmap_eq hm_eq;                                               \
                        scc_hashmap_hash hm_hash;                                           \
                        size_t hm_valoff;                                                   \
                        size_t hm_mdoff;                                                    \
                        size_t hm_size;                                                     \
                        size_t hm_capacity;                                                 \
                        size_t hm_pairsize;                                                 \
                        SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                \
                        unsigned short hm_keyalign;                                         \
                        unsigned short hm_valalign;                                         \
                        unsigned char hm_dynalloc;                                          \
                        unsigned char hm_valpad;                                            \
                        unsigned char hm_fwoff;                                             \
                        unsigned char hm_bkoff;                                             \
                    } hm0;                                                                  \
                    scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                      \
                    keytype hm_keys[SCC_HASHMAP_STACKCAP];                                  \
                } hm1;                                                                      \
                valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                    \
            } hm2;                                                                          \
            scc_hashmap_metatype hm_meta[];                                                 \
        }                                                                                   \
    )

//? .. c:function:: void *scc_hashmap_impl_new(\
//?        struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize)
//?
//?     Initialize an empty hash map and return a handle to it.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash map. The handle returned by the function
//?                  refers to the :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>` field
//?                  in this map.
//?     :param coff: Base-relative offset of :code:`hm_curr`.
//?     :param valoff: Internal offset of the value type in the key-value pair
//?     :param keysize: Size of the key type
//?     :returns: A handle to the initialized hash map
void *scc_hashmap_impl_new(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize);

//? .. c:function:: void *scc_hashmap_impl_new_dyn(scc_hashmap_eq eq, scc_hashmap_hash hash, \
//?     size_t mapsize, size_t coff, size_t valoff, size_t keysize)
//?
//?     Like :ref:`scc_hashmap_impl_new <scc_hashtmap_impl_new>` except for the
//?     hash map being allocated on the heap.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param sbase: Address of a ``struct scc_hahsmap_base`` used for parameter passing
//?     :param mapsize: Size of the map
//?     :param coff: Base-relative offset of :code:`hm_curr`.
//?     :param valoff: Internal offset of the value type in the key-value pair
//?     :param keysize: Size of the key type
//?     :returns: A handle to an initialized hash map allocated on the heap
void *scc_hashmap_impl_new_dyn(struct scc_hashmap_base const *sbase, size_t mapsize, size_t coff, size_t valoff, size_t keysize);

//! .. _scc_hashmap_with_hash:
//! .. c:function:: void *scc_hashmap_with_hash(keytype, valuetype, scc_hashmap_eq eq, scc_hashmap_hash hash)
//!
//!     Initializes a hash map using the specified hash function. The macro expands
//!     to a handle to a hash map with automatic storage duration. See
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes>` for an in-depth explanation of how to
//!     safely manage the handle.
//!
//!     The call cannot fail.
//!
//!     The returned pointer must be passed to :ref:`scc_hashmap_free <scc_hashmap_free>` to ensure
//!     allocated memory is reclaimed.
//!
//!     .. seealso::
//!
//!         :ref:`scc_hashmap_with_hash_dyn <scc_hashmap_with_hash_dyn>` for a dynamically allocated
//!         ``hashmap``.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
//!     :param eq: Pointer to function to be used for equality comparison
//!     :param hash: Pointer to function to be used for key hashing
//!     :returns: Handle to a newly created hash map. The map is allocated in the stack
//!               frame of the current function and its lifetime tied to the scope in
//!               which :c:texpr:`scc_hashmap_with_hash` is invoked.
//!
//!     .. code-block:: C
//!         :caption: Example usage
//!
//!         extern _Bool eq(void const *l, void const *r);
//!         extern unsigned long long hash(void const *data, size_t size);
//!
//!         scc_hashmap(int, _Bool) map;
//!         {
//!             map = scc_hashmap_with_hash(int, _Bool, eq, hash);
//!             /* map is valid */
//!         }
//!         /* map is no longer valid */
#define scc_hashmap_with_hash(keytype, valuetype, eq, hash)                                 \
    scc_hashmap_impl_new(                                                                   \
        (void *)&(scc_hashmap_impl_layout(keytype, valuetype)){                             \
            .hm2 = {                                                                        \
                .hm1 = {                                                                    \
                    .hm0 = {                                                                \
                        .hm_eq = eq,                                                        \
                        .hm_hash = hash,                                                    \
                        .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),           \
                        .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),             \
                        .hm_capacity = SCC_HASHMAP_STACKCAP,                                \
                        .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),   \
                        .hm_keyalign = scc_alignof(keytype),                                \
                        .hm_valalign = scc_alignof(valuetype)                               \
                    },                                                                      \
                },                                                                          \
            },                                                                              \
        },                                                                                  \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

//! .. _scc_hashmap_with_hash_dyn:
//! .. c:function:: void *scc_hashmap_with_hash_dyn(keytype, valuetype, scc_hashmap_eq eq, scc_hashmap_hash hash)
//!
//!     Like :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>` except for
//!     the table being allocated on the heap rather than on the stack.
//!
//!     .. note::
//!
//!         Unlike :ref:`scc_hashmap_with_hash`, calls to ``scc_hashmap_with_hash_dyn`` may fail.
//!         The returned pointer should always be ``NULL`` checked.
//!
//!     :param keytype: Type of the keys to be stored in the map
//!     :param valuetype: Type of the values to be stored in the map
//!     :param eq: Pointer to function to be used for key comparison
//!     :param hash: Pointer to function to be used for key hashing
//!     :returns: Handle to a newly created hash map, or ``NULL`` on
//!               allocation failure
#define scc_hashmap_with_hash_dyn(keytype, valuetype, eq, hash)                             \
    scc_hashmap_impl_new_dyn(                                                               \
        (void *)&(struct scc_hashmap_base){                                                 \
            .hm_eq = eq,                                                                    \
            .hm_hash = hash,                                                                \
            .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),                       \
            .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),                         \
            .hm_capacity = SCC_HASHMAP_STACKCAP,                                            \
            .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),               \
            .hm_keyalign = scc_alignof(keytype),                                            \
            .hm_valalign = scc_alignof(valuetype)                                           \
        },                                                                                  \
        sizeof(scc_hashmap_impl_layout(keytype, valuetype)),                                \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

//! .. _scc_hashmap_new:
//! .. c:function:: void *scc_hashmap_new(keytype, valuetype, scc_hashmap_eq eq)
//!
//!     Initializes a hash map using the default :ref:`Fowler-Noll-Vo <scc_hash_fnv1a>`
//!     hash function. Exactly equivalent to calling
//!     :code:`scc_hashmap_with_hash(keytype, valuetype, eq, scc_hash_fnv1a)`. See
//!     :ref:`its documentation <scc_hashmap_with_hash>` for restrictions and common
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
//!               which :c:texpr:`scc_hashmap_new` is invoked.
//!
//!     .. seealso::
//!
//!         :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>`
#define scc_hashmap_new(keytype, valuetype, eq)                                            \
    scc_hashmap_with_hash(keytype, valuetype, eq, scc_hash_fnv1a)

//! .. _scc_hashmap_new_dyn:
//! .. c:function:: void *scc_hashmap_new_dyn(keytype, valuetype, scc_hashmap_eq eq)
//!
//!     Like :ref:`scc_hashmap_new <scc_hashmap_new>` except for the hash
//!     map being allocated on the heap rather than on the stack.
//!
//!     .. note::
//!
//!         Unlike :ref:`scc_hashmap_new <scc_hashmap_new>`, calls to ``scc_hashmap_new_dyn``
//!         may fail. The returned pointer should always be ``NULL``-checked.
//!
//!     :param keytype: Type of the keys to be stored in the table
//!     :param valuetype: Type of the values to be stored in the table
//!     :param eq: Pointer to function to be used for equality comparison
//!     :returns: Handle to a newly created hash map, or ``NULL`` on allocation failure
#define scc_hashmap_new_dyn(keytype, valuetype, eq)                                       \
    scc_hashmap_with_hash_dyn(keytype, valuetype, eq, scc_hash_fnv1a)

//? .. c:function:: size_t scc_hashmap_impl_bkpad(void const *map)
//?
//?     Compute number of padding bytes between the
//?     :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>` and
//?     :ref:`hm_fwoff <unsigned_char_hm_fwoff>` fields in the given map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Hash map handle
//?     :returns: Number of padding bytes between the :code:`hm_curr` and
//?               :code:`hm_fwoff` fields in the map.
inline size_t scc_hashmap_impl_bkpad(void const *map) {
    return ((unsigned char const *)map)[-1] + sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
}

//? .. c:macro:: scc_hashmap_impl_base_qual(map, qual)
//?
//?     Obtain qualified pointer to the
//?     :ref:`struct scc_hashmap_base <scc_hashmap_base>` corresponding
//?     to the given map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Hash map handle
//?     :param qual: Qualifiers to apply to the pointer
//?     :returns: Qualified pointer to the base struct of the map
#define scc_hashmap_impl_base_qual(map, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(map) - scc_hashmap_impl_bkpad(map),          \
        struct scc_hashmap_base,                                            \
        hm_fwoff,                                                           \
        qual                                                                \
    )

//? .. c:macro:: scc_hashmap_impl_base(map)
//?
//?     Obtain unqualified pointer to the
//?     :ref:`struct scc_hashmap_base <scc_hashmap_base>` corresponding
//?     to the given map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Hash map handle
//?     :returns: Unqualified pointer to the base struct of the given map
#define scc_hashmap_impl_base(map)                                          \
    scc_hashmap_impl_base_qual(map,)

//! .. _scc_hashmap_free:
//! .. c:function:: void scc_hashmap_free(void *map)
//!
//!     Reclaim memory used by the given hash map. The :c:texpr:`map`
//!     parameter must refer to a valid hash map handle returned by either
//!     :ref:`scc_hashmap_new <scc_hashmap_new>` or
//!     :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>`.
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
//? .. c:function:: _Bool scc_hashmap_impl_insert(\
//?        void *mapaddr, size_t keysize, size_t valsize)
//?
//?     Attempt to insert the key-value pair in
//?     :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>` in the map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param mapaddr: Address of the handle to the hash map to insert in
//?     :param keysize: Size of the key type
//?     :param valsize: Size of the value type
//?     :returns: A :code:`_Bool` indicating whether the insertion was successful
//?               or not.
//?     :retval true: The key-value pair was successfully inserted
//?     :retval false: Memory allocation failure
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
//!         scc_hashmap(int, int) map = scc_hashmap_new(int, int, eq);
//!
//!         if(!scc_hashmap_insert(&map, 1, 38)) {
//!             fputs("Insertion failure\n", stderr);
//!             exit(EXIT_FAILURE);
//!         }
//!
//!         /* Use map */
//!
//!         scc_hashmap_free(map);
#define scc_hashmap_insert(mapaddr, key, value)                             \
    scc_hashmap_impl_insert((                                               \
            (*(mapaddr))->hp_key = (key),                                   \
            (*(mapaddr))->hp_val = (value),                                 \
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
//!         scc_hashmap(int, short) map = scc_hashmap_new(int, short, eq);
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
//!         scc_hashmap(int, short) map = scc_hashmap_new(int, short, eq);
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

//! .. c:function:: void *scc_hashmap_clone(void const *map)
//!
//!     Clone the given hashmap, returning a new instance containing
//!     the same key-value pairs. The new instance is allocated on the
//!     heap
//!
//!     :param map: The ``hashmap`` to clone
//!     :returns: Handle to a new ``hashmap``, or ``NULL`` on failure
void *scc_hashmap_clone(void const *map);

#endif /* SCC_HASHMAP_H */
