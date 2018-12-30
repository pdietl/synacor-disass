#ifndef SYNACOR_ARCH_H__
#define SYNACOR_ARCH_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_INT 32767
#define MIN_REG 32768
#define MAX_REG 32775

enum opcode {
    HALT = 0,
    SET,
    PUSH,
    POP,
    EQ,
    GT,
    JMP,
    JT,
    JF,
    ADD,
    MULT,
    MOD,
    AND,
    OR,
    NOT,
    RMEM,
    WMEM,
    CALL,
    RET,
    OUT,
    IN,
    NOOP,
    NUM_OP_CODES
};

const char *op_to_string(enum opcode op);
int addr_to_reg_num(uint16_t addr);
bool is_valid_int(uint16_t n);
bool is_reg(uint16_t addr);
int readU16(FILE *fp, uint16_t *ret);
#endif
