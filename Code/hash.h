#ifndef HASH_H
#define HASH_H

#include "data.h"

typedef struct hash_entry {
  void *key;
  void *val;
  struct hash_entry *next;
} hash_entry;

typedef struct hash_table {
  unsigned size;
  unsigned mask;
  unsigned used;
  hash_entry **table;
} hash_table;

#define HT_INITIAL_SIZE 4
#define LONG_MAX 2147483647L
#define GET_HASH_INDEX(ht, key) (ht_gen_hash_function(key) & (ht)->mask)
#define HT_ERR -1
#define HT_OK 0

static unsigned int ht_gen_hash_function(const char *str);
static hash_table *ht_create();
static int ht_expand(hash_table *ht, unsigned long size);
static int ht_add(hash_table *ht, void *key, void *val);
static int ht_replace(hash_table *ht, void *key, void *val);
static int ht_delete(hash_table *ht, const void *key);
static void ht_release(hash_table *ht);
static hash_entry *ht_find(hash_table *ht, const void *key);

#endif  // HASH_H