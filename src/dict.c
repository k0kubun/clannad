// Dict is an associated list of void pointers, which can compose a tree structure.

#include <stdlib.h>
#include <string.h>
#include "clannad.h"

const int ENTRY_NOT_FOUND = -1;

typedef struct {
  char *key;
  void *data;
} DictEntry;

Dict*
create_dict()
{
  Dict *ret = malloc(sizeof(Dict));
  ret->entries = create_vector();
  ret->parent  = NULL;
  return ret;
}

int
dict_index(Dict *dict, char *key)
{
  for (int i = 0; i < dict->entries->length; i++) {
    DictEntry *entry = vector_get(dict->entries, i);
    if (entry->key && strcmp(entry->key, key) == 0) return i;
  }
  return ENTRY_NOT_FOUND;
}

void*
dict_get(Dict *dict, char *key)
{
  int i = dict_index(dict, key);
  if (i == ENTRY_NOT_FOUND) {
    if (dict->parent) {
      return dict_get(dict->parent, key);
    } else {
      return NULL;
    }
  } else {
    DictEntry *entry = vector_get(dict->entries, i);
    return entry->data;
  }
}

void
dict_set(Dict *dict, char *key, void *value)
{
  int i = dict_index(dict, key);
  if (i == ENTRY_NOT_FOUND) {
    DictEntry *entry = malloc(sizeof(DictEntry));
    entry->key  = key;
    entry->data = value;
    vector_push(dict->entries, entry);
  } else {
    DictEntry *entry = vector_get(dict->entries, i);
    entry->data = value;
    dict->entries->data[i] = entry;
  }
}

bool
dict_delete(Dict *dict, char *key)
{
  int i = dict_index(dict, key);
  if (i != ENTRY_NOT_FOUND) {
    DictEntry *entry = vector_get(dict->entries, i);
    entry->key = NULL;
    return true;
  } else {
    return false;
  }
}
