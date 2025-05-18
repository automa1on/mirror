#include "mirror.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define mirror_log(...) do { fprintf(stderr, __VA_ARGS__);putc('\n', stderr);} while(0)

#define streq(s1,s2) (strcmp(s1,s2)==0)

static const char *getprimitive(const char *type) { // not implemented
	return type;
	
}

#define strswitch(str, ...) { const char *_strswitch_str = str; if(0){} __VA_ARGS__}
#define strcase(cas) else if(streq(_strswitch_str, cas))
#define strdefault else

#define unreachable() __builtin_unreachable()


bool MirrorFieldIsNum(struct _mirror_struct *strct, int entrynum) {
	struct _mirror_entry *e = strct->entries+entrynum;

	if(e->loi > 0 || e->isfarr) return false;
	if(e->type.prim != MirrorPrimitiveVoid) return true;

	return false;
}
bool MirrorFieldIsArr(struct _mirror_struct *strct, int entrynum) {
	struct _mirror_entry *e = strct->entries+entrynum;
	return( !(e->loi == 1 && e->type.purechar) &&
		(e->loi > 0 || e->isfarr) );
}

ssize_t _MirrorGetFieldArrLen(struct _mirror_struct *strct, int entrynum, void *pdata) {
	struct _mirror_entry *e = strct->entries+entrynum;
	if(e->farrlen)
		return e->farrlen;
	
	if(e->annot.annottype[0] != 't')
		return -MIRROR_EINVAL;

	if(e->annot.annottype[1] != 'l') // or s (sentinel) which is not implemented
		return -MIRROR_EINVAL;

	if(e->annot.annottype[2] == 'f') { // lenfield
		double num;
		printf("%s, %d\n",(char*)e->annot.annotdata, MirrorGetField(strct, e->annot.annotdata));
		int code = _MirrorGetFieldNum(&strct->entries[ MirrorGetField(strct, e->annot.annotdata) ], 
			pdata, &num);
		if(code != MIRROR_SUCCESS)
			return -code;

		return num;
	} else if(e->annot.annottype[2] == 'c') { // callback
		MirrorCBGetArrlen *f = e->annot.annotdata;
		return f(strct->name, e->name, pdata);
	}

	return -MIRROR_EINVAL;
}

