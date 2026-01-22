# Hex3

## Description

Hex3 is a lightweight hexadecimal dump utility written in C. By default, it outputs three columns: offset, data and ASCII, all of which can be individually customized or disabled with command line options

Hex3's goal is to cleanly integrate into pipelines and scripts.

## Install
```
$ make
$ sudo make install
```
## Options

Usage: hex3 [options]... file

- -w, --width=INTEGER     the number of bytes per line (1-255)
- -a, --no-ascii          remove printable characters column
- -O, --no-offset         remove the offset column
- -d, --decimal-offset    output the offset column in decimal
- -b, --binary            output data in binary representation
- -o, --output=FILE       set destination file
- -g, --group=INTEGER     set byte grouping to 1, 2, 4 or 8
- -h, --help              display this help

## Example

```
$ hex3 -w 8 -g 2 file
```
