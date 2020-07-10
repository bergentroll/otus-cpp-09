[![Build Status](
  https://travis-ci.com/bergentroll/otus-cpp-09.svg?branch=master
)](https://travis-ci.com/bergentroll/otus-cpp-09)

NAME
====

bayan - search for file duplicates

SYNOPSIS
========

**bayan** \[*OPTION*\]\... \[*PATH*\]\...

DESCRIPTION
===========

Search PATHs recursively (the current directory if nothing passed).
Print to stdout duplicated files as groups separated with empty line.
Utility cares of filesystem and tries to minimize amount of processed
blocks.

Mandatory arguments to long options are mandatory for short options too.

**-b**, **\--block-size**=*SIZE*

:   size of block to process in bytes

**-e**, **\--exclude**=*DIR\...*

:   list of directories to exclude, paths must be absolute and lies
    inside target directories, may be specified multiple times

**-f**, **\--hash-func**=*FUNCTION*

:   specify hash function, may affect performance

**-h**, **\--help**

:   list short documentation including default values

**-l**, **\--level**=*DEPTH*

:   restrict depth of search, negative value to no limit

**-p**, **\--pattern**=*REGEX\...*

:   regex to match file names, only matched will be processed, may be
    specified multiple times

**-s**, **\--min-file**=*SIZE*

:   minimum file size to process in bytes, may significantly affect
    performance

EXAMPLES
========

Searches /usr/ for duplicated shared library files bigger than 1 MB
inside.

**bayan -s 1000000 -p \".\*\\.so\" /usr/**

AUTHOR
======

Written by Anton \"bergentroll\" Karmanov as an exercise on OTUS course.
