#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include "scc_mem.h"

#include <stddef.h>

//! .. c:macro:: scc_hashtab(type)
//!
//!     Expands to an opaque pointer suitable for storing a handle to
//!     a hash tab holding instances of the given type
//!
//!     :param type: Type of the elements to be stored in the table
#define scc_hashtab(type) type *

#define SCC_HASHTAB_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

enum { SCC_HASHTAB_STACKCAP = 32 };

//! .. c:type:: _Bool(*scc_hashtab_eq)(void const *, void const *)
//!
//!     Signature of the function used for equality comparisons.
typedef _Bool(*scc_hashtab_eq)(void const *, void const *);

//! .. c:type:: unsigned long long(*scc_hashtab_hash)(void const *, size_t)
//!
//!     Signature of the hash function used.
typedef unsigned long long(*scc_hashtab_hash)(void const*, size_t);

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
 * obtained by computing &base.ht_fwoff + base.ht_fwoff + 1.
 *
 * scc_hashtab_eq ht_eq;
 *      Pointer to function used for equality comparison
 *
 * scc_hashtab_hash ht_hash
 *      Pointer to hash function.
 *
 * size_t ht_mdoff
 *      Offset of metadata array relative base address. This
 *      is used to access the metadata in the FAM part of the
 *      struct.
 *
 * size_t ht_size
 *      Size of the hash table
 *
 * size_t ht_capacity
 *      Capacity of the hash table. Always a power of two to
 *      allow for efficient modulo computation.
 *
 * struct scc_hashtab_perfevts ht_perf
 *      Performance counters
 *
 * unsigned char ht_dynalloc
 *      Set to 1 if the current table was allocated dynamically.
 *      Upon initial construction, the table is allocated on the
 *      stack. This field is set on the first rehash.
 *
 * unsigned char ht_fwoff
 *      Offset of the pointer exposed through the API. The offset
 *      is relative to the field itself.
 *
 * unsigned char ht_buffer[]
 *      FAM hiding type-specific details. For the exact layout,
 *      refer to scc_hashtab_impl_layout.
 */
