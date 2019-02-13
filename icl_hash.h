/**
 * @file icl_hash.h
 * 
 * Implementazione di una hash table tratta da 
 * http://icl.cs.utk.edu/plasma/docs/icl__hash_8c.html
 * 
 * Le uniche modifiche che sono state apportate sono riguardano la
 * documentazione.
 * 
 * Segue l'intestazione dell'originale:
 * ---------------------------------------------------------
 * 
 * Dependency free hash table implementation.
 *
 * @author Jakub Kurzak
 *
 * $Id: icl_hash.c 2838 2011-11-22 04:25:02Z mfaverge $
 * $UTK_Copyright: $ 
 */
#ifndef icl_hash_h
#define icl_hash_h

#include <stdio.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

typedef struct icl_entry_s {
    void* key;
    void *data;
    struct icl_entry_s* next;
} icl_entry_t;

typedef struct icl_hash_s {
    int nbuckets;
    int nentries;
    icl_entry_t **buckets;
    unsigned int (*hash_function)(void*);
    int (*hash_key_compare)(void*, void*);
} icl_hash_t;

/**
 * Create a new hash table.
 *
 * Se hash_function == NULL verrà impiegata la funzione di hash di
 * default hash_pjw definita in icl_hash.c
 * 
 * Se hash_key_compare == NULL verrà impiegata l'usuale funzione di
 * comparazione lessicografica tra stringhe
 * 
 * @param[in] nbuckets -- number of buckets to create
 * @param[in] hash_function -- pointer to the hashing function to be used
 * @param[in] hash_key_compare -- pointer to the hash key comparison function to be used
 *
 * @returns pointer to new hash table.
 */
icl_hash_t *
icl_hash_create( int nbuckets, unsigned int (*hash_function)(void*), int (*hash_key_compare)(void*, void*) );

/**
 * Search for an entry in a hash table.
 *
 * @param ht -- the hash table to be searched
 * @param key -- the key of the item to search for
 *
 * @returns pointer to the data corresponding to the key.
 *   If the key was not found, returns NULL.
 */
void *
icl_hash_find(icl_hash_t *ht, void* key);

/**
 * Insert an item into the hash table.
 *
 * @param ht -- the hash table
 * @param key -- the key of the new item
 * @param data -- pointer to the new item's data
 *
 * @returns pointer to the new item.  Returns NULL on error.
 */
icl_entry_t *
icl_hash_insert(icl_hash_t *ht, void* key, void *data);

/**
 * Free one hash table entry located by key (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param key -- the key of the new item
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int
icl_hash_destroy(icl_hash_t *ht, void (*free_key)(void*), void (*free_data)(void*));

/**
 * Dump the hash table's contents to the given file pointer.
 *
 * @param stream -- the file to which the hash table should be dumped
 * @param ht -- the hash table to be dumped
 *
 * @returns 0 on success, -1 on failure.
 */
int
icl_hash_dump(FILE* stream, icl_hash_t* ht);

/**
 * Free hash table structures (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int icl_hash_delete(icl_hash_t *ht, void* key, void (*free_key)(void*), void (*free_data)(void*));

#define icl_hash_foreach(ht, tmpint, tmpent, kp, dp)    \
    for (tmpint=0;tmpint<ht->nbuckets; tmpint++)        \
        for (tmpent=ht->buckets[tmpint];                                \
             tmpent!=NULL&&((kp=tmpent->key)!=NULL)&&((dp=tmpent->data)!=NULL); \
             tmpent=tmpent->next)


#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* icl_hash_h */
