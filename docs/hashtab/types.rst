#####
Types
#####

The header exposes the following typedefs and macros expanding to type

`scc_hashtab(type)`
*******************
Macro expanding to a type suitable for referring to a hash table storing instances of type `type`. All functions exposed by the API operate on a type expanded from this macro. While the type may be safely converted to and from a `void *`, any generic pointers must be cast to `scc_hashtab(type)` before being passed to a hash table function.

`_Bool(*ht_eq)(void const *, void const *)`
*******************************************
Signature of the function used for equality comparisons while probing the hash table.

`unsigned long long(*ht_hash)(void const *, size_t)`
****************************************************
Signature of the function used for hashing elements.
