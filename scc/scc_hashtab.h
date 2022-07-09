#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include "scc_assert.h"
#include "scc_bits.h"
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

#ifndef SCC_HASHTAB_STATIC_CAPACITY
//! .. c:enumerator:: SCC_HASHTAB_STATIC_CAPACITY
//!
//!     Capacity of the buffer used for the hash table small-size
//!     optimization. The value may be overridden by defining
//!     it before including the header.
//!
//!     Must be a power of 2 and larger than or equal to 32
enum { SCC_HASHTAB_STATIC_CAPACITY = 32 };
#endif

scc_static_assert(SCC_HASHTAB_STATIC_CAPACITY >= 32);
scc_static_assert(scc_bits_is_power_of_2(SCC_HASHTAB_STATIC_CAPACITY));

//! .. c:type:: _Bool(*scc_hashtab_eq)(void const *, void const *)
//!
//!     Signature of the function used for equality comparisons.
typedef _Bool(*scc_hashtab_eq)(void const *, void const *);

//! .. c:type:: unsigned long long(*scc_hashtab_hash)(void const *, size_t)
//!
//!     Signature of the hash function used.
typedef unsigned long long(*scc_hashtab_hash)(void const*, size_t);

//? .. c:type:: unsigned char scc_hashtab_metatype
//?
//?     Type used for storing hash table metadata
//?
//?     .. note::
//?         Internal use only
typedef unsigned char scc_hashtab_metatype;

//? .. c:struct:: scc_hashtab_perfevts
//?
//?     Counters for tracking performance-related events
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:var:: size_t ev_n_rehashes
//?
//?         Number of times the hash table has be rehashed
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
//?         Total size of the table, in bytes
struct scc_hashtab_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

//? .. _scc_hashtab_base:
//? .. c:struct:: scc_hashtab_base
//?
//?     Base struct of the hash table. Never exposed directly
//?     through the API. Instead, all "public" functions operate
//?     on a fat pointer referred to as a handle. Given a
//?     :code:`struct scc_hashtab_base` base, the address of said
//?     pointer is obtained by computing
//?     :c:expr:`&base.ht_fwoff + base.ht_fwoff + sizeof(base.ht_fwoff)`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _scc_hashtab_eq_ht_eq:
//?     .. c:var:: scc_hashtab_eq ht_eq
//?
//?         Pointer to function used for equality comparison
//?
//?     .. _scc_hashtab_hash_ht_hash:
//?     .. c:var:: scc_hashtab_hash ht_hash
//?
//?         Pointer to hash function
//?
//?     .. _size_t_ht_mdoff:
//?     .. c:var:: size_t ht_mdoff
//?
//?         Offset of metadata array relative base address. This
//?         is used to access the metadata in the FAM part of the
//?         struct.
//?
//?     .. _size_t_ht_size:
//?     .. c:var:: size_t ht_size
//?
//?         Size of the hash table
//?
//?     .. _size_t_ht_capacity:
//?     .. c:var:: size_t ht_capacity
//?
//?         Capacity of the hash table. Always a power fo two to
//?         allow for efficient module computation.
//?
//?     .. _struct_scc_hashtab_perfevts_ht_perf:
//?     .. c:var:: struct scc_hashtab_perfevts ht_perf
//?
//?         Performance counters.Present only if :ref:`CONFIG_PERFEVENTS <config_perfevents>`
//?         is set.
//?
//?     .. _unsigned_char_ht_dynalloc:
//?     .. c:var:: unsigned char ht_dynalloc
//?
//?         Set to 1 if the hash table was allocated dynamically.
//?         Upon initial construction, the table is allocated on the stack,
//?         meaning this field is set at first rehash.
//?
//?     .. _unsigned_char_ht_fwoff:
//?     .. c:var:: unsigned char ht_fwoff
//?
//?         Offset of the pointer exposed through the API relative the
//?         field itself.
//?
//?     .. c:var:: unsigned char ht_buffer[]
//?
//?         FAM hiding type-specific details. For the exact layout,
//?         refer to :ref:`scc_hashtab_impl_layout <scc_hashtab_impl_layout>`.
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

