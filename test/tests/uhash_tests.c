/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019-2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "uhash.h"
#include "utest.h"

#define MAX_VAL 100

UHASH_INIT(IntHash, uint32_t, uint32_t, uhash_int32_hash, uhash_identical)
UHASH_INIT_PI(IntHashPi, uint32_t, uint32_t, NULL, NULL)

static uhash_uint int32_hash(uint32_t num) {
    return uhash_int32_hash(num);
}

static bool int32_eq(uint32_t lhs, uint32_t rhs) {
    return lhs == rhs;
}

bool uhash_test_memory(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    utest_assert_not_null(set);

    uhash_ret ret = uhash_put(IntHash, set, 0, NULL);
    utest_assert(ret == UHASH_INSERTED);
    utest_assert(uhash_count(set) == 1);

    uhash_uint buckets = set->n_buckets;
    ret = uhash_resize(IntHash, set, 200);
    utest_assert(ret == UHASH_OK);
    utest_assert(set->n_buckets > buckets);

    buckets = set->n_buckets;
    ret = uhash_resize(IntHash, set, 100);
    utest_assert(ret == UHASH_OK);
    utest_assert(set->n_buckets < buckets);

    buckets = set->n_buckets;
    uhash_clear(IntHash, set);
    utest_assert(set->n_buckets == buckets);
    utest_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

bool uhash_test_base(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    utest_assert_not_null(set);

    utest_assert(uhash_get(IntHash, set, 0) == UHASH_INDEX_MISSING);
    utest_assert(uhash_count(set) == 0);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        utest_assert(uhash_put(IntHash, set, i, NULL) == UHASH_INSERTED);
    }

    utest_assert(uhash_count(set) == MAX_VAL);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_uint idx = uhash_get(IntHash, set, i);
        utest_assert(idx != UHASH_INDEX_MISSING);
        utest_assert(uhash_exists(set, idx));
    }

    utest_assert(uhash_get(IntHash, set, 200) == UHASH_INDEX_MISSING);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_uint idx = uhash_get(IntHash, set, i);
        uhash_delete(IntHash, set, idx);
        utest_assert_false(uhash_exists(set, idx));
        utest_assert(uhash_get(IntHash, set, i) == UHASH_INDEX_MISSING);
    }

    utest_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

bool uhash_test_map(void) {
    UHash(IntHash) *map = uhmap_alloc(IntHash);
    utest_assert_not_null(map);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        utest_assert(uhmap_set(IntHash, map, i, i, NULL) == UHASH_INSERTED);
    }

    UHash(IntHash) *set = uhset_alloc(IntHash);
    utest_assert_not_null(set);
    utest_assert(uhash_copy_as_set(IntHash, map, set) == UHASH_OK);
    utest_assert(uhset_equals(IntHash, set, map));
    uhash_free(IntHash, set);

    uint32_t existing_val;
    utest_assert(uhmap_set(IntHash, map, 0, 1, &existing_val) == UHASH_PRESENT);
    utest_assert(existing_val == 0);

    utest_assert(uhmap_add(IntHash, map, 0, 1, &existing_val) == UHASH_PRESENT);
    utest_assert(existing_val == 1);

    utest_assert(uhmap_replace(IntHash, map, 0, 0, &existing_val));
    utest_assert(uhmap_get(IntHash, map, 0, UINT32_MAX) == 0);
    utest_assert(existing_val == 1);

    utest_assert(uhmap_add(IntHash, map, MAX_VAL, MAX_VAL, &existing_val) == UHASH_INSERTED);
    utest_assert(uhmap_remove(IntHash, map, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing_key;
        utest_assert(uhmap_pop(IntHash, map, i, &existing_key, &existing_val));
        utest_assert(existing_key == i);
        utest_assert(existing_val == i);
    }

    uhash_free(IntHash, map);
    return true;
}

