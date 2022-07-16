#ifndef SCC_ALGORITHM_H
#define SCC_ALGORITHM_H

#include <stddef.h>

//? .. _scc_algo_lower_bound:
//? .. c:function:: size_t scc_algo_lower_bound(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *))
//?
//?     Compute the index of the first element in the sorted, consecutive value sequence
//?     starting at :code:`base` that is greater than or equal to :code:`key` as determined
//?     by the comparison function :code:`compare`.
//?
//?     The function is a `bsearch <https://man7.org/linux/man-pages/man3/bsearch.3.html>`-like
//?     adaptation of C++'s `std::lower_bound <https://en.cppreference.com/w/cpp/algorithm/lower_bound>`.
//?
//?     :param key: The key relative which the lower bound is to be found
//?     :param base: Base address of the consecutive sequence to search
//?     :param nmemb: Number of elements in the consequtive sequence starting at :code:`base`
//?     :param size: The size of each element in the sequence
//?     :param compare: The comparison function to use. It should accept the :code:`key` as its
//?                     first parameter and return an :c:expr:`int` less than, equal to, or greater
//?                     than 0 if the :code:`key` is found to be less than, equal to or greater than the array member, respectively.
//?                     See the `bsearch man page <https://man7.org/linux/man-pages/man3/bsearch.3.html>` for more information.
//?     :returns: Index of the first element greater than or equal to :code:`key`.
//?
//?     .. code-block:: C
//?         :caption: Examples
//?
//?         int compare(void const *l, void const *r) {
//?             return *(int const *)l - *(int const *)r;
//?         }
//?
//?         int seq0[] = { 2, 4, 6, 8 };
//?         /* 4 is first element >= 3 */
//?         assert(scc_lower_bound(&(int){ 3 }, seq0, 4, sizeof(int), compare) == 1u);
//?
//?         int seq1[] = { 0, 1, 2, 3, 4, 5 };
//?         /* No values >= 18, returned index is number of elements in the array */
//?         assert(scc_lower_bound(&(int){ 18 }, seq1, 6, sizeof(int), compare) == 6u);
//?
//?         int seq2[] = { 1, 2, 3 };
//?         /* 1 is >= 0 */
//?         assert(scc_lower_bound(&(int){ 0 }, seq2, 3, sizeof(int), compare) == 0);
size_t scc_algo_lower_bound(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *));

#endif /* SCC_ALGORITHM_H */
