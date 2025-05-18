#include "cJSON.h"
#include "mirror.h"

static cJSON *mjson_serialize_arr;

cJSON *mjson_serialize_struct(struct _mirror_struct *strct, const void *pdata) {
	cJSON *json = cJSON_CreateObject();
	for(int i = 0; i < strct->entries_len; i++) {
		if(MirrorFieldIsNum(strct, i)) {
			double num;
			MirrorGetFieldNum(strct, strct->entries[i].name, pdata, &num);
			cJSON_AddNumberToObject(json, strct->entries[i].name, num);
		} else if(MirrorFieldIsArr(strct, i)) {
			
		}
	}
	
	return json;
}
