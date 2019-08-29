# An Implementation of a Disassembler for the Synacor Challenge -- In C

Link to challenge: https://challenge.synacor.com/

## Build instructions

```
meson bld
ninja -C bld
./bld/syn-disass <binary to disassemble>
```

## Using the tool

You can find my implementation of the Synacor VM here https://github.com/pdietl/synacor along with `challenge.bin` which you can feed into this tool

## Example Output

```
./syn-disas <options...> [binary file]
Options:
	-h | --help:               Show this help message
	-n | --no-offset:          Do not prefix instructions with their offset relative to the beginning of the file
	-x | --hex-digits:         Show the offset and all integer arguments in hex instead of decimal
	-c | --compress-out-instr: Compress runs of out instructions into a single line
	-r | --no-show-raw-instr:  Do not display hex alongside symbolic disassembly


    002a:   0008 0001 0432          jf      0x0001, 0x0432
    002b:   0007 0001 01ef          jt      0x0001, 0x01ef
    002c:   0006 0432               jmp     0x0432
    002d:   0008 0000 01f4          jf      0x0000, 0x01f4
    002e:   0006 0432               jmp     0x0432
    002f:   0007 8000 0445          jt      r0,     0x0445
    0030:   0007 8001 0445          jt      r1,     0x0445
    0031:   0007 8002 0445          jt      r2,     0x0445
    0032:   0007 8003 0445          jt      r3,     0x0445
    0033:   0007 8004 0445          jt      r4,     0x0445
    0034:   0007 8005 0445          jt      r5,     0x0445
    0035:   0007 8006 0445          jt      r6,     0x0445
    0036:   0007 8007 0445          jt      r7,     0x0445
    0037:   0001 8000 0001          set     r0,     0x0001
    0038:   0008 8000 045e          jf      r0,     0x045e
    0039:   0001 8000 0000          set     r0,     0x0000
    003a:   0007 8000 045e          jt      r0,     0x045e
    003b:   0009 8000 0001 0001     add     r0,     0x0001, 0x0001
    003c:   0007 8000 0234          jt      r0,     0x0234
    003d:   0013 006e               out     "no add op\n" 
    003e:   0000                    hlt     
    003f:   0004 8001 8000 0002     eq      r1,     r0,     0x0002
    0040:   0007 8001 024e          jt      r1,     0x024e
    0041:   0013 006e               out     "no eq op\n"
    0042:   0000                    hlt
    0043:   0002 8000               push    r0
    0044:   0002 8001               push    r1
    0045:   0003 8000               pop     r0
    0046:   0003 8001               pop     r1
    0047:   0004 8002 8001 0002     eq      r2,     r1,     0x0002
    0048:   0008 8002 0486          jf      r2,     0x0486

```
