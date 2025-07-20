#define b BEGIN_STRUCT
#define f STRUCT_FIELD
#define arr STRUCT_FIELD_ARRAY

#define arrf(type, name, fiel)  ann("atlf", #fiel) \
                                arr(type, name)

#define farr STRUCT_FIELD_FIXED_ARRAY
#define e END_STRUCT
#define ann STRUCT_FIELD_ANNOTATION

#define typedef(a,b) MIRROR_TYPEDEF(a,b)
