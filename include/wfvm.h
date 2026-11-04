#ifndef WFVM_H
#define WFVM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define VM_MAGIC "WFVM"

#ifndef VM_VERSION
    #define VM_VERSION 999999
#endif

#define WORD_SIZE 8
#define MAX_REGISTERS 256
#define MAX_CALL_STACK 256

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
    } as;
} Value;

typedef struct {
    u8  opcode;      
    u8  is_imm;      
    u8  type_mod;    
    u8  _padding;    
    u16 dest_reg;    
    u16 src_reg;     
    u16 aux_reg;     
    i64 imm;         
} Instruction;

typedef struct {
    char     magic[4];
    u32      version;
    u64      entry_addr;
    u64      code_count;
    u64      data_size;
    u64      symbol_count;
} BinaryHeader;

typedef struct {
    u64 instruction_offset;
    u16 register_index;
    u16 name_length;
    char     name[32];
} SymbolTableEntry;

typedef enum {
    NO_OP = 0,
    MOV,
    ADD,
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
    DEBUG_PRINT
} Opcodes;

static const char* const OPCODE_NAMES[] = {
    [NO_OP]       = "no_op",
    [MOV]         = "mov",
    [ADD]         = "add",
    [CMP]         = "cmp",
    [JMP]         = "jmp",
    [JE]          = "je",
    [LDO]         = "ldo",
    [STO]         = "sto",
    [CAST]        = "cast",
    [ALLOC]       = "alloc",
    [SAVE]        = "arena.save",
    [RESET]       = "arena.reset",
    [CALL]        = "call",
    [RET]         = "ret",
    [EXIT]        = "exit",
    [DEBUG_PRINT] = "debug_print"
};

static inline const char* opcode_to_string(u8 opcode) {
    //@TODO: make this not crash
    return OPCODE_NAMES[opcode];
}

typedef struct {
    Value registers[MAX_REGISTERS];
    u64   return_pc;
} CallFrame;

typedef struct WFVMState WFVMState;

WFVMState* wfvm_create_from_memory(const u8* binary_data, size_t data_size);
int wfvm_run(WFVMState* vm);
void wfvm_free(WFVMState* vm);

#endif // WFVM_H

#ifdef WFVM_IMPLEMENTATION

struct WFVMState {
    u8* code_segment;
    u8* data_segment;
    u8* arena_block;
    size_t   arena_capacity;
    size_t   arena_cursor;
    u64      pc;
    int      is_running;

    CallFrame call_stack[MAX_CALL_STACK];
    int       frame_depth;
};

/* @OPCODES */
static inline int wfvm_op_cast(WFVMState* vm, CallFrame* frame, const Instruction* inst) {
    (void)vm;
    Value src_val = frame->registers[inst->src_reg];
    Value result = {0};
    result.type = inst->type_mod;

    switch (inst->type_mod) {
        case VAL_I32:
            result.as.i32 = (src_val.type == VAL_F32) ? (i32)src_val.as.f32 : (i32)src_val.as.i64;
            break;
        case VAL_F32:
            result.as.f32 = (src_val.type == VAL_I32) ? (f32)src_val.as.i32 : (f32)src_val.as.i64;
            break;
        default:
            fprintf(stderr, "[VM Error] Unsupported cast target type modifier: %u\n", inst->type_mod);
            return -1;
    }
    frame->registers[inst->dest_reg] = result;
    return 0;
}

static inline int wfvm_op_exit(WFVMState* vm, CallFrame* frame, const Instruction* inst) {
    (void)frame;
    (void)inst;
    vm->is_running = 0;
    return 0;
}

/* @VM STATE */

WFVMState* wfvm_create_from_memory(const u8* binary_data, size_t data_size) {
    if (data_size < sizeof(BinaryHeader)) return NULL;

    BinaryHeader header;
    memcpy(&header, binary_data, sizeof(BinaryHeader));

    if (memcmp(header.magic, VM_MAGIC, 4) != 0) {
        return NULL;
    }

    WFVMState* vm = (WFVMState*)malloc(sizeof(WFVMState));
    if (!vm) return NULL;
    memset(vm, 0, sizeof(WFVMState));

    const u8* cursor = binary_data + sizeof(BinaryHeader);

    vm->data_segment = (u8*)malloc(header.data_size);
    memcpy(vm->data_segment, cursor, header.data_size);
    cursor += header.data_size;

    size_t code_bytes = sizeof(Instruction) * header.code_count;
    vm->code_segment = (u8*)malloc(code_bytes);
    memcpy(vm->code_segment, cursor, code_bytes);

    vm->arena_capacity = 32 * 1024 * 1024;
    vm->arena_block = (u8*)malloc(vm->arena_capacity);
    vm->arena_cursor = 0;

    vm->pc = header.entry_addr;
    vm->is_running = 1;
    vm->frame_depth = 0;

    return vm;
}

int wfvm_run(WFVMState* vm) {
    CallFrame* frame = &vm->call_stack[vm->frame_depth];

    while (vm->is_running) {
        Instruction* inst = (Instruction*)(vm->code_segment + (vm->pc * sizeof(Instruction)));
        u64 current_ip = vm->pc;
        vm->pc++;

        int status = 0;
        switch (inst->opcode) {
            case CAST:
                status = wfvm_op_cast(vm, frame, inst);
                break;
            case EXIT:
                status = wfvm_op_exit(vm, frame, inst);
                break;
            default:
                printf("[VM Test] Fallthrough Opcode -> Name: %s (id: %u) at IP: %llu\n", 
                        opcode_to_string(inst->opcode), 
                        inst->opcode, 
                        (unsigned long long)current_ip);
                break;
        }

        if (status != 0) {
            return status;
        }
    }
    return 0;
}

void wfvm_free(WFVMState* vm) {
    if (!vm) return;
    free(vm->code_segment);
    free(vm->data_segment);
    free(vm->arena_block);
    free(vm);
}

#endif // WFVM_IMPLEMENTATION
