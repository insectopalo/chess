### Chess software

###### About

This utility calculates the intra connectivity matrix for each colour (the
"defensive" contact matrices or DCM) and the interconnectivity matrices between
the two colours (the "threat" contact matrices or TCM).

There are a total of 4 matrices, 2 DCM (white vs white and black vs black) and
2 TCM (white vs black and black vs white). These matrices are implemented
within the same structure in the code, and printed together as a 32x32 binary
matrix, where the value M(i,j) can be 0 or 1. A value of 1 means that the piece
i has the piece j within the reach of his possible movements, and 0 otherwise.
Note that for M(i,j) to be 1, the position where j is must be accessible for i
as per chess rules (e.g. no other pieces blocking the way, if the piece in i is the
king no movements to threatened squares.

The program keeps track of the promoted pawns too, 

###### Install

For the moment the code is all contained in a single .c source
file. It can be compiled using the provided Makefile:

```
make
```

###### Usage

Simply run the executable

```
./cmake
```

###### Soon... and TODO

The idea is to read PGN files and calculate contact matrices for each of the
turns. The accessible space is also calculated for each team on each turn

Read PGN files. Still to decide the output format (there will be one 32x32
matrix per turn).
