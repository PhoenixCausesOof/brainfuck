# Brainfuck

## Description

A Brainf*** implementation in C++ 20 using `asmjit` as a back-end.

Literally no optimizations (except combining contiguous operations into a single instruction).

It was simply an learning exercise.

## Dependencies

`Boost.Program Options`

## Building

```shell
meson setup ./build
meson configure ./build -Dbuildtype=release -Doptimization=3
meson compile -C ./build
```

**Note: will only compile, not install (installation is not supported).**

## Usage

After compilation, run the binary under the `build/` directory.

```
usage: brainfuck [options]
options:
  -h [ --help ]         display this help message
  -s [ --source ] arg   source file to compile
  --size arg (=30000)   size of stack
  -o [ --output ] arg   describes a path to the log file (if emitted, output to
                        stdout)
```