#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include "arch.h"

void out_handler(FILE *fp);

enum asm_instr_arg_type {
    EMPTY,
    REG,
    INT,
    REG_OR_INT
};

#define MAX_NUM_ARGS_PLUS_ONE 4

struct asm_instr {
    const char *mnemonic;
    enum asm_instr_arg_type arg_types[MAX_NUM_ARGS_PLUS_ONE];
    void (*special_handler)(FILE *fp);
};

struct asm_instr ops[] = {
    {"hlt",     {0}},
    {"set",     {REG, REG_OR_INT}},
    {"push",    {REG_OR_INT}},
    {"pop",     {REG_OR_INT}},
    {"eq",      {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"gt",      {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"jmp",     {REG_OR_INT}},
    {"jt",      {REG_OR_INT, REG_OR_INT}},
    {"jf",      {REG_OR_INT, REG_OR_INT}},
    {"add",     {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"mult",    {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"mod",     {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"and",     {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"or",      {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"not",     {REG_OR_INT, REG_OR_INT}},
    {"rmem",    {REG_OR_INT, REG_OR_INT}},
    {"wmem",    {REG_OR_INT, REG_OR_INT}},
    {"call",    {REG_OR_INT}},
    {"ret",     {0}},
    {"out",     {REG_OR_INT}, out_handler},
    {"in",      {REG_OR_INT}},
    {"noop",    {0}}
};

void disass(FILE *fp);

void not_implemented(enum opcode);

int line = 0;
bool no_offset = false;
bool hex_digits = false;

void usage(const char *prog_name)
{
    printf( "%s <options...> [binary file]\n"
            "Options:\n"
            "\t-h | --help:       Show this help message\n"
            "\t-n | --no-offset:  Do not prefix instructions with their offset relative to the beginning of the file\n"
            "\t-x | --hex-digits: Show the offset and all integer arguments in hex instead of decimal\n", prog_name);
}

struct option long_options[] = {
    {"no-offset",       optional_argument, 0, 'n'},
    {"hex-digits",      optional_argument, 0, 'x'},
    {"help",            optional_argument, 0, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    int option_index, ch;

    while (1) {
        option_index = 0;
        ch = getopt_long(argc, argv, "n::h::x::", long_options, &option_index);

        if (ch == -1)
            break;

        switch (ch) {
            case 'h': usage(argv[0]); return 0;
            case 'n': no_offset = true; break;
            case 'x': hex_digits = true; break;
            case '?': // fallthrough
            default: exit(1);
        }
    }

    if (argc == optind) {
        usage(argv[0]);
        return 1;
    }

    FILE *fp;

    if (!(fp = fopen(argv[optind], "r"))) {
        perror("");
    }

    disass(fp);

    return 0;
}

void disass_single(uint16_t op, FILE *fp);

void disass(FILE *fp)
{
    uint16_t op;
    while (readU16(fp, &op) != -1) {
        if (op >= NUM_OP_CODES) {
            fprintf(stderr, "ERROR: Op code out of range! Valid codes are from 0 through %u. Offending op code: %u\n", 
                NUM_OP_CODES - 1, op);
            exit(1);
        }
        disass_single(op, fp);
    }
}

long get_file_num_bytes(FILE *fp)
{
    long size;
    long pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp) + 1;
    fseek(fp, pos, SEEK_SET);
    return size;
}

const char *convert_escape(int ch)
{
    static char s[2] = {0};
    switch (ch) {
    case '\n': return "\\n";
    case '\t': return "\\t";
    case '\v': return "\\v";
    case '\r': return "\\r";
    default:
        s[0] = ch;
        return s;
    }
}

void printl(int increment, const char *format, ...)
{
    if (!no_offset) {
        if (hex_digits)
            printf("%#7x\t", line);
        else
            printf("%5d\t", line);
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    line += increment;
}

void check_valid_int(uint16_t n, const char *instruction, int arg_num)
{
    if (is_valid_int(n))
        return;

    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be a valid integer! Offending integer was: %u\nIntegers are from 0 through %u\n",
        arg_num, instruction, n, MAX_INT);
    exit(1);
}

void check_valid_reg(uint16_t val, const char *instruction, int arg_num)
{
    if (is_reg(val))
        return;
    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be a register!\nOffending value was: %d\n", arg_num, instruction, val);
    exit(1);
}

void check_valid_int_or_reg(uint16_t val, const char *instruction, int arg_num)
{
    if (is_reg(val) || is_valid_int(val))
        return;
    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be a register or integer!\nOffending value was: %d\n", 
        arg_num, instruction, val);
}

uint16_t readU16_check(FILE *fp)
{
    uint16_t n;
    if (readU16(fp, &n) == -1) {
        fprintf(stderr, "Error reading an additional u16 value!");
        exit(1);
    }
    return n;
}

void print_value(uint16_t val)
{
    if (is_reg(val)) {
        printf("r%d", val - MIN_REG);
    } else {
        if (hex_digits)
            printf("%#x", val);
        else
            printf("%d", val);
    }
}

void disass_single(uint16_t op_num, FILE *fp)
{
    struct asm_instr instr = ops[op_num];
    int num_args = 0;
    uint16_t arguments[MAX_NUM_ARGS_PLUS_ONE];
    int i;
    enum asm_instr_arg_type type;
    
    if (instr.special_handler) {
        instr.special_handler(fp);
        return;
    }

    for (i = 0; i < sizeof arguments / sizeof *arguments; ++i) {
        type = instr.arg_types[i];
        if (type == EMPTY)
            break;
        
        arguments[i] = readU16_check(fp);
        
        switch (type) {
        case REG:
            check_valid_reg(arguments[i], instr.mnemonic, i);
            break;

        case INT:
            check_valid_int(arguments[i], instr.mnemonic, i);
            break;

        case REG_OR_INT:
            check_valid_int_or_reg(arguments[i], instr.mnemonic, i); 
            break;

        default:
            fprintf(stderr, "HORRIBLE INTERNAL ERROR!");
            exit(1);
        }
    }

    num_args = i;

    printl(num_args + 1, instr.mnemonic);
    if (num_args) {
        putchar('\t');
        print_value(arguments[0]);
    }
    for (i = 1; i < num_args; ++i) {
        printf(",\t");
        print_value(arguments[i]);
    }
    putchar('\n');
}

void out_handler(FILE *fp)
{
    uint16_t ch = readU16_check(fp);
    printl(2, "out");
    printf("\t'%s'\n", convert_escape(ch));
}
