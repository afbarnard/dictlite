// Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
// LICENSE.txt for details.

#include <stdio.h>
#include <string.h>

#include "dictlite.h"

#define STRING_BUFFER_LENGTH 256
static char keyStringBuffer[STRING_BUFFER_LENGTH];
static char valueStringBuffer[STRING_BUFFER_LENGTH];
static const char * const defaultString = "<unknown-type>";

typedef enum {
  STRING,
  BOXINT
} Types;

struct Boxint {
  int value;
};

static void object_to_string(void * object, Types typeCode, char * stringBuffer, size_t stringBufferLength)
{
  int numBytes;
  switch (typeCode) {
  case STRING:
    numBytes = snprintf(stringBuffer, stringBufferLength, "\"%s\"", (char *) object);
    break;
  case BOXINT:
    numBytes = snprintf(stringBuffer, stringBufferLength, "%d", ((struct Boxint *) object)->value);
    break;
  default:
    numBytes = snprintf(stringBuffer, stringBufferLength, defaultString);
    break;
  }
  if (numBytes < 0)
    printf("Error in snprintf.\n");
  else if (numBytes >= stringBufferLength)
    stringBuffer[stringBufferLength - 1] = '\0';
}

static void dictlite_print(Dictlite * dict, Types keyTypeCode, Types valueTypeCode)
{
  printf("dictlite[%zd] {\n", dictlite_size(dict));
  DictliteItemIterator iterator = dictlite_itemIterator(dict);
  MappingItem * item;
  while ((item = dictlite_itemIterator_next(&iterator))) {
    object_to_string(item->key, keyTypeCode, keyStringBuffer, STRING_BUFFER_LENGTH);
    object_to_string(item->value, valueTypeCode, valueStringBuffer, STRING_BUFFER_LENGTH);
    printf("  %s: %s,\n", keyStringBuffer, valueStringBuffer);
  }
  printf("}\n");
}

static int compare_string_string(void * string1, void * string2)
{
  char * str1 = (char *) string1;
  char * str2 = (char *) string2;
  return strcmp(str1, str2);
}

static int compare_boxint_boxint(void * boxint1, void * boxint2)
{
  struct Boxint * bint1 = (struct Boxint *) boxint1;
  struct Boxint * bint2 = (struct Boxint *) boxint2;
  return (bint1->value) - (bint2->value);
}

int main()
{
  // Data
  char * drugs[10] = {
    "ACE Inhibitor",
    "Amphoceterin B",
    "Antibiotics",
    "Antiepileptics",
    "Benzodiazepines",
    "Beta Blockers",
    "Bisphosphonates",
    "Tricyclic Antidepressants",
    "Typical Antipsychotics",
    "Warfarin"
  };

  char * conds[10] = {
    "Angioedema",
    "Acute renal failure",
    "Acute liver failure",
    "Aplastic anemia",
    "Hip fracture",
    "None",
    "Upper GI ulcer",
    "Acute myocardial infarction",
    "Acute myocardial infarction",
    "Bleeding"
  };

  struct Boxint ints[5] = {
    {7},
    {92},
    {435},
    {9338},
    {12252}
  };

  struct Boxint * ids1[5] = {
    &ints[0],
    &ints[1],
    &ints[2],
    &ints[3],
    &ints[4],
  };

  char * ids2[5] = {
    "Rome",
    "Calcutta",
    "Sao Paulo",
    "Istanbul",
    "Tbilisi"
  };

  int rv = 0;

  // Create the dicts
  Dictlite * dl1 = dictlite_new(compare_string_string);
  if (dl1 == NULL) {
    printf("Failed to allocate dict 1.\n");
    rv = 1;
    goto finally;
  }
  Dictlite * dl2 = dictlite_new(compare_boxint_boxint);
  if (dl2 == NULL) {
    printf("Failed to allocate dict 2.\n");
    rv = 1;
    goto finally;
  }
  Dictlite * dl3 = dictlite_new(compare_string_string);
  if (dl2 == NULL) {
    printf("Failed to allocate dict 3.\n");
    rv = 1;
    goto finally;
  }

  // Populate the dicts
  int index;
  for (index = 0; index < 10; ++index) {
    dictlite_setValue(dl1, drugs[index], conds[index]);
  }
  for (index = 0; index < 5; ++index) {
    dictlite_setValue(dl2, ids1[index], ids2[index]);
  }
  dictlite_setValue(dl3, "song: 1901", "album: Wolfgang Amadeus Phoenix");
  dictlite_setValue(dl3, "song: Lisztomania", "album: Wolfgang Amadeus Phoenix");
  dictlite_setValue(dl3, "album: Wolfgang Amadeus Phoenix", "band: Phoenix");
  dictlite_setValue(dl3, "song: Long Distance Call", "album: It's Never Been Like That");
  dictlite_setValue(dl3, "song: Sometimes In The Fall", "album: It's Never Been Like That");
  dictlite_setValue(dl3, "album: It's Never Been Like That", "band: Phoenix");

  // Print the dict contents
  dictlite_print(dl1, STRING, STRING);
  printf("\n");

  // Try out the operations
  printf("contains \"%s\"?: %d\n", "Rofecoxib", dictlite_contains(dl1, "Rofecoxib"));
  dictlite_setValue(dl1, "Rofecoxib", "None");
  printf("contains \"%s\"?: %d\n", "Rofecoxib", dictlite_contains(dl1, "Rofecoxib"));
  dictlite_setValue(dl1, "ACE Inhibitor", "Bleeding");
  printf("\n");

  // Print the dict contents
  dictlite_print(dl1, STRING, STRING);
  printf("\n");

  dictlite_print(dl2, BOXINT, STRING);
  printf("\n");

  struct Boxint intkey1 = {7};
  struct Boxint intkey2 = {12252};
  struct Boxint intkey3 = {39912};

  printf("contains %d?: %d; value: \"%s\"\n", intkey1.value, dictlite_contains(dl2, &intkey1),
	 (char *) dictlite_getValue(dl2, &intkey1));
  printf("contains %d?: %d; value: \"%s\"\n", intkey2.value, dictlite_contains(dl2, &intkey2),
	 (char *) dictlite_getValue(dl2, &intkey2));
  printf("contains %d?: %d; value: \"%s\"\n", intkey3.value, dictlite_contains(dl2, &intkey3),
	 (char *) dictlite_getValue(dl2, &intkey3));
  dictlite_setValue(dl2, &intkey3, "Beijing");
  printf("contains %d?: %d; value: \"%s\"\n", intkey3.value, dictlite_contains(dl2, &intkey3),
	 (char *) dictlite_getValue(dl2, &intkey3));
  printf("\n");

  dictlite_print(dl2, BOXINT, STRING);
  printf("\n");

  // Combine some dicts
  dictlite_print(dl3, STRING, STRING);
  printf("\n");
  dictlite_addFromDict(dl3, dl1);
  dictlite_print(dl3, STRING, STRING);
  printf("\n");
  dictlite_print(dl1, STRING, STRING);
  printf("\n");

 finally:
  // Free the dicts (the contents aren't dynamic so we don't need to free them)
  if (dl1 != NULL)
    dictlite_del(dl1);
  if (dl2 != NULL)
    dictlite_del(dl2);
  if (dl3 != NULL)
    dictlite_del(dl3);

  return rv;
}
