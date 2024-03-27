#include "hash.h"

static unsigned long _ht_next_power(unsigned long size);
static unsigned long _ht_get_index(hash_table *ht, const void *key);

/* Generic hash function (a popular one from Bernstein) */
static unsigned int ht_gen_hash_function(const char *str) {
  unsigned int hash = 5381;

  while (*str) hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
  return hash;
}

/* Create a new hash table */
static hash_table *ht_create() {
  hash_table *ht = malloc(sizeof(hash_table));
  if (!ht) return NULL;

  ht->size = HT_INITIAL_SIZE;
  ht->used = 0;
  ht->mask = ht->size - 1;
  ht->table = calloc(ht->size, sizeof(hash_entry *));
  if (!ht->table) {
    free(ht);
    return NULL;
  }

  return ht;
}

/* Expand the hash table */
static int ht_expand(hash_table *ht, unsigned long size) {
  hash_table *new_ht = ht_create();
  if (!new_ht) return HT_ERR;

  new_ht->size = _ht_next_power(size);
  new_ht->mask = new_ht->size - 1;
  new_ht->used = ht->used;
  new_ht->table = calloc(new_ht->size, sizeof(hash_entry *));
  if (!new_ht->table) {
    free(new_ht);
    return HT_ERR;
  }

  for (unsigned long i = 0; i < ht->size; i++) {
    hash_entry *entry = ht->table[i];
    while (entry) {
      ht_add(new_ht, entry->key, entry->val);
      entry = entry->next;
      ht->used--;
    }
  }

  assert(ht->used == 0);
  free(ht->table);

  ht = new_ht;

  return HT_OK;
}

/* Add a new key-value pair to the hash table */
static int ht_add(hash_table *ht, void *key, void *val) {
  int index = _ht_get_index(ht, key);
  if (index == -1) return HT_ERR;

  hash_entry *entry = malloc(sizeof(hash_entry));
  if (!entry) return HT_ERR;

  entry->key = key;
  entry->val = val;
  entry->next = ht->table[index];
  ht->table[index] = entry;
  ht->used++;

  return HT_OK;
}

/* Add an element, discarding the old if the key already exists.
 * Return 1 if the key was added from scratch, 0 if there was already an
 * element with such key and ht_replace() just performed a value update
 * operation. */
static int ht_replace(hash_table *ht, void *key, void *val) {
  if (ht_add(ht, key, val) == HT_OK) return 1;

  hash_entry *entry = ht_find(ht, key);
  if (!entry) return 0;

  entry->val = val;

  return 0;
}

/* Delete an element from the hash table */
static int ht_delete(hash_table *ht, const void *key) {
  if (ht->used == 0) return HT_ERR;

  unsigned long index = GET_HASH_INDEX(ht, key);

  hash_entry *entry = ht->table[index];
  hash_entry *prev = NULL;
  while (entry) {
    if (!strcmp(entry->key, key)) {
      if (prev) {
        prev->next = entry->next;
      } else {
        ht->table[index] = entry->next;
      }
      free(entry);
      ht->used--;
      return HT_OK;
    }
    prev = entry;
    entry = entry->next;
  }

  return HT_ERR; /* Not found */
}

/* Clear & Release the hash table */
static void ht_release(hash_table *ht) {
  for (unsigned long i = 0; i < ht->size; i++) {
    hash_entry *entry = ht->table[i];
    while (entry) {
      hash_entry *next = entry->next;
      free(entry);
      entry = next;
    }
  }
  free(ht->table);
  free(ht);
}

/* Find an element in the hash table */
static hash_entry *ht_find(hash_table *ht, const void *key) {
  unsigned long index = GET_HASH_INDEX(ht, key);

  hash_entry *entry = ht->table[index];
  while (entry) {
    if (!strcmp(entry->key, key)) return entry;
    entry = entry->next;
  }

  return NULL;
}

/* Our hash table capability is a power of two */
static unsigned long _ht_next_power(unsigned long size) {
  unsigned long i = HT_INITIAL_SIZE;

  if (size >= LONG_MAX) return LONG_MAX;
  while (1) {
    if (i >= size) return i;
    i *= 2;
  }
}

/* Returns the index of a free slot that can be populated with
 * an hash entry for the given 'key'.
 * If the key already exists, -1 is returned. */
static unsigned long _ht_get_index(hash_table *ht, const void *key) {
  if (ht->used >= ht->size) {
    if (!ht_expand(ht, ht->size * 2)) return -1;
  }

  unsigned long index = GET_HASH_INDEX(ht, key);

  hash_entry *entry = ht->table[index];
  while (entry) {
    if (!strcmp(entry->key, key)) return -1;
    entry = entry->next;
  }

  return index;
}