bool uhash_test_set(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    utest_assert_not_null(set);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        utest_assert(uhset_insert(IntHash, set, i) == UHASH_INSERTED);
    }

    utest_assert(uhset_insert(IntHash, set, 0) == UHASH_PRESENT);
    utest_assert(uhash_count(set) == MAX_VAL);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing;
        utest_assert(uhset_insert_get_existing(IntHash, set, i, &existing) == UHASH_PRESENT);
        utest_assert(existing == i);
    }

    uint32_t elements[MAX_VAL + 1];
    for (uint32_t i = 0; i < MAX_VAL + 1; ++i) {
        elements[i] = i;
    }

    utest_assert(uhset_insert_all(IntHash, set, elements, MAX_VAL) == UHASH_PRESENT);
    utest_assert(uhset_insert_all(IntHash, set, elements, MAX_VAL + 1) == UHASH_INSERTED);

    utest_assert(uhash_contains(IntHash, set, MAX_VAL));
    utest_assert(uhset_remove(IntHash, set, MAX_VAL));
    utest_assert_false(uhash_contains(IntHash, set, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing;
        utest_assert(uhset_pop(IntHash, set, i, &existing));
        utest_assert(existing == i);
    }

    UHash(IntHash) *other_set = uhset_alloc(IntHash);
    utest_assert_not_null(other_set);
    uhset_insert_all(IntHash, set, elements, MAX_VAL);
    uhset_insert_all(IntHash, other_set, elements, MAX_VAL / 2);

    utest_assert(uhset_is_superset(IntHash, set, other_set));
    utest_assert_false(uhset_is_superset(IntHash, other_set, set));

    utest_assert_false(uhset_equals(IntHash, set, other_set));
    uhset_insert_all(IntHash, other_set, elements, MAX_VAL);
    utest_assert(uhset_equals(IntHash, set, other_set));

    uhash_free(IntHash, other_set);
    other_set = uhset_alloc(IntHash);
    utest_assert_not_null(other_set);
    utest_assert(uhash_copy(IntHash, set, other_set) == UHASH_OK);
    utest_assert(uhset_equals(IntHash, set, other_set));
    uhash_free(IntHash, other_set);

    other_set = uhset_alloc(IntHash);
    utest_assert_not_null(other_set);
    uhset_insert(IntHash, other_set, MAX_VAL);
    utest_assert(uhset_union(IntHash, other_set, set) == UHASH_OK);

    utest_assert(uhset_is_superset(IntHash, other_set, set));
    utest_assert_false(uhset_is_superset(IntHash, set, other_set));

    uhset_intersect(IntHash, other_set, set);
    utest_assert(uhset_equals(IntHash, other_set, set));
    uhash_free(IntHash, other_set);

    uint32_t element = uhset_get_any(IntHash, set, MAX_VAL);
    utest_assert(element != MAX_VAL);

    uint32_t replaced;
    utest_assert(uhset_replace(IntHash, set, element, &replaced));
    utest_assert(replaced == element);

    uhash_clear(IntHash, set);
    element = uhset_get_any(IntHash, set, MAX_VAL);
    utest_assert(element == MAX_VAL);

    uhash_free(IntHash, set);
    return true;
}

bool uhash_test_per_instance(void) {
    UHash(IntHashPi) *map = uhmap_alloc_pi(IntHashPi, int32_hash, int32_eq);
    utest_assert_not_null(map);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        utest_assert(uhmap_set(IntHashPi, map, i, i, NULL) == UHASH_INSERTED);
    }

    uint32_t existing_val;
    utest_assert(uhmap_set(IntHashPi, map, 0, 1, &existing_val) == UHASH_PRESENT);
    utest_assert(existing_val == 0);

    utest_assert(uhmap_add(IntHashPi, map, 0, 1, &existing_val) == UHASH_PRESENT);
    utest_assert(existing_val == 1);

    utest_assert(uhmap_replace(IntHashPi, map, 0, 0, &existing_val));
    utest_assert(uhmap_get(IntHashPi, map, 0, UINT32_MAX) == 0);
    utest_assert(existing_val == 1);

    utest_assert(uhmap_add(IntHashPi, map, MAX_VAL, MAX_VAL, &existing_val) == UHASH_INSERTED);
    utest_assert(uhmap_remove(IntHashPi, map, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing_key;
        utest_assert(uhmap_pop(IntHashPi, map, i, &existing_key, &existing_val));
        utest_assert(existing_key == i);
        utest_assert(existing_val == i);
    }

    uhash_free(IntHashPi, map);
    return true;
}