#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <ctype.h>
#include "arch.h"

enum asm_instr_arg_type {
    EMPTY,
    REG,
    INT,
    REG_OR_INT
};

#define MAX_NUM_ARGS 3
#define MAX_NUM_ARGS_PLUS_ONE (MAX_NUM_ARGS + 1)

struct asm_instr {
    const char *mnemonic;
    unsigned char op_code;
    enum asm_instr_arg_type arg_types[MAX_NUM_ARGS_PLUS_ONE];
};

struct asm_instr ops[] = {
    {"hlt",     0,  {0}},
    {"set",     1,  {REG, REG_OR_INT}},
    {"push",    2,  {REG_OR_INT}},
    {"pop",     3,  {REG_OR_INT}},
    {"eq",      4,  {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"gt",      5,  {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"jmp",     6,  {REG_OR_INT}},
    {"jt",      7,  {REG_OR_INT, REG_OR_INT}},
    {"jf",      8,  {REG_OR_INT, REG_OR_INT}},
    {"add",     9,  {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"mult",    10, {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"mod",     11, {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"and",     12, {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"or",      13, {REG_OR_INT, REG_OR_INT, REG_OR_INT}},
    {"not",     14, {REG_OR_INT, REG_OR_INT}},
    {"rmem",    15, {REG_OR_INT, REG_OR_INT}},
    {"wmem",    16, {REG_OR_INT, REG_OR_INT}},
    {"call",    17, {REG_OR_INT}},
    {"ret",     18, {0}},
    {"out",     19, {REG_OR_INT}},
    {"in",      20, {REG_OR_INT}},
    {"noop",    21, {0}}
};

void disass(FILE *fp);
void printl(int increment, const char *format, ...);

void not_implemented(enum opcode);

int line = 0;
bool no_offset = false;
bool hex_digits = false;
bool raw_instr = true;
bool compress_outs = false;

static bool last_instr_was_out = false;

void usage(const char *prog_name)
{
    printf( "%s <options...> [binary file]\n"
            "Options:\n"
            "\t-h | --help:               Show this help message\n"
            "\t-n | --no-offset:          Do not prefix instructions with their offset relative to the beginning of the file\n"
            "\t-x | --hex-digits:         Show the offset and all integer arguments in hex instead of decimal\n"
            "\t-c | --compress-out-instr: Compress runs of out instructions into a single line\n"
            "\t-r | --no-show-raw-instr:  Do not display hex alongside symbolic disassembly\n", prog_name);
}

struct option long_options[] = {
    {"help",              optional_argument, 0, 'h'},
    {"no-offset",         optional_argument, 0, 'n'},
    {"hex-digits",        optional_argument, 0, 'x'},
    {"compress-outs",     optional_argument, 0, 'c'},
    {"no-show-raw-instr", optional_argument, 0, 'r'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    int option_index, ch;

    while (1) {
        option_index = 0;
        ch = getopt_long(argc, argv, "n::h::x::c::r::", long_options, &option_index);

        if (ch == -1)
            break;

        switch (ch) {
            case 'h': usage(argv[0]); return 0;
            case 'n': no_offset = true; break;
            case 'x': hex_digits = true; break;
            case 'c': compress_outs = true; break;
            case 'r': raw_instr = false; break;
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

void print_data(uint16_t val)
{
    uint8_t one, two;
    one = val >> 8;
    two = val & 0xff;
    printl(1, "data\t");
    printf("%#04x %#04x\t", one, two);
    printf("|%c%c|\n", isprint(one) ? one : '.', isprint(two) ? two : '.');
}

void disass_single(uint16_t op, FILE *fp);

void disass(FILE *fp)
{
    uint16_t op;
    while (readU16(fp, &op) != -1) {
        if (op >= NUM_OP_CODES)
            print_data(op);
        else
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
            printf("    %04x:\t", line);
        else
            printf("%8d:\t", line);
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

    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be a "
                    "valid integer! Offending integer was: %u\n"
                    "Integers are from 0 through %u\n",
        arg_num, instruction, n, MAX_INT);
    exit(1);
}

void check_valid_reg(uint16_t val, const char *instruction, int arg_num)
{
    if (is_reg(val))
        return;
    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be "
                    "a register!\nOffending value was: "
                    "%d\n", arg_num, instruction, val);
    exit(1);
}

void check_valid_int_or_reg(uint16_t val, const char *instruction, int arg_num)
{
    if (is_reg(val) || is_valid_int(val))
        return;
    fprintf(stderr, "ERROR: expected argument %d of instruction %s to be "
                    "a register or integer!\nOffending value was: %d\n", 
        arg_num, instruction, val);
}

void print_int(uint16_t n)
{
    printf(" %04x", n);
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
            printf("0x%04x", val);
        else
            printf("%6d", val);
    }
}

void print_value_with_comma(uint16_t val)
{
    if (is_reg(val)) {
        printf("r%d,     ", val - MIN_REG);
    } else {
        if (hex_digits)
            printf("0x%04x, ", val);
        else
            printf("%6d, ", val);
    }
}

void disass_single(uint16_t op_num, FILE *fp)
{
    struct asm_instr instr = ops[op_num];
    int num_args = 0;
    uint16_t arguments[MAX_NUM_ARGS_PLUS_ONE];
    int i;
    enum asm_instr_arg_type type;
  
    /* special case for OUT instruction */
    if (op_num == OUT) {
        uint16_t ch = readU16_check(fp);
        if ((compress_outs && !last_instr_was_out) || !compress_outs) {
            if (raw_instr) {
                printl(num_args + 1, "%04x", instr.op_code);
                print_int(ch);
                printf("          \t\t%-4s\t", instr.mnemonic);
            } else {
                printl(num_args + 1, "%-4s\t", instr.mnemonic);
            }
        }
        if (compress_outs && !last_instr_was_out)
            printf("\"%s", convert_escape(ch));
        else if (compress_outs)
            printf("%s", convert_escape(ch));
        else
            printf("\"%s\"\n", convert_escape(ch));
        last_instr_was_out = true;
        return;
    } else if (compress_outs && last_instr_was_out) {
        printf("\"\n");
        last_instr_was_out = false;
    }

    if (raw_instr)
        printl(num_args + 1, "%04x", instr.op_code);

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
        if (raw_instr)
            print_int(arguments[i]);
    }

    num_args = i;

    for (i = 0; raw_instr && i < MAX_NUM_ARGS - num_args; ++i)
        printf("    ");

    if (raw_instr)
        printf("\t\t%-4s\t", instr.mnemonic);
    else
        printl(num_args + 1, "%-4s\t", instr.mnemonic);

    for (i = 0; i < num_args - 1; ++i) {
        print_value_with_comma(arguments[i]);
    }
    
    if (num_args)
        print_value(arguments[num_args - 1]);
    putchar('\n');
}
