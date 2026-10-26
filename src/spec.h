#ifndef WF_VM_SPEC
#define WF_VM_SPEC

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define unused(x) ((void)(x))

typedef int32_t i32;
typedef int64_t i64;
typedef float   f32;
typedef double  f64;

typedef enum {
    VAL_I32,
    VAL_I64,
    VAL_F32,
    VAL_F64,
    VAL_BOOL,
    VAL_OBJECT,
} ValueType;

typedef struct {
    ValueType type;
    union {
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;
        bool boolean;
        void *ptr;
    }as;
} Value;

typedef struct {
    i64 opcode;
    i64 dest;
    i64 src1;
    i64 src2;
} Instruction;


#endif /* WF_VM_SPEC */
