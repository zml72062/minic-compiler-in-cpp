# A SysY to RISC-V compiler

Author: zml72062

## Overview

This project is part of the *Compiler Principles* course of PKU, for 2023 spring semester. A compiler that compiles SysY, a subset of C language, into RISC-V assembly, is built. See https://pku-minic.github.io/online-doc/ for more info.

## Build

Run `make` to build.

## Run

Run

```
./compiler -riscv <input-file> -o <output-file>
```

to compile the input SysY source program into RISC-V assembly.