static MirrorErrcode get_number(double *pnum, const char *data, enum mirror_primitive prim, bool issigned) {
	double num;

	#define p(lc,uc,rest)	case MirrorPrimitive##uc##rest: \
	if(issigned) num=*(signed lc##rest*)data; 			 \
	else					 num = *(unsigned lc##rest*)data; \
	break;

	switch(prim) {
		p(s,S,hort)
		p(c,C,har)
		p(i,I,nt)
		p(l,L,ong)
		
		case MirrorPrimitiveDFloat:
			num = *(double*)data;
			break;
		
		case MirrorPrimitiveFloat:
			num = *(float*)data;
			break;
		
		case MirrorPrimitiveDLong:
			if(issigned)
				num = *(signed long long*)data;
			else
				num = *(unsigned long long*)data;
		
			break;
		
		case MirrorPrimitiveVoid:
		case MirrorPrimitiveEnd:
			return MIRROR_EINVAL;
	}

	#undef p

	*pnum = num;

	return MIRROR_SUCCESS;
}

int _MirrorGetFieldNum(struct _mirror_entry *field, const void *pdata, double *pnum) {
	const char *data = pdata;
	data+=field->offset;
	double num;

	return get_number(pnum, data, field->type.prim, field->type.issigned);
}

int MirrorGetField(struct _mirror_struct *s, const char* name) {
	for(int i = 0; i < s->entries_len; i++) {
		if(streq(name, s->entries[i].name)) return i;
	}

	return -MIRROR_ENOENT;
}

int MirrorGetFieldNum(struct _mirror_struct *s, const char *fieldname, const void *pdata, double *pnum) {
	int n = MirrorGetField(s, fieldname);
	if(n<0) return +n;
	return _MirrorGetFieldNum(s->entries+n, pdata, pnum);
}


static int set_number(double value, char *data, enum mirror_primitive prim, bool issigned) {
	#define p(lc,uc,rest)	case MirrorPrimitive##uc##rest:									\
							if(issigned) *(signed lc##rest*)data = value;			\
							else					 *(unsigned lc##rest*)data = value;		\
							return MIRROR_SUCCESS;

	switch(prim) {
		case MirrorPrimitiveDFloat:
			*(double*)data = value;
			return MIRROR_SUCCESS;
		case MirrorPrimitiveFloat:
			*(float*)data = value;
			return MIRROR_SUCCESS;
		case MirrorPrimitiveDLong:
			if(issigned) *(signed long long*)data = value;
			else 				 *(unsigned long long*)data = value;
			return MIRROR_SUCCESS;
		
		p(s,S,hort)
		p(c,C,har)
		p(i,I,nt)
		p(l,L,ong)

		case MirrorPrimitiveEnd:
		case MirrorPrimitiveVoid:
			return MIRROR_EINVAL;
	}

	#undef p
}

int _MirrorSetFieldNum(double value, struct _mirror_entry *e, void *pdata) {
	char *data = pdata;
	data+=e->offset;

	return set_number(value, data, e->type.prim, e->type.issigned);
}


int _MirrorGetFieldArrNum(struct _mirror_struct *strct, int fieldnum, int idxnum, const void *pdata, double *pnum) {
	const char *data = pdata;
	struct _mirror_entry *e = strct->entries+fieldnum;
	if(e->isfarr) {
		data += e->offset;
		data += idxnum * e->type.size;
		return get_number(pnum, data, e->type.prim, e->type.issigned);
	}

	data += e->offset;
	data = *(void**)data;
	data += idxnum * e->type.size;

	return get_number(pnum, data, e->type.prim, e->type.issigned);

	unreachable();
}

int _MirrorSetFieldArrNum(struct _mirror_struct *strct, int fieldnum, int idxnum, void *pdata, double pnum) {
	char *data = pdata;
	struct _mirror_entry *e = strct->entries+fieldnum;
	if( !e->isfarr ) {
		data += e->offset;
		data = *(void**)data;
		data += idxnum * e->type.size;
		
		return set_number(pnum, data, e->type.prim, e->type.issigned);
	}
		

	if(idxnum >= e->farrlen)
		return MIRROR_EOOBND;

	data += e->offset;
	data += idxnum * e->type.size;

	return set_number(pnum, data, e->type.prim, e->type.issigned);
}

int MirrorSetFieldNumTrunc(double value, struct _mirror_entry *e, void *data) {
	return _MirrorSetFieldNum(value, e, data);
}

int MirrorSetFieldNum(struct _mirror_struct *s, const char *fieldname, void *data, double value) {
	int n = MirrorGetField(s, fieldname);
	if(n<0) return +n;
	struct _mirror_entry *e = s->entries+n;
	if(! (e->type.prim == MirrorPrimitiveDFloat || e->type.prim == MirrorPrimitiveFloat) ) {
		if(rint(value) != value) {
			return MIRROR_EINVAL;
		}
	}
	return MirrorSetFieldNumTrunc(value, e, data);
}

static struct _mirror_struct *structs = NULL;

static struct _mirror_entry *entry_array;
static int entry_array_len;

static enum mirror_primitive getprimfrombarestr(const char *type) {
	static struct {
		const char *name;
	} primtable[MirrorPrimitiveEnd] = {
		#define p(lc,uc,rest) [MirrorPrimitive##uc##rest] = #lc #rest,
		p(s,S,hort)
		p(c,C,har)
		p(i,I,nt)
		p(l,L,ong)
		p(f,F,loat)
		p(v,V,oid)
		#undef p

		[MirrorPrimitiveDLong] = "long long",
		[MirrorPrimitiveDFloat] = "double",
		
	};

	int prim = MirrorPrimitiveEnd;
	for(int pti = 0; pti < MirrorPrimitiveEnd; pti++) {
		if(strcmp(type, primtable[pti].name) == 0) {
			return pti;
		}
	}

	/*if(prim == MirrorPrimitiveEnd)*/ {
		mirror_log("unknown type: %s", type);
		abort();
	}

	
	unreachable();
}

static struct structll_entry {
	struct _mirror_struct *strct;
	struct structll_entry *next;
} structll;

static struct structll_entry *structll_last=NULL;

int _mirror_init_struct(const char *name, struct _mirror_unparsed_entry entries[], 
						struct _mirror_struct *strct)
{
	int i;

	printf("struct name: %s\nfields:\n", name);

	int entry_len = 1;
	struct _mirror_unparsed_entry *e = &entries[0];
	if(e->invalid) { // im not adding ts
		entry_len = 0;
		return 4;
	}

	for(i = 1; !e->invalid; e = &entries[i], i++) {
		printf("\t%s %s, offset: %d, isfarr: %d\n", e->type, e->name, e->offset, e->isfarr);
	}
	entry_len = i-1;

	struct _mirror_entry *pentries = calloc(entry_len, sizeof(struct _mirror_entry));

	int pei; // parsed entry increment
	for(i = 0, pei=0; i < entry_len; i++, pei++) {
		e = &entries[i];

		if(e->isannotation) {
			pei--; // it'll increment

			continue;
		}

		struct _mirror_entry *pe = &pentries[pei];

		{
			char *type = strdup(e->type);

			for(int stri = strlen(type)-1; stri >= 0; stri--) {
				if(type[stri] == '*') {
					pe->loi++;
					type[stri]=0;
				}

			}

			if(strncmp("signed", type, sizeof("signed")-1) == 0)
				type+=sizeof("signed")-0;

			if(strncmp("unsigned", type, sizeof("unsigned")-1) == 0) {
				if(type[sizeof("unsigned")-1] == ' ') { // no implicit "int"
					mirror_log("ther ismore");

					pe->type.prim = getprimfrombarestr(&type[sizeof("unsigned ")-1]);

				} else
					pe->type.prim = MirrorPrimitiveInt;
			} else {
				pe->type.issigned = true;
				pe->type.prim = getprimfrombarestr(type);
			}
		}

		pe->offset = e->offset;
		pe->name = strdup(e->name);
		pe->isfarr = e->isfarr;
		pe->farrlen = e->length;
		pe->lenfield = e->lenfield;
		pe->type.size = e->typesize;

		if(i > 0 && entries[i-1].isannotation) {
			memcpy(&pe->annot, &( (e-1)->annot), sizeof pe->annot );
		}

		printf("%d\n", pe->type.issigned);
	}

	//entry_array_len=entry_len;
	//entry_array=pentries;

	strct->entries = pentries;
	strct->entries_len = entry_len;

	return true;
}

int MirrorSetFieldArrNum(struct _mirror_struct *strct, const char *field,
	int idxnum, void *pdata, double pnum)
{
	int n = MirrorGetField(strct, field);
	if(n<0) return n;
	//if(strct->entries[n].isfarr || strct->entries[n].annot.annottype)

	return _MirrorSetFieldArrNum(strct, n, idxnum, pdata, pnum);
}

