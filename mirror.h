#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#ifndef BEGIN_STRUCT

typedef long MirrorCBGetArrlen(const char *structname, const char *field, void *data);

typedef enum MirrorErrcode {
	MIRROR_SUCCESS = 1, // not 0 because functions that normally return positive values, return a negated errcode
	MIRROR_EINVAL,
	MIRROR_ENOENT,
	MIRROR_EOOBND, // out of bounds, array
} MirrorErrcode;

struct _mirror_annot {
	const char *annottype;
	void *annotdata; // MirrorGetArrlen, length field, etc...
};

struct _mirror_unparsed_entry {
	bool invalid, isannotation;

	struct _mirror_annot annot;

	const char *type;
	const char *name;

	int offset;

	bool isfarr;
	int length;

	const char *lenfield;

	int typesize;
};

struct _mirror_entry {
	char *name;
	int offset;

	int loi;

	bool isfarr; // is fixed-length array
	int farrlen;

	const char *lenfield; // name of the field that has the dynamic arrays length

	struct _mirror_annot annot;

	struct {
		int32_t size;

		bool issigned, 
			 purechar;
		
		enum mirror_primitive {
			MirrorPrimitiveVoid,
			MirrorPrimitiveShort,
			MirrorPrimitiveInt,
			MirrorPrimitiveChar,
			MirrorPrimitiveLong,
			MirrorPrimitiveDLong, // long long
			MirrorPrimitiveFloat,
			MirrorPrimitiveDFloat, // double
		
			MirrorPrimitiveEnd,
		} prim;
	} type;
};

struct _mirror_struct {
	char *name;

	int entries_len;
	struct _mirror_entry *entries;
};

int MirrorGetField(struct _mirror_struct *s, const char* name);
bool MirrorFieldIsNum(struct _mirror_struct *strct, int entrynum);
bool MirrorFieldIsArr(struct _mirror_struct *strct, int entrynum);

ssize_t _MirrorGetFieldArrLen(struct _mirror_struct *strct, int entrynum, void *pdata);

int MirrorSetFieldArrNum(struct _mirror_struct *strct, const char *field,
	int idxnum, void *pdata, double pnum);

int MirrorSetFieldNum(struct _mirror_struct *s, const char *fieldname, void *data, double value);
int _MirrorSetFieldNum(double value, struct _mirror_entry *e, void *pdata);

int MirrorGetFieldNum(struct _mirror_struct *s, const char *fieldname, const void *pdata, double *pnum);
int _MirrorGetFieldNum(struct _mirror_entry *field, const void *pdata, double *pnum);
int _MirrorGetFieldArrNum(struct _mirror_struct *strct, int fieldnum, int idxnum, const void *pdata, double *pnum);

#endif
#ifdef BEGIN_STRUCT


	#undef BEGIN_STRUCT
	#undef _BEGIN_STRUCT_FR
	#undef _BEGIN_STRUCT_FRFR
	
	#undef STRUCT_FIELD
	#undef STRUCT_FIELD_ARRAY
	#undef STRUCT_FIELD_FIXED_ARRAY
	
	#undef END_STRUCT

	#undef STRUCT_FIELD_ANNOTATION

#endif


#if defined(MIRROR_INIT)



int _mirror_init_struct(const char *name, struct _mirror_unparsed_entry entries[], struct _mirror_struct *strct);

	#define _GETFOFF(nam) ( (char*)&(_dummy.nam) - (char*)&_dummy )

	#define _BEGIN_STRUCT_FRFR(n) { struct _mirror_struct *strct = &_mirror_struct_##n; \
									char *_name = #n;\
									struct n _dummy;\
									static struct _mirror_unparsed_entry _cnoprsr[] = {
									
	#define _BEGIN_STRUCT_FR(n) _BEGIN_STRUCT_FRFR(n)

	#define BEGIN_STRUCT(n) _BEGIN_STRUCT_FR(n)
	#define END_STRUCT() {.invalid=1} }; _mirror_init_struct(_name, _cnoprsr, strct); }
	#define STRUCT_FIELD(typ, nam) {.type=#typ, .name=#nam, .offset=_GETFOFF(nam), .typesize=sizeof(typ)},
	#define STRUCT_FIELD_ARRAY(typ,nam) {.type=#typ, .name=#nam, .offset=_GETFOFF(nam), .lenfield="#lenfiel", .typesize=sizeof(typ)},
	#define STRUCT_FIELD_FIXED_ARRAY(typ, nam, len) {.type=#typ, .name=#nam, .offset= _GETFOFF(nam), .length=len, .isfarr=true, .typesize=sizeof(typ) },


	#define STRUCT_FIELD_ANNOTATION(ann,data) {.isannotation=true, .annot={.annottype=ann, .annotdata=data}},

#elif defined(MIRROR_INIT_NONSCOPE)
	#define BEGIN_STRUCT(n)	struct _mirror_struct _mirror_struct_##n;\

	#define STRUCT_FIELD(type, name) 
	#define STRUCT_FIELD_ARRAY(type, name) 
	#define STRUCT_FIELD_FIXED_ARRAY(type, name, len) 

	#define END_STRUCT() 

	#define STRUCT_FIELD_ANNOTATION(annot,d)

#else

	#define BEGIN_STRUCT(n)	extern struct _mirror_struct _mirror_struct_##n;\
							struct n {

	#define STRUCT_FIELD(type, name) type name; // put stars on type
	#define STRUCT_FIELD_ARRAY(type, name) type *name;
	#define STRUCT_FIELD_FIXED_ARRAY(type, name, len) type name[len];
	
	#define END_STRUCT() };

	#define STRUCT_FIELD_ANNOTATION(annot,d)
#endif

