#include "cJSON.h"
#include "mirror.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mjson.h"

cJSON *mjson_serialize_struct(struct _mirror_struct *const strct, const void *const pdata) {
	cJSON *json = cJSON_CreateObject();
	for(int i = 0; i < strct->entries_len; i++) {
		struct _mirror_entry *e = &strct->entries[i];

		double num = NAN;

		if(MirrorFieldIsStr(strct, i)) {
			const char *const *str = pdata + e->offset;
			cJSON_AddStringToObject(json, e->name, *str);
		}
		else if(MirrorFieldIsStruct(strct, i)) {
			/*
				TODO: Doesn't support nested arrays or pointers. (same thing)
			*/
			//printf("ğeğeğe\n");
			void *strctptr;
			if(e->loi == 0) {
				strctptr = e->offset + (char*)pdata;
				cJSON_AddItemReferenceToObject(json, e->name, 
					mjson_serialize_struct(MirrorGetStruct(e->type.structname), strctptr));
			} else if(e->loi == 1) {
				strctptr = *(void**)(char*)( e->offset + (char*)pdata);
				if(!MirrorFieldIsArr(strct, i) && _MirrorGetFieldArrLen(strct, i, pdata) < 2) { // not array
					
					cJSON_AddItemReferenceToObject(json, e->name, 
						mjson_serialize_struct(MirrorGetStruct(e->type.structname), strctptr));
				} else {
					cJSON *arr = cJSON_AddArrayToObject(json, e->name);
					int arrlen = _MirrorGetFieldArrLen(strct, i, pdata);
					for(int j = 0; j < arrlen; j++) {
						cJSON_AddItemReferenceToArray(arr, 
						mjson_serialize_struct(MirrorGetStruct(e->type.structname), strctptr));
					}
				}
			} else assert(false);
			
		}
		else if(MirrorFieldIsArr(strct, i)) {
			if(MirrorFieldIsNum(strct, i)) {
				cJSON *arr = cJSON_CreateArray();
				for(int j = 0; j < _MirrorGetFieldArrLen(strct, i, pdata); j++) {
					
					int ret = _MirrorGetFieldArrNum(strct, i, j, pdata, &num);
					assert(ret == MIRROR_SUCCESS);

					cJSON *arrval = cJSON_CreateNumber(num);
					cJSON_AddItemToArray(arr, arrval);
				}

				cJSON_AddItemReferenceToObject(json, e->name, arr);
			} else assert(false);
		}
		else if(MirrorFieldIsNum(strct, i)) {
			
			MirrorGetFieldNum(strct, e->name, pdata, &num);
			cJSON_AddNumberToObject(json, e->name, num);
		}

		//printf("%s", e->type.structname ? e->type.structname : "nul");
	}
	
	return json;
}

void mjson_deserialize_struct(struct _mirror_struct *strct, void *pdata, cJSON *json, mjson_alloc_callback *alloc_cb) {
	for(int i = 0; i < strct->entries_len; i++) {
		struct _mirror_entry *e = &strct->entries[i];
		cJSON *jsonent = cJSON_GetObjectItemCaseSensitive(json, e->name);
		assert(jsonent != NULL);

		if(cJSON_IsString(jsonent)) {
			const char *str = cJSON_GetStringValue(jsonent);
			{
				int size = _MirrorGetFieldArrLen(strct, i, pdata);
				if(size > -1)
					if(strlen(str) +1 != size) {
						alloc_cb(strct, i, 1, strlen(str) +1, pdata);
					}
			}

			char *const *spstr;
			if(!e->isfarr)
				spstr = (void*)( (char*)pdata + e->offset);
			else {
				static char *ss;
				ss = pdata;
				ss += e->offset;
				spstr = &ss;
			}
			if(*spstr == NULL)
				alloc_cb(strct, i, 1, strlen(str) +1, pdata);

			memcpy(*spstr, str, strlen(str) +1);
		}
		else if(cJSON_IsArray(jsonent)) {
			int j = 0;
			cJSON *elem;
			{
				void *ptr = (void*)( (char*)pdata + e->offset);
				if(ptr == NULL) {
					alloc_cb(strct, i, e->type.size, cJSON_GetArraySize(jsonent), pdata);
				}
			}
			cJSON_ArrayForEach(elem, jsonent) {
				if(cJSON_IsNumber(elem)) {
					MirrorSetFieldArrNum(strct, e->name, j, pdata, cJSON_GetNumberValue(elem));
				}

				j++;
			}
		}
		else if(cJSON_IsNumber(jsonent)) {
			MirrorSetFieldNum(strct, e->name, pdata, cJSON_GetNumberValue(jsonent));
		}
		else if(cJSON_IsObject(jsonent)) {
			mjson_deserialize_struct(MirrorGetStruct(e->type.structname), 
				e->offset + (char*)pdata, 
				jsonent, alloc_cb);
		}
	}
}