//? .. _scc_hashtab_impl_layout:
//? .. c:macro:: scc_hashtab_impl_layout(type)
//?
//?     The actual layout of a hash table storing instances of
//?     the given :code:`type`. The :c:texpr:`ht_eq` through
//?     :c:texpr:`ht_fwoff` members are the same as for
//?     :ref:`struct scc_hashtab_base <scc_hashtab_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:struct:: @layout
//?
//?         .. c:var:: scc_hashtab_eq ht_eq
//?
//?             See :ref:`ht_eq <scc_hashtab_eq_ht_eq>`.
//?
//?         .. c:var:: scc_hashtab_hash ht_hash
//?
//?             See :ref:`ht_hash <scc_hashtab_hash_ht_hash>`.
//?
//?         .. c:var:: size_t ht_mdoff
//?
//?             See :ref:`ht_mdoff <size_t_ht_mdoff>`.
//?
//?         .. c:var:: size_t ht_size
//?
//?             See :ref:`ht_size <size_t_ht_size>`.
//?
//?         .. c:var:: size_t ht_capacity
//?
//?             See :ref:`ht_capacity <size_t_ht_capacity>`.
//?
//?         .. c:var:: struct scc_hashtab_perfevts ht_perfevts
//?
//?             See :ref:`ht_perfevts <struct_scc_hashtab_perfevts_ht_perf>`.
//?
//?         .. c:var:: unsigned char ht_dynalloc
//?
//?             See :ref:`ht_dynalloc <unsigned_char_ht_dynalloc>`.
//?
//?         .. c:var:: unsigned char ht_fwoff
//?
//?             See :ref:`ht_fwoff <unsigned_char_ht_fwoff>`.
//?
//?         .. _unsigned_char_ht_bkoff:
//?         .. c:var:: unsigned char ht_bkoff
//?
//?             Field used for tracking padding between :ref:`ht_fwoff <unsigned_char_ht_fwoff>`
//?             and :ref:`ht_curr <type_ht_curr>`. Just as :code:`ht_fwoff` is used to
//?             compute the address of :code:`ht_curr` given a base address, :code:`ht_bkoff` is
//?             used to compute the base address given the address of :code:`ht_curr`.
//?
//?             The primary purpose of the field itself is to force injection of padding bytes
//?             between :code:`ht_fwoff` and :code:`ht_curr`. Whether the offset is actually
//?             stored in this field depends on the alignment of the :code:`type` parameter. In
//?             practice, this is the case only for 1-byte aligned types. For types with stricted
//?             alignemtn requirements, the last padding byte is used instead.
//?
//?
//?         .. _type_ht_curr:
//?         .. c:var:: type ht_curr
//?
//?             Volatile instance of the type stored in the hash table used as intermediary storage
//?             for rvalue support. Values to be inserted or used in probing are written to this
//?             field to allow for operating on an lvalue.
//?
//?         .. _type_ht_data:
//?         .. c:var:: type ht_data[SCC_HASHTAB_STATIC_CAPACITY]
//?
//?             The data array in which the actual elements of the table are stored.
//?
//?         .. _scc_hashtab_metatype_ht_meta:
//?         .. c:var:: scc_hashtab_metatype ht_meta[SCC_HAShTAB_STATIC_CAPACITY]
//?
//?             Metadata array used for tracking vacant slots. Each slot *n* in the
//?             metadata array correspons to slot *n* in the :ref:`ht_data array <type_ht_data>`.
//?
//?             A slot is vacant if zero-indexed bit 7 in its metadata entry is not set. If the
//?             entire byte is 0, the slot has never been occupied and serves as a stop marker for
//?             both :ref:`insertion <scc_hashtab_insert>` and :ref:`find <scc_hashtab_find>` probing.
//?             The metadata entries of previously used but now vacated slots are assigned the value
//?             :c:expr:`0x7f`. These slots are used for insertion but serve as probe stops only if
//?             it is the very last slot searched in the table.
//?
//?             An entyre with zero-indexed bit 6 set signifies that a slot is occupied. In this case,
//?             zero-indexed bits 0-6, inclusive, contain the 7 most significant bits of the hash
//?             computed for the corresponding entry in the :code:`ht_data` array. This is used
//?             for avoiding unnecessary calls to :ref:`ht_eq <scc_hashtab_eq_ht_eq>`.
//?
//?         .. c:var:: scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ]
//?
//?             Guard to allow for unaligned vector loads on architectures supporting
//?             it without risking reads from potential guard pages. The :code:`SCC_HASHTAB_GUARDSZ`
//?             low bytes of :ref:`ht_meta <scc_hashtab_metatype_ht_meta>` are mirrored in the guard.
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
        type ht_data[SCC_HASHTAB_STATIC_CAPACITY];                          \
        scc_hashtab_metatype ht_meta[SCC_HASHTAB_STATIC_CAPACITY];          \
        scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ];                 \
    }

