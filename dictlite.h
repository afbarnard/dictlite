// Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
// LICENSE.txt for details.

#ifndef __DICTLITE_H__
#define __DICTLITE_H__

#include <stddef.h>


/*
 * Dictlite
 *
 * A lightweight dictionary for small, flexible mappings.
 */

/* The items in the dictionary that store the mappings. */
struct dictlite_MappingItem {
  void * key;
  void * value;
  struct dictlite_MappingItem * next;
};
typedef struct dictlite_MappingItem MappingItem;

/* The dictionary data, like a linked list. */
struct dictlite_Dictlite {
  MappingItem * head;
  MappingItem * end;
  size_t size;
  int (* compareKeys)(void * key1, void * key2);
};
typedef struct dictlite_Dictlite Dictlite;

/* Create a new dict.  The key comparison function should work like an
 * ordered comparison function.  That is, return 0 for equality and
 * other values for inequality (typically -1 if key1 < key2 and 1 if
 * key1 > key2).  If the key comparison function is null, an identity
 * comparison function is used.  O(1).
 */
Dictlite * dictlite_new(int (* key_comparison_function)(void * key1, void * key2));

/* Free a dict.  This does not free the keys or items.  The API user is
 * responsible for doing that (if necessary) prior to freeing the
 * dict.  O(n).
 */
void dictlite_del(Dictlite * dict);

/* Return the size of a dict.  O(1). */
size_t dictlite_size(Dictlite * dict);

/* Return whether the dict contains a key.  O(n). */
int dictlite_contains(Dictlite * dict, void * key);

/* Gets the value associated with a key.  O(n). */
void * dictlite_getValue(Dictlite * dict, void * key);

/* Sets the value associated with a key.  Adds the key if it is not
 * already present.  Returns the previous value or null if there was no
 * previous value.  O(n).
 */
void * dictlite_setValue(Dictlite * dict, void * key, void * item);

/* Removes the given key and associated item from the dict.  Returns the
 * mapping item containing the key and value or null if there was no
 * such key.  The caller is responsible for freeing the returned mapping
 * item because the caller is responsible for freeing the key and value
 * (if needed) and the mapping item is the easiest container to return
 * them in.  Note that the key in the mapping item and the key given to
 * the function may be differenct objects.  O(n).
 */
MappingItem * dictlite_delItem(Dictlite * dict, void * key);

/* Adds the mappings in the other dict to this dict.  Updates any
 * existing mappings to those in the other dict.
 */
void dictlite_addFromDict(Dictlite * dict, Dictlite * otherDict);

/* Iteration support */

/* Iterator for items ((key, value) pairs) */
struct dictlite_ItemIterator {
  MappingItem * nextItem;
};
typedef struct dictlite_ItemIterator DictliteItemIterator;

/* Return a new iterator.  The iterator is not dynamically allocated, so
 * do not free it.
 */
DictliteItemIterator dictlite_itemIterator(Dictlite * dict);

/* The returned pointers point to live MappingItems in the dictionary.
 * This was done to allow flexibility.  Keys and values may be changed
 * and those changes will be reflected in the dictionary, but be careful
 * (e.g. don't create duplicate mappings).
 */
MappingItem * dictlite_itemIterator_next(DictliteItemIterator * iterator);

#endif
