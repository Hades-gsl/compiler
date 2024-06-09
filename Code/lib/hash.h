#ifndef HASH_H
#define HASH_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct HashEntry {
  void *key;
  void *val;
  struct HashEntry *next;
} HashEntry;

typedef struct HtType {
  unsigned int (*hashFunction)(const void *key);
  void *(*keyDup)(void *privdata, const void *key);
  void *(*valDup)(void *privdata, const void *obj);
  int (*keyCompare)(void *privdata, const void *key1, const void *key2);
  void (*keyDestructor)(void *privdata, void *key);
  void (*valDestructor)(void *privdata, void *obj);
} HtType;

typedef struct HashTable {
  HtType *type;
  unsigned size;
  unsigned mask;
  unsigned used;
  HashEntry **table;
  void *privdata;
} HashTable;

/* This is the initial size of every hash table */
#define HT_INITIAL_SIZE 4
/* This is the maximum size of the hash table */
#define LONG_MAX 2147483647L

/* status code */
#define HT_ERR -1
#define HT_OK 0

/* ------------------------------- Macros ------------------------------------*/
#define htFreeEntryVal(ht, entry) \
  if ((ht)->type->valDestructor)  \
  (ht)->type->valDestructor((ht)->privdata, (entry)->val)

#define htSetHashVal(ht, entry, _val_)                        \
  do {                                                        \
    if ((ht)->type->valDup)                                   \
      entry->val = (ht)->type->valDup((ht)->privdata, _val_); \
    else                                                      \
      entry->val = (_val_);                                   \
  } while (0)

#define htFreeEntryKey(ht, entry) \
  if ((ht)->type->keyDestructor)  \
  (ht)->type->keyDestructor((ht)->privdata, (entry)->key)

#define htSetHashKey(ht, entry, _key_)                        \
  do {                                                        \
    if ((ht)->type->keyDup)                                   \
      entry->key = (ht)->type->keyDup((ht)->privdata, _key_); \
    else                                                      \
      entry->key = (_key_);                                   \
  } while (0)

#define htCompareHashKeys(ht, key1, key2)                   \
  (((ht)->type->keyCompare)                                 \
       ? (ht)->type->keyCompare((ht)->privdata, key1, key2) \
       : (key1) == (key2))

#define htHashKey(ht, key) (ht)->type->hashFunction(key)

#define htGetEntryKey(he) ((he)->key)
#define htGetEntryVal(he) ((he)->val)
#define htSlots(ht) ((ht)->size)
#define htSize(ht) ((ht)->used)

/* API */
unsigned int htGenHashFunction(const char *str);
HashTable *htCreate(HtType *type, void *privDataPtr);
int htExpand(HashTable *ht, unsigned long size);
int htAdd(HashTable *ht, void *key, void *val);
int htReplace(HashTable *ht, void *key, void *val);
int htDelete(HashTable *ht, const void *key);
void htRelease(HashTable *ht);
HashEntry *htFind(HashTable *ht, const void *key);

#endif  // HASH_H