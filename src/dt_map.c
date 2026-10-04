/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define DT_MAP_BUCKETS 16

/*
 * One key and its value.
 */
typedef struct dt_map_entry {
    char *key;                 /* our own copy of the key */
    dt_value value;            /* not ours to free */
    struct dt_map_entry *next; /* next entry in the same bucket */
} dt_map_entry;

/*
 * A hash table for lookup that remembers insertion order.
 */
struct dt_map {
    dt_map_entry **buckets; /* one linked list per bucket */
    size_t bucket_count;    /* how many buckets */
    dt_map_entry **order;   /* entries in the order they were added */
    size_t length;          /* how many keys */
    size_t capacity;        /* how many entries order can hold */
};

/*
 * bucket_of hashes key with 64-bit FNV-1a and returns its bucket index.
 */
static size_t bucket_of(const dt_map *m, const char *key)
{
    /* FNV's fixed starting value. Unsigned, so overflow wraps instead of being UB. */
    unsigned long long h = 14695981039346656037ULL;

    /* unsigned char keeps each byte in 0..255 even where char is signed. */
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p; /* mix this byte into the hash */
        h *= 1099511628211ULL;       /* FNV prime: spreads that byte across all bits */
    }

    /* Compress the huge hash into 0..bucket_count-1. */
    return (size_t)(h % m->bucket_count);
}

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */
    dt_map *m = malloc(sizeof *m);
    if (m == NULL) {
        return NULL;
    }

    m->buckets = calloc(DT_MAP_BUCKETS, sizeof *m->buckets);
    if (m->buckets == NULL) {
        free(m);
        return NULL;
    }

    m->bucket_count = DT_MAP_BUCKETS;
    m->order = NULL;
    m->length = 0;
    m->capacity = 0;

    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */
    if (m == NULL) return;

    for (size_t i = 0; i < m->bucket_count; i++) {
        dt_map_entry *curr = m->buckets[i];
        while (curr != NULL) {
            dt_map_entry *next = curr->next;
            free(curr->key);
            free(curr);
            curr = next;
        }
    }

    free(m->buckets);

    free(m->order);

    free(m);
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */
    return m->length;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */
    (void)m;
    (void)key;
    (void)v;
    return DT_ERR_CAPACITY;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
    (void)m;
    (void)key;
    (void)out;
    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    /* TODO: Remove the entry from its bucket and insertion position.
       Release the copied key. Return DT_ERR_KEY when the key is absent.
       a map holding alpha, beta, gamma:
         dt_map_remove(m, "alpha")  -> DT_OK, order is now beta, gamma
         dt_map_remove(m, "ghost")  -> DT_ERR_KEY, nothing changes
       reinserting "alpha" appends it after "gamma"
       cases/normal/map_basics.case, cases/boundary/map_remove_missing_key.case */
    (void)m;
    (void)key;
    return DT_ERR_KEY;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */
    (void)m;
    (void)index;
    (void)out;
    return DT_ERR_RANGE;
}
