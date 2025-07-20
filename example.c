#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "cJSON.h"
#include "structdefs.h"
#include <memory.h>


MirrorCBGetArrlen testarrlencb;
long testarrlencb(const char *structname, const char *field, void *data) {
	return 19;
}

#define MIRROR_INIT_NONSCOPE
#include "structdefs.h"
#undef MIRROR_INIT_NONSCOPE

void init() {
	#define MIRROR_INIT

	#include "structdefs.h"

	#undef MIRROR_INIT

	
}

#include "mjson.c"

// return value may or may not be buf
char *strfield(struct _mirror_struct *strct, const void *pdata, int fidx, char *buf, int buflen) {
	struct _mirror_entry *e = strct->entries+fidx;
	if(MirrorFieldIsStruct(strct, fidx))
		return NULL;

	//char format[5] = {'%', 0,0,0,0};


	if(MirrorFieldIsNum(strct, fidx)) {
		double num;
		assert(MirrorGetFieldNum(strct, e->name, pdata, &num) == MIRROR_SUCCESS);

		const char format[] = "%.lf";
		snprintf(buf, buflen, format, num);
		return buf;
	}


	return NULL;
}

mjson_alloc_callback alloc_callback;
bool alloc_callback(struct _mirror_struct *strct, int field_idx, int nmemb, int size, void *data) {
	double ptr = (double)(uintmax_t)calloc(nmemb, size);
	MirrorSetFieldNum(strct, strct->entries[field_idx].name, data, ptr);
}

int main() {
	init();

	struct ses a = {0};
	a.intarray[7] = 4;
	a.intarray[1] = 'D';

	a.shorter=8794;
	a.character=21;

	a.aed_len = 31;
	a.dynarr_len = 0;

	a.nested.ises_int = 3;
	a.justAstring = strdup("hello i am string");	

	a.aed = calloc(a.aed_len, sizeof(*a.aed));
	
	a.tlc = calloc(testarrlencb(0,0,0), sizeof(*a.tlc));

	MirrorSetFieldArrNum(&_mirror_struct_ses, "tlc", 3, &a, 31);

	double num;
	MirrorGetFieldNum(&_mirror_struct_ses, "shorter", &a, &num);

	printf("%lf\n", num); // 8794

	MirrorSetFieldNum(&_mirror_struct_ses, "shorter", &a, 5);

	printf("ercode:%d\n", MirrorSetFieldArrNum(&_mirror_struct_ses, "intarray", 31, &a, 344));

	MirrorGetFieldNum(&_mirror_struct_ses, "shorter", &a, &num);

	printf("%d\n", a.intarray[31]);


	printf("len:%ld\n", _MirrorGetFieldArrLen(&_mirror_struct_ses, MirrorGetField(&_mirror_struct_ses, "tlc"), &a));

	printf("str: %p\n", a.justAstring);
	
	cJSON *json = mjson_serialize_struct(&_mirror_struct_ses, &a);

	char *ah = cJSON_Print(json);
	printf("%s\n",ah);

	memset(&(a), 0, sizeof a);

	mjson_deserialize_struct(&_mirror_struct_ses, &a, json, alloc_callback);

	char buf[44];
	strfield(&_mirror_struct_ses, &a, 
		MirrorGetField(&_mirror_struct_ses, "shorter"), buf, sizeof buf);
	printf("%s\n", buf);
}

