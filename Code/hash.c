#include "hash.h"

/* -------------------------- private prototypes ---------------------------- */

static unsigned long _htNextPower(unsigned long size);
static unsigned long _htGetIndex(HashTable *ht, const void *key);
static void _htReset(HashTable *ht);
static int _htInit(HashTable *ht, HtType *type, void *privDataPtr);
static int _htClear(HashTable *ht);

/* -------------------------- hash functions -------------------------------- */

/* Generic hash function (a popular one from Bernstein) */
unsigned int htGenHashFunction(const char *str) {
  unsigned int hash = 5381;

  while (*str) hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
  return hash;
}

/* ----------------------------- API implementation ------------------------- */

/* Create a new hash table */
HashTable *htCreate(HtType *type, void *privDataPtr) {
  HashTable *ht = malloc(sizeof(HashTable));
  if (!ht) return NULL;

  _htInit(ht, type, privDataPtr);

  return ht;
}

/* Expand the hash table */
int htExpand(HashTable *ht, unsigned long size) {
  HashTable new_ht; /* new hash table */

  unsigned long realsize = _htNextPower(size);
  if (ht->used > size) return HT_ERR;

  _htInit(&new_ht, ht->type, ht->privdata);
  new_ht.size = realsize;
  new_ht.mask = realsize - 1;
  new_ht.used = ht->used;
  new_ht.table = calloc(realsize, sizeof(HashEntry *));
  if (!new_ht.table) {
    return HT_ERR;
  }

  /* Copy all the elements from the old to the new table:
   * note that if the old hash table is empty ht->size is zero,
   * so ht_xpand just creates an hash table. */
  for (unsigned long i = 0; i < ht->size && ht->used > 0; i++) {
    HashEntry *entry = ht->table[i];
    while (entry) {
      HashEntry *next = entry->next;

      unsigned long index = htHashKey(ht, entry->key) & new_ht.mask;
      entry->next = new_ht.table[index];
      new_ht.table[index] = entry;
      ht->used--;

      entry = next;
    }
  }

  assert(ht->used == 0);
  free(ht->table);
  free(ht);

  /* Remap the new hashtable in the old */
  *ht = new_ht;

  return HT_OK;
}

/* Add a new key-value pair to the hash table */
int htAdd(HashTable *ht, void *key, void *val) {
  /* Get the index of the new element, or -1 if
   * the element already exists. */
  int index = _htGetIndex(ht, key);
  if (index == -1) return HT_ERR;

  /* Allocates the memory and stores key */
  HashEntry *entry = malloc(sizeof(HashEntry));
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
 * element with such key and htReplace() just performed a value update
 * operation. */
int htReplace(HashTable *ht, void *key, void *val) {
  /* Try to add the element. If the key
   * does not exists htAdd will succeed. */
  if (htAdd(ht, key, val) == HT_OK) return 1;

  /* It already exists, get the entry */
  HashEntry *entry = htFind(ht, key);
  if (!entry) return 0;

  /* Free the old value and set the new one */
  /* Set the new value and free the old one. Note that it is important
   * to do that in this order, as the value may just be exactly the same
   * as the previous one. In this context, think to reference counting,
   * you want to increment (set), and then decrement (free), and not the
   * reverse. */
  HashEntry old = *entry;
  htSetHashVal(ht, entry, val);
  htFreeEntryVal(ht, &old);

  return 0;
}

/* Delete an element from the hash table */
int htDelete(HashTable *ht, const void *key) {
  if (ht->used == 0) return HT_ERR;

  unsigned long index = htHashKey(ht, key) & ht->mask;

  HashEntry *entry = ht->table[index];
  HashEntry *prev = NULL;
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
void htRelease(HashTable *ht) {
  _htClear(ht);
  free(ht);
}

/* Find an element in the hash table */
HashEntry *htFind(HashTable *ht, const void *key) {
  if (ht->used == 0) return NULL;

  unsigned long index = htHashKey(ht, key) & ht->mask;

  HashEntry *entry = ht->table[index];
  while (entry) {
    if (htCompareHashKeys(ht, key, entry->key)) return entry;
    entry = entry->next;
  }

  return NULL;
}

/* ------------------------- private functions ------------------------------ */

/* Our hash table capability is a power of two */
static unsigned long _htNextPower(unsigned long size) {
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
static unsigned long _htGetIndex(HashTable *ht, const void *key) {
  if (ht->used >= ht->size) {
    if (htExpand(ht, ht->size * 2) == HT_ERR) return -1;
  }

  unsigned long index = htHashKey(ht, key) & ht->mask;

  HashEntry *entry = ht->table[index];
  while (entry) {
    if (htCompareHashKeys(ht, key, entry->key)) return -1;
    entry = entry->next;
  }

  return index;
}

/* Reset an hashtable already initialized with ht_init(). */
static void _htReset(HashTable *ht) {
  ht->table = NULL;
  ht->size = 0;
  ht->mask = 0;
  ht->used = 0;
}

/* Initialize the hash table */
static int _htInit(HashTable *ht, HtType *type, void *privDataPtr) {
  _htReset(ht);
  ht->type = type;
  ht->privdata = privDataPtr;
  return HT_OK;
}

/* Destroy an entire hash table */
static int _htClear(HashTable *ht) {
  /* Free all the elements */
  for (unsigned long i = 0; i < ht->size; i++) {
    HashEntry *entry = ht->table[i];
    while (entry) {
      HashEntry *next = entry->next;
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
  _htReset(ht);

  return HT_OK; /* never fails */
}
