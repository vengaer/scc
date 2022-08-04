#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>
#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btree.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare(void const *left, void const *right) {
    uint8_t leftval = *(uint8_t const *)left;
    uint8_t rightval = *(uint8_t const *)right;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) { /* NOLINT(readability-function-size) */
    if(size-- < 2u) {
        return 0;
    }

    scc_btree(uint8_t) btree;
#define CASE(i) case i: btree = scc_btree_with_order(uint8_t, compare, i); break;
    switch(*data++) {
        CASE(3);   CASE(4);   CASE(5);   CASE(6);   CASE(7);   CASE(8);   CASE(9);   CASE(10);  CASE(11);  CASE(12);
        CASE(13);  CASE(14);  CASE(15);  CASE(16);  CASE(17);  CASE(18);  CASE(19);  CASE(20);  CASE(21);  CASE(22);
        CASE(23);  CASE(24);  CASE(25);  CASE(26);  CASE(27);  CASE(28);  CASE(29);  CASE(30);  CASE(31);  CASE(32);
        CASE(33);  CASE(34);  CASE(35);  CASE(36);  CASE(37);  CASE(38);  CASE(39);  CASE(40);  CASE(41);  CASE(42);
        CASE(43);  CASE(44);  CASE(45);  CASE(46);  CASE(47);  CASE(48);  CASE(49);  CASE(50);  CASE(51);  CASE(52);
        CASE(53);  CASE(54);  CASE(55);  CASE(56);  CASE(57);  CASE(58);  CASE(59);  CASE(60);  CASE(61);  CASE(62);
        CASE(63);  CASE(64);  CASE(65);  CASE(66);  CASE(67);  CASE(68);  CASE(69);  CASE(70);  CASE(71);  CASE(72);
        CASE(73);  CASE(74);  CASE(75);  CASE(76);  CASE(77);  CASE(78);  CASE(79);  CASE(80);  CASE(81);  CASE(82);
        CASE(83);  CASE(84);  CASE(85);  CASE(86);  CASE(87);  CASE(88);  CASE(89);  CASE(90);  CASE(91);  CASE(92);
        CASE(93);  CASE(94);  CASE(95);  CASE(96);  CASE(97);  CASE(98);  CASE(99);  CASE(100); CASE(101); CASE(102);
        CASE(103); CASE(104); CASE(105); CASE(106); CASE(107); CASE(108); CASE(109); CASE(110); CASE(111); CASE(112);
        CASE(113); CASE(114); CASE(115); CASE(116); CASE(117); CASE(118); CASE(119); CASE(120); CASE(121); CASE(122);
        CASE(123); CASE(124); CASE(125); CASE(126); CASE(127); CASE(128); CASE(129); CASE(130); CASE(131); CASE(132);
        CASE(133); CASE(134); CASE(135); CASE(136); CASE(137); CASE(138); CASE(139); CASE(140); CASE(141); CASE(142);
        CASE(143); CASE(144); CASE(145); CASE(146); CASE(147); CASE(148); CASE(149); CASE(150); CASE(151); CASE(152);
        CASE(153); CASE(154); CASE(155); CASE(156); CASE(157); CASE(158); CASE(159); CASE(160); CASE(161); CASE(162);
        CASE(163); CASE(164); CASE(165); CASE(166); CASE(167); CASE(168); CASE(169); CASE(170); CASE(171); CASE(172);
        CASE(173); CASE(174); CASE(175); CASE(176); CASE(177); CASE(178); CASE(179); CASE(180); CASE(181); CASE(182);
        CASE(183); CASE(184); CASE(185); CASE(186); CASE(187); CASE(188); CASE(189); CASE(190); CASE(191); CASE(192);
        CASE(193); CASE(194); CASE(195); CASE(196); CASE(197); CASE(198); CASE(199); CASE(200); CASE(201); CASE(202);
        CASE(203); CASE(204); CASE(205); CASE(206); CASE(207); CASE(208); CASE(209); CASE(210); CASE(211); CASE(212);
        CASE(213); CASE(214); CASE(215); CASE(216); CASE(217); CASE(218); CASE(219); CASE(220); CASE(221); CASE(222);
        CASE(223); CASE(224); CASE(225); CASE(226); CASE(227); CASE(228); CASE(229); CASE(230); CASE(231); CASE(232);
        CASE(233); CASE(234); CASE(235); CASE(236); CASE(237); CASE(238); CASE(239); CASE(240); CASE(241); CASE(242);
        CASE(243); CASE(244); CASE(245); CASE(246); CASE(247); CASE(248); CASE(249); CASE(250); CASE(251); CASE(252);
        CASE(253); CASE(254); CASE(255);
        default:
            return 0;
    }
#undef CASE

    uint8_t const *it;
    dbg_pr("Order: %" PRIu8 "\n", data[-1]);
    dbg_pr("Data:\n");
    dbg_pr_n(it, data, size, "%" PRIu8 " ", *it);
    dbg_pr("\n");
    uint8_t const *p;
    unsigned mask;
    for(size_t i = 0u; i < size; ++i) {
        fuzz_assert(scc_btree_insert(&btree, data[i]), "Could not insert %" PRIu8 " in tree with order %" PRIu8, data[i], data[-1]);
        fuzz_assert(scc_btree_size(btree) == i + 1u, "Incorrect size");

        mask = scc_btree_inspect_invariants(btree);
        fuzz_assert(!mask, "Invariants violated with %#x after insertion of %" PRIu8 " in tree with order %" PRIu8,
                    mask, data[i], data[-1]);

        for(size_t j = 0u; j < i; ++j) {
            p = scc_btree_find(btree, data[j]);
            fuzz_assert(p, "Found no %" PRIu8 " in tree with order %" PRIu8, data[j], data[-1]);
            fuzz_assert(*p == data[j], "Find for %" PRIu8 " returned incorrect value %" PRIu8 " in tree with order %" PRIu8,
                        data[j], *p, data[-1]);
        }
    }

    for(size_t i = 0u; i < size; ++i) {
        fuzz_assert(scc_btree_remove(btree, data[i]), "Could not remove %" PRIu8 " from tree with order %" PRIu8, data[i], data[-1]);
        fuzz_assert(scc_btree_size(btree) == size - i - 1u, "Incorrect size after removal of %" PRIu8, data[i]);
        mask = scc_btree_inspect_invariants(btree);
        fuzz_assert(!mask, "Invariants violated with %#x after removal of %" PRIu8 " in tree with order %" PRIu8,
                    mask, data[i], data[-1]);
    }

    scc_btree_free(btree);
    return 0;
}
