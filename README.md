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
offset number -- mnumonic -- args
  493   jmp 1074
  495   jf  0,  500
  498   jmp 1074
  500   jt  r0, 1093
  503   jt  r1, 1093
  506   jt  r2, 1093
  509   jt  r3, 1093
  512   jt  r4, 1093
  515   jt  r5, 1093
  518   jt  r6, 1093
  521   jt  r7, 1093
  524   set r0, 1 
  527   jf  r0, 1118
  530   set r0, 0 
  533   jt  r0, 1118
  536   add r0, 1,  1 
  540   jt  r0, 564
  543   out 'n'
  545   out 'o'
  547   out ' '
  549   out 'a'
  551   out 'd'
  553   out 'd'
  555   out ' '
  557   out 'o'
  559   out 'p'
  561   out '\n'
  563   hlt   
  564   eq  r1, r0, 2 
  568   jt  r1, 590
  571   out 'n'
  573   out 'o'
  575   out ' '
  577   out 'e'
  579   out 'q'
  581   out ' '
  583   out 'o'
  585   out 'p'
  587   out '\n'
  589   hlt   
  590   push    r0    
  592   push    r1    
  594   pop r0
  596   pop r1
  598   eq  r2, r1, 2 
  602   jf  r2, 1158
  605   eq  r2, r0, 1 
  609   jf  r2, 1158
  612   gt  r2, r1, r0
  616   jf  r2, 1139
  619   gt  r2, r0, r1
  623   jt  r2, 1139
  626   gt  r2, 42, 42
  630   jt  r2, 1139
  633   and r0, 28912,  19626 
  637   eq  r1, r0, 16544 
  641   jf  r1, 1177
  644   or  r1, 28912,  19626 
  648   eq  r0, r1, 31994 
```