//? .. c:function:: void *scc_hashtab_impl_new(struct scc_hashtab_base *base, size_t coff, size_t mdoff)
//?
//?     Initialize the given hash table and return a handle to it.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of hash table base. The handle returned yb the function refers to the :ref:`ht_curr <type_ht_curr>`
//?                  entry in this table.
//?     :param coff: Offset of :code:`ht_curr` relative the address of :c:expr:`*base`
//?     :param mdoff: Offset of :code:`ht_meta` relative the address of :c:expr`*base`
void *scc_hashtab_impl_new(struct scc_hashtab_base *base, size_t coff, size_t mdoff);

//! .. _scc_hashtab_with_hash:
//! .. c:function:: void *scc_hashtab_with_hash(type, scc_hashtab_eq eq, scc_hashtab_hash hash)
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
//!               which :c:texpr:`scc_hashtab_with_hash` is invoked.
//!
//!     .. code-block:: C
//!         :caption: Example usage
//!
//!         extern _Bool eq(void const *l, void const *r);
//!         extern unsigned long long hash(void const *data, size_t size);
//!
//!         scc_hashtab(int) tab;
//!         {
//!             tab = scc_hashtab_with_hash(int, eq, hash);
//!             /* tab is valid */
//!         }
//!         /* tab is no longer valid */
#define scc_hashtab_with_hash(type, eq, hash)                               \
    scc_hashtab_impl_new(                                                   \
        (void *)&(scc_hashtab_impl_layout(type)){                           \
            .ht_eq = eq,                                                    \
            .ht_hash = hash,                                                \
            .ht_capacity = SCC_HASHTAB_STATIC_CAPACITY                      \
        },                                                                  \
        offsetof(scc_hashtab_impl_layout(type), ht_curr),                   \
        offsetof(scc_hashtab_impl_layout(type), ht_meta)                    \
    )

//! .. _scc_hashtab_new:
//! .. c:function:: void *scc_hashtab_new(type, scc_hashtab_eq eq)
//!
//!     Initializes a hash table using the default :ref:`Fowler-Noll-Vo <scc_hashtab_fnv1a>`
//!     hash function. Exactly equivalent to calling
//!     :code:`scc_hashtab_with_hash(keytype, valuetype, eq, scc_hashtab_fnv1a)`. See
//!     :ref:`its documentation <scc_hashtab_with_hash>` for restrictions and common
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
//!               which :c:texpr:`scc_hashtab_new` is invoked.
//!
//!     .. seealso:: :ref:`scc_hashtab_with_hash <scc_hashtab_with_hash>`
#define scc_hashtab_new(type, eq)                                           \
    scc_hashtab_with_hash(type, eq, scc_hashtab_fnv1a)

//? .. c:function:: size_t scc_hashtab_impl_bkpad(void const *tab)
//?
//?     Compute number of padding bytes between :ref:`ht_curr <type_ht_curr>` and
//?     :ref:`ht_fwoff <unsigned_char_ht_fwoff>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Hash table handle
//?     :returns: The number of padding bytes between :code:`ht_curr` and :code:`ht_fwoff`
inline size_t scc_hashtab_impl_bkpad(void const *tab) {
    return ((unsigned char const *)tab)[-1] + sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
}

//? .. c:macro:: scc_hashtab_impl_base_qual(tab, qual)
//?
//?     Obtain qualified pointer to the
//?     :ref:`struct scc_hashtab_base <scc_hashtab_base>` corresponding
//?     to the given :c:texpr:`tab`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Hash table handle
//?     :param qual: Oprional qualifiers to apply to the pointer
//?     :returns: Appropriately qualified address of the
//?               :ref:`struct scc_hashtab_base <scc_hashtab_base>`
//?               corresponding th :code:`tab`.
#define scc_hashtab_impl_base_qual(tab, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(tab) - scc_hashtab_impl_bkpad(tab),          \
        struct scc_hashtab_base,                                            \
        ht_fwoff,                                                           \
        qual                                                                \
    )

