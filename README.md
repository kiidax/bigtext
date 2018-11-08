# bigtext

bigtext is a collection of tools to process large text files as
quickly as possible. For
deeplearning, it is often required to prepare large text files, often
more than hundreds of tens of giga bytes. bigtext can be used for
most common tasks with such text files more quickly than with Python
or naiive code. But if your
task needs more complicated processing, then you'll need to
write your own code.

bigtext is a small binary, it probably runs on cheapest laptop with
4GB RAM. But some of tasks needs large memory. At least more than 16GB
RAM is recommended if you want to process text files of tens of giga
bytes.

## Getting binary

The latest Windows x64 binary is available here.
https://github.com/kiidax/bigtext/releases/download/v0.9.2/bigtext_0.9.2.0_x64_Release.exe

Visual C++ x64 Runtime for Visual Studio 2017 is required to run the command.
https://go.microsoft.com/fwlink/?LinkId=746572

## Avaiable tools

These tools are available

- count: Counting or guessing number of lines.
- sample: Sampling or shuffling lines
- vocab: Counting vocabulary.

## Show help message

```
$ bigtext
```

## Counting number of lines

```
$ bigtext count test.txt
```

### Quick

### Full

## Counting word frequency

### Quick

### Full

## Sampling

### Quick

### Full

## Shuffling

### One-pass

### Multi-pass

## Building from source code

### Getting source code

https://github.com/kiidax/bigtext

### Windows

#### Prerequisites

- Visual Studio 2017 (MSVC)
- Boost C++ Libraries 1.68.0
