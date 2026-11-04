#ifndef WF_VM_SPEC
#define WF_VM_SPEC

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define unused(x) ((void)(x))

#define VM_MAGIC "WFVM"

#ifndef VM_VERSION
    #define VM_VERSION 999999
#endif

#define WORD_SIZE 8

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

typedef enum {
    VAL_CHAR,
    VAL_I32,
    VAL_I64,
    VAL_F32,
    VAL_F64,
    VAL_PTR,
    VAL_DATA_PTR
} ValueType;

typedef struct {
    ValueType type;
    union {
        char     c;
        int32_t  i32;
        int64_t  i64;
        float    f32;
        double   f64;
        uint64_t ptr;
        uint64_t data_ptr;
    }as;
} Value;

typedef struct {
    u8 opcode;
    u8 is_imm;
    u16 dest_reg;
    u16 src_reg;
    u16 aux_reg;
    i64 imm;
} Instruction;

typedef struct {
    char magic[4];
    u32 version;
    u64 entry_addr;
    u64 code_count;
    u64 data_size;
    u64 symbol_count;
} BinaryHeader;

typedef struct {
    u64 instruction_offset;
    u16 register_index;
    u16 name_length;
    char name[32];
} SymbolTableEntry;

typedef enum {
    NO_OP = 0,
    MOV,
    ADD,
    SUB,
    CMP,
    JMP,
    JE,
    LDO,
    STO,
    CAST,
    ALLOC,
    SAVE,
    RESET,
    CALL,
    RET,
    EXIT,
    DEBUG_PRINT,
} Opcodes;


#endif /* WF_VM_SPEC */
