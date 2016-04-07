### Chess software

##### About

This utility calculates the intra connectivity matrix for each colour (the
"defensive" contact matrices or DCM) and the interconnectivity matrices between
the two colours (the "threat" contact matrices or TCM).

There are a total of 4 matrices, 2 DCM (white vs white and black vs black) and
2 TCM (white vs black and black vs white). These matrices are implemented
within the same structure in the code, and printed together as a 32x32 binary
matrix, where the value M(i,j) can be 0 or 1. A value of 1 means that the piece
i has the piece j within the reach of his possible movements, and 0 otherwise.
Note that for M(i,j) to be 1, the position where j is must be accessible for i
as per chess rules (e.g. no other pieces blocking the way, if the piece in i is
the king no movements to threatened squares.

The program keeps track of the promoted pawns too, 

The input requires Extended Position Description file format (EPD), but readily
available software deals with chess format conversion (see below).

##### Install

For the moment the code is all contained in a single .c source
file. It can be compiled using the provided Makefile:

```
make
```

##### Usage

Simply run the executable with the EPD input file as argument to the `-i` flag:

```
./cmatrix -i <file.epd>
```
The `-v` flag prints a lot of verbose, usually for debugging purposes.

###### Filtering and cleaning the PGN files

In the folder `data` there are some example PGN files from Mark Hebden.
For now, the PGN file needs to have one move per line. I suggest firstly check
the inegrity/validity of the PNG files with `pgn-extract -r -s` (can be
installed in ubuntu by `sudo apt-get update pgn-extract`) and then use only the
games with no errors:
```
pgn-extract -o<outputfile.epd> -C -D -N -V -Wepd <inputfile>
```
This command serves as a filter to games which have impossible moves or no
result (in addition, `-C` flag removes useless comments, `-N` removes
annotations, `-V` removes variations and `-D` removes duplicated games).  In
addition, it outputs in EPD format, which is lovely since it is also serving as
a format conversion tool (accepts a list of inputs, see
[https://www.cs.kent.ac.uk/people/staff/djb/pgn-extract/help.html]).

So, if you have a number of files, list them in a `file.txt` and specify
the list as `-f` argument. :
```
pgn-extract -o<outputfile.epd> -C -D -N -V -Wepd -f<filelist.txt>
```

**NOTE ON THE EPD FORMAT**

This format is more complete and easier to parse than the more popular PGN
format. This comes in detriment of user readability. But it saves a lot of
lines of code (and possible errors coming along with it).

##### Soon... and TODO

Read PGN files?

Still to decide the output format (there will be one 32x32
matrix per turn).
