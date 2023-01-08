#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>
#include <inspect/btmap_inspect.h>
#include <scc/btmap.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static size_t partition(uint32_t *data, size_t size);
static int compare(void const *l, void const *r);
static void fuzz_insertion(void *map, uint32_t const *data, size_t ue, size_t size);
static void fuzz_find(void *map, uint32_t const *data, size_t ue);
static void fuzz_removal(void *map, uint32_t const *data, size_t ue);
static void fuzz_nonexistent(void *map, uint32_t const *data, size_t ue);

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    scc_btmap(uint32_t, uint32_t) btmap;
#define CASE(i) case i: btmap = scc_btmap_with_order(uint32_t, uint32_t, compare, i); break;
    switch(*data) {
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

    uint32_t *buf = 0;
    size = size / sizeof(*buf);
    if(!size) {
        scc_btmap_free(btmap);
        return 0;
    }

    buf = malloc(size * sizeof(*buf));
    if(!buf) {
        return 0;
    }

    memcpy(buf, data, size * sizeof(*buf));
    unsigned unique_end = partition(buf, size);

    uint32_t const *it = 0;
    dbg_pr("Data:\n");
    dbg_pr_n(it, buf, size, "%#08" PRIx32 " ", *it);
    dbg_pr("\n");

    fuzz_insertion(btmap, buf, unique_end, size);
    fuzz_find(btmap, buf, unique_end);
    fuzz_removal(btmap, buf, unique_end);
    fuzz_nonexistent(btmap, buf, unique_end);

    free(buf);
    scc_btmap_free(btmap);
    return 0;
}

static size_t partition(uint32_t *data, size_t size) {
    bool unique = true;
    unsigned unique_end = 0u;
    /* The most stupid partitioning scheme ever */
    for(unsigned i = 0u; i < size; ++i) {
        unique = true;
        for(unsigned j = 0u; j < i; ++j) {
            if(data[i] == data[j]) {
                unique = false;
                break;
            }
        }
        if(unique) {
            data[unique_end++] = data[i];
        }
    }
    return unique_end;
}

static int compare(void const *l, void const *r) {
    uint32_t leftval = *(uint32_t const *)l;
    uint32_t rightval = *(uint32_t const *)r;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

static void fuzz_insertion(void *map, uint32_t const *data, size_t ue, size_t size) {
    scc_btmap(uint32_t, uint32_t) btmap = map;

    scc_inspect_mask mask;
    uint32_t *val;
    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(
            scc_btmap_insert(&btmap, data[i], data[i] << 1u),
            "Could not insert (%#08" PRIx32 ", %#08" PRIx32 ") in the map",
            data[i], data[i] << 1u
        );

        fuzz_assert(
            scc_btmap_size(btmap) == i + 1u,
            "Invalid size %zu after %u insertions", scc_btmap_size(btmap), i + 1u
        );

        mask = scc_btmap_inspect_invariants(btmap);
        fuzz_assert(
            !mask,
            "Properties violated upon insertion of %#08" PRIx32 ", mask %#" PRIx32,
            data[i], (uint32_t)mask
        );

        for(unsigned j = 0u; j < i; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            fuzz_assert(val, "Key %#08" PRIx32 " not found in map", data[j]);
            fuzz_assert(
                *val == data[j] << 1u,
                "Key %#08" PRIx32 " maps to unexpected value %#08" PRIx32, data[j], *val
            );
        }
    }

    size_t mapsize = scc_btmap_size(btmap);
    for(unsigned i = ue; i < size; ++i) {
        fuzz_assert(
            scc_btmap_insert(&btmap, data[i], 0u),
            "Could not overwrite value for key %#08" PRIx32, data[i]
        );

        fuzz_assert(
            scc_btmap_size(btmap) == mapsize,
            "Map size increased on overwrite"
        );

        for(unsigned j = ue; j < i; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            fuzz_assert(val, "Key %#08" PRIx32 " not found in map", data[j]);
            fuzz_assert(!*val, "Value mapping to key %#08" PRIx32 " not cleared", data[j]);
        }
    }

    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(scc_btmap_insert(&btmap, data[i], data[i] << 1u));
    }
}

static void fuzz_find(void *map, uint32_t const *data, size_t ue) {
    scc_btmap(uint32_t, uint32_t) btmap = map;
    uint32_t *val;
    for(unsigned i = 0u; i < ue; ++i) {
        val = scc_btmap_find(btmap, data[i]);
        fuzz_assert(val, "Could not find key %#08" PRIx32, data[i]);
        fuzz_assert(*val == data[i] << 1u, "Value mismatch for key %#08" PRIx32, data[i]);
    }
}

static void fuzz_removal(void *map, uint32_t const *data, size_t ue) {
    scc_btmap(uint32_t, uint32_t) btmap = map;
    scc_inspect_mask mask;
    uint32_t *val;
    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(scc_btmap_size(btmap) == ue - i, "Size mismatch");
        fuzz_assert(
            scc_btmap_remove(btmap, data[i]),
            "Error removing %#08" PRIx32, data[i]
        );

        mask = scc_btmap_inspect_invariants(btmap);
        fuzz_assert(
            !mask,
            "Properties violated upon removal of %#08" PRIx32 ", mask %#" PRIx32,
            data[i], (uint32_t)mask
        );

        for(unsigned j = i + 1u; j < ue; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            fuzz_assert(val, "Key %#08" PRIx32 " not found in map", data[j]);
            fuzz_assert(
                *val == data[j] << 1u,
                "Key %#08" PRIx32 " maps to unexpected value %#08" PRIx32, data[j], *val
            );
        }
    }

    fuzz_assert(!scc_btmap_size(btmap), "Map not empty");
}

static void fuzz_nonexistent(void *map, uint32_t const *data, size_t ue) {
    scc_btmap(uint32_t, uint32_t) btmap = map;
    uint32_t nonexistent = 0u;
    bool found = false;
    fuzz_assert(ue < UINT32_MAX);
    while(!found) {
        found = true;
        for(size_t i = 0u; i < ue; ++i) {
            if(nonexistent == data[i]) {
                ++nonexistent;
                found = false;
            }
            break;
        }
    }

    fuzz_assert(!scc_btmap_find(btmap, nonexistent));
    fuzz_assert(!scc_btmap_remove(btmap, nonexistent));
}
