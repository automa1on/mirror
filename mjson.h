#pragma once
#include "cJSON.h"

typedef bool mjson_alloc_callback(struct _mirror_struct *strct, int field_idx, int nmemb, int size, void *data);

cJSON *mjson_serialize_struct(struct _mirror_struct *const strct, const void *const pdata);
void mjson_deserialize_struct(struct _mirror_struct *strct, void *pdata, cJSON *json, mjson_alloc_callback *alloc_cb);
