#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include "arch.h"

void (*op_functions[NUM_OP_CODES])(FILE *) = {0};
void disass(FILE *fp);

void not_implemented(enum opcode);
void halt(FILE *fp);
void jmp(FILE *fp);
void out(FILE *fp);
void noop(FILE *fp);
void jt(FILE *fp);
void jf(FILE *fp);
void set(FILE *fp);

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

    op_functions[HALT] = halt;
    op_functions[JMP] = jmp;
    op_functions[OUT] = out;
    op_functions[NOOP] = noop;
    op_functions[JT] = jt;
    op_functions[JF] = jf;
    op_functions[SET] = set;

    disass(fp);

    return 0;
}

void disass(FILE *fp)
{
    uint16_t op;
    while (readU16(fp, &op) != -1) {
        if (op >= NUM_OP_CODES) {
            fprintf(stderr, "ERROR: Op code out of range! Valid codes are from 0 through %u. Offending op code: %u\n", 
                NUM_OP_CODES - 1, op);
            exit(1);
        }
        if (op_functions[op] == 0) {
            not_implemented(op);
        }
        op_functions[op](fp);
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

void check_int_or_reg(uint16_t val, const char *instruction, int arg_num)
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


/* instruction functions */

void not_implemented(enum opcode code)
{
    printf("*** DEBUG: Opcode number %d (%s) not implemented.\n", code, op_to_string(code));
    exit(0);
}

void halt(FILE *fp)
{
    printl(1, "hlt\n");
}

void out(FILE *fp)
{
    uint16_t ch = readU16_check(fp);
    printl(2, "out\t'%s'\n", convert_escape(ch));
}

void noop(FILE *fp)
{
    printl(1, "noop\n");
}

void jmp(FILE *fp)
{
    uint16_t addr = readU16_check(fp);
    check_int_or_reg(addr, __func__, 1);
    printl(2, "jmp\t");
    print_value(addr);
    putchar('\n');
}

void jump_tf_common(FILE *fp, const char *op_name)
{
    uint16_t addr, cond;
    cond = readU16_check(fp);
    check_int_or_reg(cond, op_name, 1);
    addr = readU16_check(fp);
    check_int_or_reg(addr, op_name, 2);
    printl(3, op_name);
    putchar('\t');
    print_value(cond);
    putchar('\t');
    print_value(addr);
    putchar('\n');
}

void jt(FILE *fp)
{
    jump_tf_common(fp, "jt");
}

void jf(FILE *fp)
{
    jump_tf_common(fp, "jf");
}

void set(FILE *fp)
{
    uint16_t reg, val;
    reg = readU16_check(fp);
    check_valid_reg(reg, __func__, 1);
    val = readU16_check(fp);
    check_int_or_reg(val, __func__, 2);
    printl(3, "set\t");
    print_value(reg);
    putchar('\t');
    print_value(val);
    putchar('\n');
}


