#ifndef HASH_H
#define HASH_H

#include "data.h"

typedef struct hash_entry {
  void *key;
  void *val;
  struct hash_entry *next;
} hash_entry;

typedef struct ht_type {
  unsigned int (*hash_function)(const void *key);
  void *(*key_dup)(void *privdata, const void *key);
  void *(*val_dup)(void *privdata, const void *obj);
  int (*key_compare)(void *privdata, const void *key1, const void *key2);
  void (*key_destructor)(void *privdata, void *key);
  void (*val_destructor)(void *privdata, void *obj);
} ht_type;

typedef struct hash_table {
  ht_type *type;
  unsigned size;
  unsigned mask;
  unsigned used;
  hash_entry **table;
  void *privdata;
} hash_table;

/* This is the initial size of every hash table */
#define HT_INITIAL_SIZE 4
/* This is the maximum size of the hash table */
#define LONG_MAX 2147483647L

/* status code */
#define HT_ERR -1
#define HT_OK 0

/* ------------------------------- Macros ------------------------------------*/
#define htFreeEntryVal(ht, entry) \
  if ((ht)->type->val_destructor) \
  (ht)->type->val_destructor((ht)->privdata, (entry)->val)

#define htSetHashVal(ht, entry, _val_)                         \
  do {                                                         \
    if ((ht)->type->val_dup)                                   \
      entry->val = (ht)->type->val_dup((ht)->privdata, _val_); \
    else                                                       \
      entry->val = (_val_);                                    \
  } while (0)

#define htFreeEntryKey(ht, entry) \
  if ((ht)->type->key_destructor) \
  (ht)->type->key_destructor((ht)->privdata, (entry)->key)

#define htSetHashKey(ht, entry, _key_)                         \
  do {                                                         \
    if ((ht)->type->key_dup)                                   \
      entry->key = (ht)->type->key_dup((ht)->privdata, _key_); \
    else                                                       \
      entry->key = (_key_);                                    \
  } while (0)

#define htCompareHashKeys(ht, key1, key2)                    \
  (((ht)->type->key_compare)                                 \
       ? (ht)->type->key_compare((ht)->privdata, key1, key2) \
       : (key1) == (key2))

#define htHashKey(ht, key) (ht)->type->hash_function(key)

#define htGetEntryKey(he) ((he)->key)
#define htGetEntryVal(he) ((he)->val)
#define htSlots(ht) ((ht)->size)
#define htSize(ht) ((ht)->used)

/* API */
static unsigned int ht_gen_hash_function(const char *str);
static hash_table *ht_create(ht_type *type, void *privdata_ptr);
static int ht_expand(hash_table *ht, unsigned long size);
static int ht_add(hash_table *ht, void *key, void *val);
static int ht_replace(hash_table *ht, void *key, void *val);
static int ht_delete(hash_table *ht, const void *key);
static void ht_release(hash_table *ht);
static hash_entry *ht_find(hash_table *ht, const void *key);

#endif  // HASH_H