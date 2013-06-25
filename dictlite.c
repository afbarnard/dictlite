// Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
// LICENSE.txt for details.

#include <stdio.h>
#include <stdlib.h>

#include "dictlite.h"


////////////////////////////////////////
// Dictlite
////////////////////////////////////////

static MappingItem * dictlite_findItem(Dictlite * dict, void * key)
{
  MappingItem * item = dict->head;
  while (item != NULL) {
    // Handle errors?
    if ((dict->compareKeys)(item->key, key) == 0)
      return item;
    item = item->next;
  }
  return NULL;
}

static MappingItem * dictlite_insertItem(Dictlite * dict, void * key, void * value)
{
  // Create and populate a new mapping item
  MappingItem * item = (MappingItem *) malloc(sizeof(MappingItem));
  if (item == NULL)
    return NULL;
  item->key = key;
  item->value = value;
  item->next = NULL;

  if (dict->head == NULL) {
    // Insert the item as the only item
    dict->head = item;
    dict->end = item;
    dict->size = 1;
  } else {
    // Insert the item as the new last item
    dict->end->next = item;
    dict->end = item;
    ++(dict->size);
  }
  return item;
}

static int dictlite_identityComparison(void * key1, void * key2)
{
  return key1 - key2;
}

Dictlite * dictlite_new(int (* key_comparison_function)(void * key1, void * key2))
{
  Dictlite * dict = (Dictlite *) malloc(sizeof(Dictlite));
  if (dict == NULL)
    return NULL;
  dict->head = NULL;
  dict->end = NULL;
  dict->size = 0;
  dict->compareKeys = (key_comparison_function != NULL ?
		       key_comparison_function :
		       dictlite_identityComparison);
  return dict;
}

void dictlite_del(Dictlite * dict)
{
  if (dict == NULL)
    return;

  // This function assumes that the API user has already deleted the keys and items as necessary
  // Delete the mapping items
  MappingItem * item = dict->head;
  MappingItem * toFree;
  while (item != NULL) {
    toFree = item;
    item = item->next;
    free(toFree);
  }
  // Delete the dict
  free(dict);
}

size_t dictlite_size(Dictlite * dict)
{
  return dict->size;
}

int dictlite_contains(Dictlite * dict, void * key)
{
  return (dictlite_findItem(dict, key) != NULL);
}

void * dictlite_getValue(Dictlite * dict, void * key)
{
  MappingItem * item = dictlite_findItem(dict, key);
  if (item == NULL)
    return NULL;
  return item->value;
}

void * dictlite_setValue(Dictlite * dict, void * key, void * value)
{
  MappingItem * item = dictlite_findItem(dict, key);
  if (item == NULL) {
    // Insert a new mapping (don't bother to check whether malloc failed)
    dictlite_insertItem(dict, key, value);
    return NULL;
  } else {
    // Replace the value
    void * oldValue = item->value;
    item->value = value;
    return oldValue;
  }
}

MappingItem * dictlite_delItem(Dictlite * dict, void * key)
{
  MappingItem * previous = NULL;
  MappingItem * current = dict->head;
  while (current != NULL) {
    if ((dict->compareKeys)(current->key, key) == 0) {
      // Remove the mapping item from the list
      if (previous == NULL) {
	// The item is the first item so modify the dict
	dict->head = current->next;
      } else {
	// Skip the current item
	previous->next = current->next;
      }
      // Update the end pointer
      if (current->next == NULL) {
	dict->end = previous;
      }
      // Return the mapping item
      current->next = NULL;
      return current;
    }
  }
  return NULL;
}

void dictlite_addFromDict(Dictlite * dict, Dictlite * otherDict)
{
  DictliteItemIterator iterator = dictlite_itemIterator(otherDict);
  MappingItem * item;
  while ((item = dictlite_itemIterator_next(&iterator))) {
    dictlite_setValue(dict, item->key, item->value);
  }
}

DictliteItemIterator dictlite_itemIterator(Dictlite * dict)
{
  DictliteItemIterator iterator = {dict->head};
  return iterator;
}

MappingItem * dictlite_itemIterator_next(DictliteItemIterator * iterator)
{
  MappingItem * item = iterator->nextItem;
  if (item != NULL)
    iterator->nextItem = item->next;
  return item;
}
