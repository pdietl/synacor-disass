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

    0256:   0004 8002 8001 0002     eq      r2,     r1,     0x0002
    025a:   0008 8002 0486          jf      r2,     0x0486
    025d:   0004 8002 8000 0001     eq      r2,     r0,     0x0001
    0261:   0008 8002 0486          jf      r2,     0x0486
    0264:   0005 8002 8001 8000     gt      r2,     r1,     r0
    0268:   0008 8002 0473          jf      r2,     0x0473
    026b:   0005 8002 8000 8001     gt      r2,     r0,     r1
    026f:   0007 8002 0473          jt      r2,     0x0473
    0272:   0005 8002 002a 002a     gt      r2,     0x002a, 0x002a
    0276:   0007 8002 0473          jt      r2,     0x0473
    0279:   000c 8000 70f0 4caa     and     r0,     0x70f0, 0x4caa
    027d:   0004 8001 8000 40a0     eq      r1,     r0,     0x40a0
    0281:   0008 8001 0499          jf      r1,     0x0499
    0284:   000d 8001 70f0 4caa     or      r1,     0x70f0, 0x4caa
    0288:   0004 8000 8001 7cfa     eq      r0,     r1,     0x7cfa
    028c:   0007 8000 02ac          jt      r0,     0x02ac
    028f:   0013 006e               out     "no bitwise or\n"
    02ab:   0000                    hlt
    02ac:   000e 8000 0000          not     r0,     0x0000
    02af:   0004 8001 8000 7fff     eq      r1,     r0,     0x7fff
    02b3:   0008 8001 04b8          jf      r1,     0x04b8
    02b6:   000e 8000 5555          not     r0,     0x5555
    02b9:   0004 8001 8000 2aaa     eq      r1,     r0,     0x2aaa
    02bd:   0008 8001 04b8          jf      r1,     0x04b8
    02c0:   0011 0505               call    0x0505
    02c2:   0006 0509               jmp     0x0509
    02c4:   0003 8000               pop     r0
    02c6:   0004 8001 8000 02c4     eq      r1,     r0,     0x02c4
    02ca:   0007 8001 0509          jt      r1,     0x0509
    02cd:   0004 8001 8000 02c2     eq      r1,     r0,     0x02c2
    02d1:   0008 8001 0509          jf      r1,     0x0509
    02d4:   0001 8000 0507          set     r0,     0x0507
    02d7:   0011 8000               call    r0
    02d9:   0006 0509               jmp     0x0509
    02db:   0003 8000               pop     r0
    02dd:   0004 8001 8000 02db     eq      r1,     r0,     0x02db
    02e1:   0007 8001 0509          jt      r1,     0x0509
    02e4:   0004 8001 8000 02d9     eq      r1,     r0,     0x02d9
    02e8:   0008 8001 0509          jf      r1,     0x0509
    02eb:   0009 8000 7fff 7fff     add     r0,     0x7fff, 0x7fff
    02ef:   0004 8001 8000 7ffe     eq      r1,     r0,     0x7ffe


```
