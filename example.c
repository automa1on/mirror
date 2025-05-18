#include <stddef.h>
#include <stdio.h>
#include "cJSON.h"
#include "structdefs.h"


MirrorCBGetArrlen testarrlencb;
long testarrlencb(const char *structname, const char *field, void *data) {
	return 1989;
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

int main() {
	init();

	struct ses a = {.ae=NULL};
	a.intarray[7] = 4;
	a.intarray[1] = 'D';

	a.shorter=8794;
	a.character=3;

	a.aed_len = 31;
	
	double num;
	MirrorGetFieldNum(&_mirror_struct_ses, "shorter", &a, &num);

	printf("%lf\n", num); // 8794

	MirrorSetFieldNum(&_mirror_struct_ses, "shorter", &a, 5);

	printf("ercode:%d\n", MirrorSetFieldArrNum(&_mirror_struct_ses, "intarray", 31, &a, 344));

	_MirrorGetFieldNum(_mirror_struct_ses.entries+1, &a, &num);

	printf("%d\n", a.intarray[31]);


	printf("len:%ld\n", _MirrorGetFieldArrLen(&_mirror_struct_ses, MirrorGetField(&_mirror_struct_ses, "tlc"), &a));

	//int fieldtlc = MirrorGetField(&_mirror_struct_ses, "intarray");
	//assert(fieldtlc >= 0);
	//for(int i = 0; i < _MirrorGetFieldArrLen(&_mirror_struct_ses, fieldtlc, &a); i++) {
	//	_MirrorGetFieldArrNum(&_mirror_struct_ses, fieldtlc, i, &a, &num);
    //    printf("%lf, ", num);
    //    if(i % 5 == 0)
    //        putchar('\n');
	//}
	
	char *ah = cJSON_Print(mjson_serialize_struct(&_mirror_struct_ses, &a));
	printf("%s",ah);
}