struct scc_hashtab_base {
    scc_hashtab_eq ht_eq;
    scc_hashtab_hash ht_hash;
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

/* scc_hashtab_impl_layout
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
        scc_hashtab_eq ht_eq;                                               \
        scc_hashtab_hash ht_hash;                                           \
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
 * scc_hashtab_eq eq
 *      Pointer to the equality function to use
 *
 * scc_hashtab_hash hash
 *      Pointer to the hash function to use
 *
 * size_t coff
 *      Offset of ht_curr relative the address of *base
 *
 * size_t mdoff
 *      Offset of ht_meta relative the address of *base
 */
void *scc_hashtab_impl_init(struct scc_hashtab_base *base, scc_hashtab_eq eq, scc_hashtab_hash hash, size_t coff, size_t mdoff);

//! .. _scc_hashtab_init_with_hash:
//! .. c:function:: void *scc_hashtab_init_with_hash(type, scc_hashtab_eq eq, scc_hashtab_hash hash)
//!
//!     Initializes a hash table using the specified hash function. The macro expands
//!     to a handle to a table with automatic storage duration. See
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes>` for an in-depth explanation of how to
//!     safely manage the handle.
//!
//!     The call cannot fail.
//!
//!     The returned pointer must be passed to :ref:`scc_hashtab_free <scc_hashtab_free>` to ensure
//!     allocated memory is reclaimed.
//!
//!     :param type: Type of the elements to be stored in the table
//!     :param eq: Pointer to function to be used for equality comparison
//!     :param hash: Pointer to function to be used for key hashing
//!     :returns: Handle to a newly created hash table. The table is allocated in the stack
//!               frame of the current function and its lifetime tied to the scope in
//!               which :c:texpr:`scc_hashtab_init_with_hash` is invoked.
//!
//!     .. code-block:: C
//!         :caption: Example usage
//!
//!         extern _Bool eq(void const *l, void const *r);
//!         extern unsigned long long hash(void const *data, size_t size);
//!
//!         scc_hashtab(int) tab;
//!         {
//!             tab = scc_hashtab_init_with_hash(int, eq, hash);
//!             /* tab is valid */
//!         }
//!         /* tab is no longer valid */
#define scc_hashtab_init_with_hash(type, eq, hash)                          \
    scc_hashtab_impl_init(                                                  \
        (void *)&(scc_hashtab_impl_layout(type)){ 0 },                      \
        eq,                                                                 \
        hash,                                                               \
        offsetof(scc_hashtab_impl_layout(type), ht_curr),                   \
        offsetof(scc_hashtab_impl_layout(type), ht_meta)                    \
    )

//! .. _scc_hashtab_init:
//! .. c:function:: void *scc_hashtab_init(type, scc_hashtab_eq eq)
//!
//!     Initializes a hash table using the default :ref:`Fowler-Noll-Vo <scc_hashtab_fnv1a>`
//!     hash function. Exactly equivalent to calling
//!     :code:`scc_hashtab_init_with_hash(keytype, valuetype, eq, scc_hashtab_fnv1a)`. See
//!     :ref:`its documentation <scc_hashtab_init_with_hash>` for restrictions and common
//!     pitfalls.
//!
//!     The call cannot fail.
//!
//!     The returned pointer must be passed to :ref:`scc_hashtab_free <scc_hashtab_free>` to ensure
//!     allocated memory is reclaimed.
//!
//!     :param type: Type of the elements to be stored in the table
//!     :param eq: Pointer to function to be used for equality comparison
//!     :returns: Handle to a newly created hash table. The table is allocated in the stack
//!               frame of the current function and its lifetime tied to the scope in
//!               which :c:texpr:`scc_hashtab_init` is invoked.
//!
//!     .. seealso:: :ref:`scc_hashtab_init_with_hash <scc_hashtab_init_with_hash>`
#define scc_hashtab_init(type, eq)                                          \
    scc_hashtab_init_with_hash(type, eq, scc_hashtab_fnv1a)

/* scc_hashtab_impl_bkpad
 *
 * Internal use only
 *
 * Compute number of padding bytes between ht_curr and ht_fwoff
 */
inline size_t scc_hashtab_impl_bkpad(void const *tab) {
    return ((unsigned char const *)tab)[-1] + sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
}

/* scc_hashtab_impl_base_qual
 *
 * Internal use only
 *
 * Obtain qualified pointer to the struct scc_hashtab_base
 * corresponding to the given handle
 */
#define scc_hashtab_impl_base_qual(tab, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(tab) - scc_hashtab_impl_bkpad(tab),          \
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
#define scc_hashtab_impl_base(tab)                                          \
    scc_hashtab_impl_base_qual(tab,)

//! .. _scc_hashtab_fnv1a:
//! .. c:function:: unsigned long long scc_hashtab_fnv1a(void const *data, size_t size)
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
unsigned long long scc_hashtab_fnv1a(void const *data, size_t size);

//! .. _scc_hashtab_free:
//! .. c:function:: void scc_hashtab_free(void *tab)
//!
//!     Reclaim memory used by the given hash table. The :c:texpr:`tab`
//!     parameter must refer to a valid hash table handle returned by either
//!     :ref:`scc_hashtab_init <scc_hashtab_init>` or
//!     :ref:`scc_hashtab_init_with_hash <scc_hashtab_init_with_hash>`.
//!
//!     :param tab: Handle to the hash table to free
void scc_hashtab_free(void *tab);

/* scc_hashtab_impl_insert
 *
 * Internal use only
 *
 * Insert the value in ht_curr in the table. Return true
 * on success.
 *
 * void *tabaddr
 *      Address of the handle used to refer to the hash table
 *
 * size_t elemsize
 *      Size of the elements stored in the hash table
 */
_Bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize);

//! .. c:function:: _Bool scc_hashtab_insert(void *tabaddr, type element)
//!
//!     Insert an element in the hash table. Succeeds only if  :c:texpr:`element`
//!     is not already present in the table.
//!
//!     The call may result in the hash table being reallocated.
//!
//!     :param tabaddr: Address of the handle used for referring to the hash table. Should
//!                     the table have to be reallocated to accomodate the insertion, :c:texpr:`*tabaddr`
//!                     is changed to refer to the newly allocated hash table.
//!     :param element: The element to be inserted. Must be implicitly convertible to
//!                     the type with which the table was instantiated.
//!     :returns: A :c:texpr:`_Bool` indicating whether the insertion was successful.
//!     :retval true: The element was successfully inserted.
//!     :retval false: :c:texpr:`element` already present in table.
//!     :retval false: Memory allocation failure
//!
//!     .. code-block:: C
//!         :caption: Insert the value 111 in a table if ints
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashtab(int) tab = scc_hash_init(int, eq);
//!
//!         if(!scc_hashtab_insert(&tab, 111)) {
//!             fputs("Insertion failure\n", stderr);
//!             exit(1);
//!         }
//!
//!         /* Use tab */
//!
//!         scc_hashtab_free(tab);
#define scc_hashtab_insert(tabaddr, value)                                  \
    scc_hashtab_impl_insert((**(tabaddr) = (value), (tabaddr)), sizeof(**(tabaddr)))

//! .. c:function:: size_t scc_hashtab_capacity(void const *tab)
//!
//!     Return the current capacity of the given hash table.
//!
//!     :param tab: Handle to the hash table
//!     :returns: The number of elements the hash table is able to
//!               store at the time of the call. Note that the hash
//!               table *will* be reallocated before the capacity is
//!               reached.
inline size_t scc_hashtab_capacity(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_capacity;
}

//! .. c:function:: size_t scc_hashtab_size(void const *tab)
//!
//!     Return the current size of the given hash table.
//!     :param tab: Handle to the hash table
//!     :returns: The number of elements stored in the hash table at the time of the call.
inline size_t scc_hashtab_size(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_size;
}

/* scc_hashtab_impl_find
 *
 * Internal use only
 *
 * Probe for the value in *tab in the table. Return a pointer to
 * the element if found, NULL otherwise
 *
 * void const *tab
 *      Handle to the hash table in question
 *
 * size_t elemsize
 *      Size of the elements stored in the hash table
 */
void const *scc_hashtab_impl_find(void const *tab, size_t elemsize);

//! .. c:function:: void *scc_hashtab_find(void *tab, type value)
//!
//!     Probe for :c:texpr:`value` in the given hash table.
//!
//!     :param tab: Handle for identifying the hash table
//!     :param value: The element to probe for
//!     :returns: :code:`const`-qualified pointer to the element found in the table, if any.
//!     :retval NULL: If :c:texpr:`value` was not found in :c:texpr:`tab`.
//!     :retval Address of element in table: The :c:texpr:`table` contains :c:texpr:`value`.
//!
//!     .. code-block:: C
//!         :caption: Check if a hash table contains a specific string literal
//!
//!         extern _Bool eq(void const *l, void const *r);
//!         char const *str = "a waste of time";
//!
//!         scc_hashtab(char const *) tab = scc_hashtab_init(char const *, eq);
//!         assert(scc_hashtab_insert(&tab, str));
//!
//!         /* Note: Probing is based on address of string literal
//!          * rather than strcmp here. This is safe but error prone */
//!         char const *found = scc_hashtab_find(tab, str);
//!         assert(!strcmp(found, str));
//!
//!         scc_hashtab_free(tab);
#define scc_hashtab_find(tab, value)                                        \
    scc_hashtab_impl_find((*(tab) = (value), (tab)), sizeof(*(tab)))

/* scc_hashtab_impl_reserve
 *
 * Internal use only
 *
 * Reserve storage for specified number of slots. Return true on success,
 * otherwise false. On failure, *(void **)tabaddr remains untouched.
 *
 * void *tabaddr
 *      Address of the tabaddr used for referring to the hash table
 *
 * size_t capacity
 *      The desired capacity. If it is not a power of 2, it is
 *      rounded up to the next such power.
 */
_Bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize);

//! .. c:function:: _Bool scc_hashtab_reserve(void *tabaddr, size_t capacity)
//!
//!     Reserve storage for at least :c:texpr:`capacity` elements in the table.
//!     If the table is successfully reallocated, :c:texpr:`*(void **)tabaddr` is updated
//!     to refer to the newly allocate table. If the reallocation fails, :c:texpr:`*(void **)tabaddr`
//!     is left untouched.
//!
//!     :param tabaddr: Address of the handle used to refer to the hash table
//!     :param capacity: The desired capacity
//!     :returns: A :c:texpr:`_Bool` indicating whether the demand could be carried out
//!     :retval true: The table capacity was already sufficiently large. :c:texpr:`*tabaddr` is not modified.
//!     :retval true: The table was successfully reallocated. :c:texpr:`*tabaddr` was updated.
//!     :retval false: The table has to be reallocated and but the allocation failed
//!
//!     .. code-block:: C
//!         :caption: Reserving a table capacity of 256 slots
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashtab(int) tab = scc_hashtab_init(int, eq);
//!         assert(scc_hashtab_reserve(&tab, 256));
//!
//!         scc_hashtab_free(tab);
#define scc_hashtab_reserve(tabaddr, capacity)                              \
    scc_hashtab_impl_reserve(tabaddr, capacity, sizeof(**(tabaddr)))

/* scc_hashtab_impl_remove
 *
 * Internal use only
 *
 * Remove the value stored in the handle from the hash
 * table. Return true if the value was found, otherwise
 * false
 *
 * void *tab
 *      Handle used to refer to the hash table
 *
 * size_t elemsize
 *      Size of each value stored in the table
 */
_Bool scc_hashtab_impl_remove(void *tab, size_t elemsize);

//! .. c:function:: _Bool scc_hashtab_remove(void *tab, type value)
//!
//!     Remove an element from the given hash table. Return :c:texpr:`true` if
//!     the element was found while probing.
//!
//!     :param tab: Handle identifying the hash table
//!     :param value: The value to probe for and remove
//!     :returns: Whether the operation was successful
//!     :retval true: :c:texpr:`tab` did contain :c:texpr:`value` before the call and
//!                   it has now been removed.
//!     :retval false: :c:texpr:`tab` did not contain :c:texpr:`value` at the time of the
//!                    call.
//!
//!     .. code-block:: C
//!         :caption: Insert and immediately remove an element in a hash table
//!
//!         extern _Bool eq(void const *l, void const *r);
//!
//!         scc_hashtab(int) tab = scc_hashtab_init(int, eq);
//!
//!         assert(scc_hashtab_insert(&tab, 18));
//!         assert(scc_hashtab_remove(tab, 18));
//!         assert(!scc_hashtab_find(tab, 18));
//!
//!         scc_hashtab_free(tab);
#define scc_hashtab_remove(tab, value)                                      \
    scc_hashtab_impl_remove((*(tab) = (value), (tab)), sizeof(*(tab)))

//! .. c:function:: void scc_hashtab_clear(void *tab)
//!
//!     Clear all entries in the given hash table.
//!
//!     :param tab: Handle identifying the hash table
void scc_hashtab_clear(void *tab);

#endif /* SCC_HASHTAB_H */
