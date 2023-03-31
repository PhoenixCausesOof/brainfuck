# Brainfuck

## Description

A Brainfuck implementation in C++ using asmjit as a back-end for code generation; minimally optimized for performance.

## Building

```shell
meson setup ./builddir
meson configure ./builddir -Dbuildtype=release -Doptimization=3
meson compile -C ./builddir
```

**Note: will only compile, not install (installation is not supported).**