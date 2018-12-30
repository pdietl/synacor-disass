cmake_minimum_required(VERSION 3.2)
project(synacor_disassembler VERSION 0.1 LANGUAGES C)

add_executable(synacor_disas
    main.c
    arch.c
)
