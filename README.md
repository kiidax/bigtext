# bigtext

bigtext is a collection of tools to process large text files as
quickly as possible. For
deeplearning, it is often required to prepare large text files, often
more than hundreds of tens of giga bytes. bigtext can be used for
most common tasks with such text files more quickly than with Python
or naiive code. But if your
task needs more complicated processing, then you'll need to
write your own code.

bigtext is a small binary, it probably runs on a cheapest laptop with
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

Without arguments, bigtext shows list of available commands.

```
$ bigtext
usage: bigtext COMMAND [ARGS]

bigtext is a collection of tools to process large text files.

List of commands:

   count      Count the number of lines in the files.
   sample     Sample lines from the files.
   vocab      Count the words in the files.
   version    Show the version info.
```

With a command name without any extra arguments, bigtext shows
help message for the command.

```
$ bigtext count
Usage: bigtext count [OPTION]... INPUTFILE...
Count number of lines in the file.

 -c         full count mode
 -h         show this help message
 INPUTFILE  input file
```

## Count number of lines

In many cases, you can tell the total number of samples
in the training data by counting the total number of lines in the
training data file as they have one sample data per one line.
Knowing the number of samples are important to estimate the quality
of the model after training.

The count command has two modes, the quick mode and the full mode.
The quick mode estimates the total number of lines in a file by
reading only the first 100MB. The full mode actually counts lines
and outputs accurate numbers.

### Quick mode

The quick mode is enabled by default. EstLineCount is the estimated
total number of lines.

```
$ bigtext count shakespeare.txt
shakespeare.txt MinLineSize     1
shakespeare.txt MaxLineSize     86
shakespeare.txt AvgLineSize     43.79
shakespeare.txt StdLineSize     17.58
shakespeare.txt UsedLineCount   124796
shakespeare.txt FileSize        5465397
shakespeare.txt EstLineCount    124796
```

### Full mode

The full mode is enabled with the -c option.

```
$ bigtext count -c shakespeare.txt
 0.120902s wall, 0.046875s user + 0.000000s system = 0.046875s CPU (38.8%)

shakespeare.txt LineCount       124796
```

## Count word frequency

The vocab command counts frequencies of words in text files and outputs a
vocabulary file. A vocabulary file contains lines of tab separated word and
frequency pairs and is sorted by descending frequencies.

The input files should be tokenized in advance and stemmed if needed. The
vocab command breaks text into words by white space characters, like
spaces, tabs and new lines.

```
$ bigtext vocab shakespeare.txt -o vocab.txt
vocab.txt       TargetColumn    0
 2.206307s wall, 1.265625s user + 0.843750s system = 2.109375s CPU (95.6%)
```

vocab.txt should contains something like this.

```
the     23407
I       19540
and     18358
to      15682
of      15649
...
```

If the file contains tab separated text, then the vocab command can count
words in the specified columns.

```
$ bigtext vocab parallel_corpus.txt -c 1 vocab_src.txt -c 2 vocab_tgt.txt
vocab_src.txt   TargetColumn    1
vocab_tgt.txt   TargetColumn    2
 0.071133s wall, 0.000000s user + 0.015625s system = 0.015625s CPU (22.0%)
```

## Sample lines randomly

The sampling command can be used to randomly sample lines from text files.

### Quick mode

If you want to sample small number of lines from large text files, then
this mode is the quickest way. It is often used to see the sampled lines
and check if the training data is correct manually.

This mode repeats seeking the text file in the random position and sampling
the next line to the position. If you want to sample 1 million lines
from a file with 2 million lines, then this mode is not efficient.

```
$ bigtext sample -q shakespeare.txt -n 100 result.txt
vocab_src.txt   TargetColumn    1
vocab_tgt.txt   TargetColumn    2
 0.071133s wall, 0.000000s user + 0.015625s system = 0.015625s CPU (22.0%)
```

### Full mode

This mode can be used to divide data into training/dev/test set. The
following splits lines into 10% dev data, 10% test data, 80% training data.

```
$ bigtext sample shakespeare.txt -r 10% dev.txt -r 10% test.txt -o train.txt
dev.txt TargetRate      0.1
test.txt        TargetRate      0.1
train.txt       TargetRate      1
 2.700869s wall, 0.125000s user + 0.093750s system = 0.218750s CPU (8.1%)
```

This mode also can be used to reduce the training data.

```
$ bigtext sample shakespeare.txt -r 50% result.txt
result.txt      TargetRate      0.5
 0.214666s wall, 0.031250s user + 0.062500s system = 0.093750s CPU (43.7%)
```

## Shuffle lines randomly

It is needed to shuffle traing data to train model efficiently. Shuffling
requires large memory. One-pass mode reads all the lines in memory, so
the file size must be smaller than available memory size. Multi-pass mode
scans files n-times and reads only one n-th of lines in memory at once.

The sample command with the -s option shuffle files. The -c option
specifies the number of passes. One-pass mode is enabled if the number
of pass is 1 and the multi-pass mode is enabled if the number of pass
is more than 1. If the -c option is not specified, then it automatically
geuss which mode to use.

### One-pass mode

The following shuffles
and splits lines into test data with 1000 lines and training data with
remaining lines.

```
$ bigtext sample -s -c 1 shakespeare.txt -n 1000 test.txt -o train.txt
test.txt        TargetLineCount 1000
train.txt       TargetRate      1
        PhysicalMemorySize      4116836352
shakespeare.txt CharCount       5465397
shakespeare.txt LineCount       124796
        LineCount       124796
test.txt        LineCount       1000
train.txt       LineCount       123796
 0.729716s wall, 0.140625s user + 0.062500s system = 0.203125s CPU (27.8%)
```

### Multi-pass

The following also shuffles
and splits lines into test data with 1000 lines and training data with
remaining lines. If the input file size is 100GB, then it only uses around
20GB memory at once, which probably works with a machine with 32GB memory.

```
$ bigtext sample -s -c 5 shakespeare.txt -n 1000 test.txt -o train.txt
```

## Building from source code

### Getting source code

The source code is available in GitHub.
https://github.com/kiidax/bigtext

### Windows

Open the solution file bigtext.sln and build it. 

#### Prerequisites

- Visual Studio 2017 (MSVC)
- Boost C++ Libraries 1.68.0