//? .. c:macro:: scc_hashtab_impl_base(tab)
//?
//?     Obtain unqualified pointer to the
//?     :ref:`struct scc_hashtab_base <scc_hashtab_base>` corresponding
//?     to the given :code:`tab`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Handle to the hash table whose base address is to be obtained
//?     :returns: Address of the :ref:`struct scc_hashtab_base <scc_hashtab_base>`
//?               corresponding to the given :code:`tab`.
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
//!     :ref:`scc_hashtab_new <scc_hashtab_new>` or
//!     :ref:`scc_hashtab_with_hash <scc_hashtab_with_hash>`.
//!
//!     :param tab: Handle to the hash table to free
void scc_hashtab_free(void *tab);

//? .. c:function:: _Bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize)
//?
//?     Insert the value in :ref:`ht_curr <type_ht_curr>` in the hash table. See
//?     :ref:`scc_hashtab_insert <scc_hashtab_insert>` for more info.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tabaddr: Address of the handle used to refer to the hash table.
//?     :param elemsize: Size of the elements stored in the hash table
//?     :returns: A :code:`_Bool` indicating whether the element was successfully
//?               inserted.
//?     :retval true: The element was successfully inserted
//?     :retval false: The given hash table already contains the element in :code:`ht_curr`
//?     :retval false: Memory allocation faliure
_Bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize);

//! .. _scc_hashtab_insert:
//! .. c:function:: _Bool scc_hashtab_insert(void *tabaddr, type element)
//!
//!     Insert an element in the hash table. Succeeds only if  :c:texpr:`element`
//!     is not already present in the table.
//!
//!     The call may result in the hash table being reallocated in which case
//!     :c:expr:`*(void **)tabaddr` is updated to refer to the newly allocated table.
//!     Should such a reallocation fail, :c:expr:`*(void **)tabaddr` remains unchanged
//!     and must still be passed to :ref:`scc_hashtab_free <scc_hashtab_free>` in order
//!     for the its memory to be reclaimed.
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
//!         scc_hashtab(int) tab = scc_hashtab_new(int, eq);
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

//? .. c:function:: void const *scc_hashtab_impl_find(void const *tab, size_t elemsize)
//?
//?     Probe the given hash table for the value stored in :ref:`ht_curr <type_ht_curr>`.
//?     See :ref:`scc_hashtab_find <scc_hashtab_find>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Handle to the hash table to search
//?     :param elemsize: Size of the elements stored in the table
//?     :returns: Const-qualified address of the element in the table
//?     :retval NULL: The element was not found in the table
//?     :retval Valid address: Const-qualified address of the found element
void const *scc_hashtab_impl_find(void const *tab, size_t elemsize);

//! .. _scc_hashtab_find:
//! .. c:function:: void const *scc_hashtab_find(void *tab, type value)
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
//!         scc_hashtab(char const *) tab = scc_hashtab_new(char const *, eq);
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

//? .. c:function:: _Bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize)
//?
//?     Reserve storage for specified number of slots. If the table is already sufficiently
//?     large, return immediately. If it is successfully reallocated, :c:expr:`*(void **)tabaddr`
//?     is updated to refer to the new table. If reallocation fails, :c:expr:`*(void **)tabaddr`
//?     remains unchanged and must still be passed to :ref:`scc_hashtab_free <scc_hashtab_free>`
//?     in order to reclaim any memory allocated for it.
//?
//?     The actual capacity allocated for the table may exceed the requested one.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tabaddr: Address of the handle used to refer to the hash table
//?     :param capacity: The desired capacity
//?     :param elemsize: Size of each element in the given table
//?     :returns: A :code:`_Bool` indicating whether the request could be fulfilled
//?     :retval true: The hash table capacity is already sufficiently large
//?     :retval true: The hash table was successfully rehashed to accomodate the request
//?     :retval false: Memory allocation failure
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
//!         scc_hashtab(int) tab = scc_hashtab_new(int, eq);
//!         assert(scc_hashtab_reserve(&tab, 256));
//!
//!         scc_hashtab_free(tab);
#define scc_hashtab_reserve(tabaddr, capacity)                              \
    scc_hashtab_impl_reserve(tabaddr, capacity, sizeof(**(tabaddr)))

//? .. c:function:: _Bool scc_hashtab_impl_remove(void *tab, size_t elemsize)
//?
//?     Remove the value stored in :ref:`ht_curr <type_ht_curr>` from the
//?     hash table.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Hash table handle
//?     :param elemsize: Size of each element in the hash table
//?     :returns: A :code:`_Bool` indicating whether the element was removed
//?     :retval true: The element was found in and subsequently removed from the hash table
//?     :retval false: The hash table did not contain the element in question.
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
//!         scc_hashtab(int) tab = scc_hashtab_new(int, eq);
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
