#include "hash.h"

/* -------------------------- private prototypes ---------------------------- */

static unsigned long _ht_next_power(unsigned long size);
static unsigned long _ht_get_index(hash_table *ht, const void *key);
static void _ht_reset(hash_table *ht);
static int _ht_init(hash_table *ht, ht_type *type, void *privdata_ptr);
static int _ht_clear(hash_table *ht);

/* -------------------------- hash functions -------------------------------- */

/* Generic hash function (a popular one from Bernstein) */
static unsigned int ht_gen_hash_function(const char *str) {
  unsigned int hash = 5381;

  while (*str) hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
  return hash;
}

/* ----------------------------- API implementation ------------------------- */

/* Create a new hash table */
static hash_table *ht_create(ht_type *type, void *privdata_ptr) {
  hash_table *ht = malloc(sizeof(hash_table));
  if (!ht) return NULL;

  _ht_init(ht, type, privdata_ptr);

  return ht;
}

/* Expand the hash table */
static int ht_expand(hash_table *ht, unsigned long size) {
  hash_table *new_ht = malloc(sizeof(hash_table)); /* new hash table */
  if (!new_ht) return HT_ERR;

  _ht_init(new_ht, ht->type, ht->privdata);
  new_ht->size = _ht_next_power(size);
  new_ht->mask = new_ht->size - 1;
  new_ht->used = ht->used;
  new_ht->table = calloc(new_ht->size, sizeof(hash_entry *));
  if (!new_ht->table) {
    free(new_ht);
    return HT_ERR;
  }

  /* Copy all the elements from the old to the new table:
   * note that if the old hash table is empty ht->size is zero,
   * so ht_xpand just creates an hash table. */
  for (unsigned long i = 0; i < ht->size; i++) {
    hash_entry *entry = ht->table[i];
    while (entry) {
      hash_entry *next = entry->next;

      unsigned long index = htHashKey(ht, entry->key) & new_ht->mask;
      entry->next = new_ht->table[index];
      new_ht->table[index] = entry;
      ht->used--;

      entry = next;
    }
  }

  assert(ht->used == 0);
  free(ht->table);
  free(ht);

  ht = new_ht;

  return HT_OK;
}

/* Add a new key-value pair to the hash table */
static int ht_add(hash_table *ht, void *key, void *val) {
  /* Get the index of the new element, or -1 if
   * the element already exists. */
  int index = _ht_get_index(ht, key);
  if (index == -1) return HT_ERR;

  /* Allocates the memory and stores key */
  hash_entry *entry = malloc(sizeof(hash_entry));
  if (!entry) return HT_ERR;

  entry->next = ht->table[index];
  ht->table[index] = entry;

  htSetHashKey(ht, entry, key);
  htSetHashVal(ht, entry, val);
  ht->used++;

  return HT_OK;
}

/* Add an element, discarding the old if the key already exists.
 * Return 1 if the key was added from scratch, 0 if there was already an
 * element with such key and ht_replace() just performed a value update
 * operation. */
static int ht_replace(hash_table *ht, void *key, void *val) {
  /* Try to add the element. If the key
   * does not exists ht_add will succeed. */
  if (ht_add(ht, key, val) == HT_OK) return 1;

  /* It already exists, get the entry */
  hash_entry *entry = ht_find(ht, key);
  if (!entry) return 0;

  /* Free the old value and set the new one */
  /* Set the new value and free the old one. Note that it is important
   * to do that in this order, as the value may just be exactly the same
   * as the previous one. In this context, think to reference counting,
   * you want to increment (set), and then decrement (free), and not the
   * reverse. */
  void *old_val = entry->val;
  htSetHashVal(ht, entry, val);
  htFreeEntryVal(ht, entry);

  return 0;
}

/* Delete an element from the hash table */
static int ht_delete(hash_table *ht, const void *key) {
  if (ht->used == 0) return HT_ERR;

  unsigned long index = htHashKey(ht, key) & ht->mask;

  hash_entry *entry = ht->table[index];
  hash_entry *prev = NULL;
  while (entry) {
    if (htCompareHashKeys(ht, entry->key, key)) {
      if (prev) {
        prev->next = entry->next;
      } else {
        ht->table[index] = entry->next;
      }

      htFreeEntryKey(ht, entry);
      htFreeEntryVal(ht, entry);
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
  _ht_clear(ht);
  free(ht);
}

/* Find an element in the hash table */
static hash_entry *ht_find(hash_table *ht, const void *key) {
  if (ht->used == 0) return NULL;

  unsigned long index = htHashKey(ht, key) & ht->mask;

  hash_entry *entry = ht->table[index];
  while (entry) {
    if (htCompareHashKeys(ht, key, entry->key)) return entry;
    entry = entry->next;
  }

  return NULL;
}

/* ------------------------- private functions ------------------------------ */

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

  unsigned long index = htHashKey(ht, key) & ht->mask;

  hash_entry *entry = ht->table[index];
  while (entry) {
    if (htCompareHashKeys(ht, key, entry->key)) return -1;
    entry = entry->next;
  }

  return index;
}

/* Reset an hashtable already initialized with ht_init(). */
static void _ht_reset(hash_table *ht) {
  ht->table = NULL;
  ht->size = 0;
  ht->mask = 0;
  ht->used = 0;
}

/* Initialize the hash table */
static int _ht_init(hash_table *ht, ht_type *type, void *privdata_ptr) {
  _ht_reset(ht);
  ht->type = type;
  ht->privdata = privdata_ptr;
  return HT_OK;
}

/* Destroy an entire hash table */
static int _ht_clear(hash_table *ht) {
  /* Free all the elements */
  for (unsigned long i = 0; i < ht->size; i++) {
    hash_entry *entry = ht->table[i];
    while (entry) {
      hash_entry *next = entry->next;
      htFreeEntryKey(ht, entry);
      htFreeEntryVal(ht, entry);
      free(entry);
      ht->used--;
      entry = next;
    }
  }

  /* Free the table and the allocated cache structure */
  free(ht->table);
  /* Re-initialize the table */
  _ht_reset(ht);

  return HT_OK; /* never fails */
}
