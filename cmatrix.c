#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

struct globalArgs_t {
    int input;                  /* -i input */
    char *inFileName;
    FILE *inFile;
    int verbose;                /* -v option */
    int help;                   /* -h option */
} globalArgs;

static const char *optString = "i:hv?";

/* Headers */
/** Prints help message */
void usage(char*);
void printBoard_num(int**);
void printBoard_txt(int**);
void printCM(int**);
int **allocBoard(void);
void restartBoard(int**);
int **allocCM(void);
void calcCM(int**,int**,int[],int**,int[]);

/* Pieces */
const char *pText[] = {
    "   ",
    "RLb", "BLb", "NLb", "Qb ",  "Kb ",  "NRb", "BRb", "RRb",
    "p8b", "p7b", "p6b", "p5b", "p4b", "p3b", "p2b", "p1b",
    "p1w", "p2w", "p3w", "p4w", "p5w", "p6w", "p7w", "p8w",
    "RLw", "BLw", "NLw", "Qw ",  "Kw ",  "NRw", "BRw", "RRw"
};

/* Main */
int main ( int argc, char *argv[] )
{
    clock_t start = clock();
    /****************************/
    /* Parse cmd line arguments */
    /****************************/
    globalArgs.input = 1;             /* Output is default to stdout */
    globalArgs.inFileName = NULL;     /* Output file name */
    globalArgs.inFile = stdout;       /* Output FILE handle */
    globalArgs.verbose = 0;           /* Prints heaps of stuff */
    
    int index;
    
    opterr = 0;
    
    int c;
    while ((c = getopt (argc, argv, optString)) != -1) {
        char *ptr = NULL;
        switch (c) {
            case 'i':
                globalArgs.inFileName = optarg;
                break;
            case 'v':
                globalArgs.verbose = 1;
                break;
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
            case '?':
                if (optopt == '1')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }
    
    for (index = optind; index < argc; index++) {
        fprintf (stderr, "Argument '%s' does not correspond to any options.\n", argv[index]);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /*********************/
    /* Declare variables */
    /*********************/
    
    // Initialise board game
    int **board;
    board = allocBoard();
    restartBoard(board);

    int promotedPawns[32];
    int i;
    for (i = 0; i < 32; i++) {
        promotedPawns[i] = 0;
    }

    // Initialise seudoboard for passing pawns
    int **passedPawns;
    passedPawns = allocBoard();

    // Castles/king moves made?
    // blackLeft, blackKing, blackRight, whiteLeft, whiteKing, whiteRight
    int Castling[6] = {0, 0, 0, 0, 0, 0};

    // NOTES FOR MOVES
    // Remember to update the promotedPawns if a pawn is promoted. Choose the numbers
    // corresponding to the appropriate colour, and any of the sides.
    // Also, remember to upgrade the Castling array with values of 1 when castles or
    // king move. Also when Castling!
    // Last, remember to tag the passedPawns when pawns move long :) and then clean it
    // in the next turn resetting to 0

    printBoard_num(board);
    // fprintf(stdout, "\n");
    // printBoard_txt(board);

    int **cm;
    cm = allocCM();
    calcCM(cm, board, promotedPawns, passedPawns, Castling);
    printCM(cm);

    clock_t end = clock();
    //fprintf(stderr, "Time elapsed = %d (%.3f secs)\n", (int) (end - start), (float) (end - start) / CLOCKS_PER_SEC);
}

int **allocBoard(void) {
    int **b;
    if (( b = malloc( 8*sizeof( int* ) )) == NULL ) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }

    int i;
    for (i = 0; i < 8; i++) {
        if (( b[i] = malloc( 8*sizeof( int ) )) == NULL ) {
            fprintf(stderr, "ERROR: out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    int j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            b[i][j] = 0;
        }
    }
    return b;
}

int **allocCM(void) {
    int **cm;
    if (( cm = malloc( 33*sizeof( int* ) )) == NULL ) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }

    int i;
    for (i = 0; i < 33; i++) {
        if (( cm[i] = malloc( 32*sizeof( int) )) == NULL ) {
            fprintf(stderr, "ERROR: out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    int j;
    for (i = 0; i < 33; i++) {
        for (j = 0; j < 33; j++) {
            cm[i][j] = 0;
        }
    }
    return cm;
}

void restartBoard(int **b) {
    int i, j, p;
    p = 1;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (i < 2 || i > 5)
            {
                b[i][j] = p;
                p++;
            }
            else
            {
                b[i][j] = 0;
            }
        }
    }
}

void usage(char *pname) {
    fprintf(stderr, "%s -i <file.pgn> [OPTIONS]\n", pname);
    fprintf(stderr, "  -s       PGN input file to parse\n"
                    "  -v       Prints heaps of useless stuff. Mainly for debugging\n"
                    "  -h       Prints (this) help message\n");
}

/*
 * Calculates the contact matrix between each pair of pieces.
 * The values between pairs of the same colour indicate "protection", while
 * values between pairs of different colour indicate "threat".
 * A value of 1 indicates "protection"/"threat" and a value of 0 indicates
 * no "protection"/"threat".
 */

void calcCM(int **cm, int **b, int sPawns[], int **pPawns, int Castling[]) {
    int wAccessible[8][8];
    int bAccessible[8][8];
    int i, j;
    for (i = 0; i < 8; i++) {
        for ( j = 0; j < 8; j++) {
            wAccessible[i][j] = 0;
            bAccessible[i][j] = 0;
        }
    }
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            int p, p2;
            p = b[i][j];  // Get type of piece
            p2 = b[i][j]; // Get type of piece, used only in case of promoted Pawns
            //fprintf(stdout,"%d: ", p);
            int isPromoted = 0;
            if (sPawns[p] != 0) { // Correct type of piece for upgraded pawns
                p = sPawns[p];
                isPromoted = 1;
            }
            /* CASTLE */
            if (p == 1 || p == 8 || p == 25 || p == 32) {
                fprintf(stdout, "%d(%s) - Castle in position %d:%d\n", p, pText[p], i, j);
                int n;
                // Calculate possible moves
                // 1) left
                for (n = j-1; n >= 0; n--) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        if ( isPromoted )
                            cm[p2][b[i][n]] = 1;
                        else 
                            cm[p][b[i][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 1 || p == 8)
                            bAccessible[i][n]++;
                        else
                            wAccessible[i][n]++;
                    }
                }
                // 2) right
                for (n = j+1; n < 8; n++) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        if ( isPromoted )
                            cm[p2][b[i][n]] = 1;
                        else 
                            cm[p][b[i][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 1 || p == 8)
                            bAccessible[i][n]++;
                        else
                            wAccessible[i][n]++;
                    }
                }
                // 3) up
                for (n = i-1; n >= 0; n--) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        if ( isPromoted )
                            cm[p2][b[n][j]] = 1;
                        else 
                            cm[p][b[n][j]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 1 || p == 8)
                            bAccessible[n][j]++;
                        else
                            wAccessible[n][j]++;
                    }
                }
                // 4) down
                for (n = i+1; n < 8; n++) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        if ( isPromoted )
                            cm[p2][b[n][j]] = 1;
                        else 
                            cm[p][b[n][j]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 1 || p == 8)
                            bAccessible[n][j]++;
                        else
                            wAccessible[n][j]++;
                    }
                }
            }
            /* KNIGHT */
            else if (p == 2 || p == 7 || p == 26 || p == 31) {
                fprintf(stdout, "%d(%s) - Knight in position %d:%d\n", p, pText[p], i, j);
                int K_moves[8][2] = { {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1} };
                int n;
                for (n = 0; n < 8; n++ ) {
                    int x = i + K_moves[n][0];
                    int y = j + K_moves[n][1];
                    if (x > 7 || x < 0 || y > 7 || y < 0) {
                        continue;
                    }
                    if (b[x][y] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[x][y], pText[b[x][y]], x, y);
                        if ( isPromoted )
                            cm[p2][b[x][y]] = 1;
                        else
                            cm[p][b[x][y]] = 1;
                    } else {
                        // Space accessible but empty
                        if (p == 2 || p == 7)
                            bAccessible[x][y]++;
                        else
                            wAccessible[x][y]++;
                    }
                }
            }
            /* BISHOP */
            else if (p == 3 || p == 6 || p == 27 || p == 30){
                fprintf(stdout, "%d(%s) - Bishop in position %d:%d\n", p, pText[p], i, j);
                int m, n;
                // 1) up left
                for (n = j-1, m = i-1; n >= 0 && m >= 0; n--, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 3 || p == 6)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 2) up right
                for (n = j+1, m = i-1; n < 8 && m >= 0; n++, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 3 || p == 6)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 3) down left
                for (n = j-1, m = i+1; n >= 0 && m < 8; n--, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 3 || p == 6)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 4) down right
                for (n = j+1, m = i+1; n < 8 && m < 8; n++, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 3 || p == 6)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
            }
            /* QUEEN */
            else if (p == 4 || p == 28) {
                fprintf(stdout, "%d(%s) - Queen in position %d:%d\n", p, pText[p], i, j);
                int m, n;
                // Calculate possible moves
                // 1) left
                for (n = j-1; n >= 0; n--) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        if ( isPromoted )
                            cm[p2][b[i][n]] = 1;
                        else
                            cm[p][b[i][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[i][n]++;
                        else
                            wAccessible[i][n]++;
                    }
                }
                // 2) right
                for (n = j+1; n < 8; n++) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        if ( isPromoted )
                            cm[p2][b[i][n]] = 1;
                        else
                            cm[p][b[i][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[i][n]++;
                        else
                            wAccessible[i][n]++;
                    }
                }
                // 3) up
                for (n = i-1; n >= 0; n--) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        if ( isPromoted )
                            cm[p2][b[n][j]] = 1;
                        else
                            cm[p][b[n][j]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[n][j]++;
                        else
                            wAccessible[n][j]++;
                    }
                }
                // 4) down
                for (n = i+1; n < 8; n++) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        if ( isPromoted )
                            cm[p2][b[n][j]] = 1;
                        else
                            cm[p][b[n][j]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[n][j]++;
                        else
                            wAccessible[n][j]++;
                    }
                }
                // 5) up left
                for (n = j-1, m = i-1; n >= 0 && m >= 0; n--, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 6) up right
                for (n = j+1, m = i-1; n < 8 && m >= 0; n++, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 7) down left
                for (n = j-1, m = i+1; n >= 0 && m < 8; n--, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
                // 8) down right
                for (n = j+1, m = i+1; n < 8 && m < 8; n++, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        if ( isPromoted )
                            cm[p2][b[m][n]] = 1;
                        else
                            cm[p][b[m][n]] = 1;
                        break;
                    } else {
                        // Space accessible but empty
                        if (p == 4)
                            bAccessible[m][n]++;
                        else
                            wAccessible[m][n]++;
                    }
                }
            }
            /* BLACK PAWN */
            else if (p >= 9 && p <= 16) {
                fprintf(stdout, "%d(%s) - Black pawn in position %d:%d\n", p, pText[p], i, j);
                if (i+1 < 8 && j-1 >=0) {
                    if (b[i+1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i+1][j-1], pText[b[i+1][j-1]], i+1, j-1);
                        cm[p][b[i+1][j-1]] = 1;
                    } else {
                        // Space accessible but empty
                        bAccessible[i+1][j-1]++;
                    }
                    if (pPawns[i+1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i+1][j-1], pText[pPawns[i+1][j-1]], i+1, j-1);
                        cm[p][pPawns[i+1][j-1]] = 1;
                    }
                }
                if (i+1 < 8 && j+1 < 8) {
                    if (b[i+1][j+1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i+1][j+1], pText[b[i+1][j+1]], i+1, j+1);
                        cm[p][b[i+1][j+1]] = 1;
                    } else {
                        // Space accessible but empty
                        bAccessible[i+1][j+1]++;
                    }
                    if (pPawns[i+1][j+1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i+1][j+1], pText[pPawns[i+1][j+1]], i+1, j+1);
                        cm[p][pPawns[i+1][j+1]] = 1;
                    }
                }
            }
            /* WHITE PAWN */
            else if (p >= 17 && p <= 24) {
                fprintf(stdout, "%d(%s) - White pawn in position %d:%d\n", p, pText[p], i, j);
                if (i-1 >= 0 && j-1 >=0) {
                    if (b[i-1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i-1][j-1], pText[b[i-1][j-1]], i-1, j-1);
                        cm[p][b[i-1][j-1]] = 1;
                    } else {
                        // Space accessible but empty
                        wAccessible[i-1][j-1]++;
                    }
                    if (pPawns[i-1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i-1][j-1], pText[pPawns[i-1][j-1]], i-1, j-1);
                        cm[p][pPawns[i-1][j-1]] = 1;
                    }
                }
                if (i-1 >= 0 && j+1 < 8) {
                    if (b[i-1][j+1]) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i-1][j+1], pText[b[i-1][j+1]], i-1, j+1);
                        cm[p][b[i-1][j+1]] = 1;
                    } else {
                        // Space accessible but empty
                        wAccessible[i-1][j+1]++;
                    }
                    if (pPawns[i-1][j+1]) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i-1][j+1], pText[pPawns[i-1][j+1]], i-1, j+1);
                        cm[p][pPawns[i-1][j+1]] = 1;
                    }
                }
            }
            /* KING */
            else if (p == 5 || p == 29) {
                fprintf(stdout, "%d(%s) - King in position %d:%d\n", p, pText[p], i, j);
                int m, n;
                for (n = j-1; n <= j+1; n++) {
                    for (m = i-1; m <= i+1; m++) {
                        //fprintf(stdout, " KING: checking position %d:%d\n", m, n);
                        if (n < 0 || n > 7 || m < 0 || m > 7) {
                            fprintf(stdout, "       out of board\n");
                            continue;
                        }
                        if (n == j && m == i) {
                            //fprintf(stdout, "       the king is there\n");
                            continue;
                        }
                        if (b[m][n] != 0) {
                            fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                            if ( isPromoted )
                                cm[p2][b[m][n]] = 1;
                            else
                                cm[p][b[m][n]] = 1;
                        } else {
                            //fprintf(stdout, " Space accessible but empty\n");
                            // Space accessible but empty
                            if (p == 5)
                                bAccessible[m][n]++;
                            else
                                wAccessible[m][n]++;
                        }
                    }
                }
                // Castling TODO: check spaces are free of threat: now easy to do with the new bAccessible
                // and wAccessible matrices :)
                // Also TODO is to transfer the KING to the end of the CM construction, out of this loop,
                // since it needs to know the enemies pieces accessible area to know if it can castle
                if (p == 5) {
                    int is_check = 0;
                    int s;
                    for (s = 17; s <= 32; s++) {
                        if (cm[s][p]) {
                            fprintf(stdout, " King %d is under check by %d\n", p, s);
                            is_check = 1;
                            break;
                        }
                    }
                    fprintf(stdout, " King %d is in check? %d\n", p, is_check);
                    if (Castling[1] != 1 && is_check != 1) {
                        // Long black castling
                        if (Castling[0] != 1 && b[0][1]+b[0][2]+b[0][3] == 0) {
                            // YES
                            continue;
                        }
                        // Short black castling possible
                        if (Castling[2] != 1 && b[0][5]+b[0][6] == 0) {
                            // YES
                            continue;
                        }
                    }
                } else {
                    int is_check = 0;
                    int s;
                    for (s = 9; s <= 16; s++) {
                        if (cm[p][s]) {
                            fprintf(stdout, " King %d is under check by %d\n", p, s);
                            is_check = 1;
                            break;
                        }
                    }
                    fprintf(stdout, " King %d is in check? %d\n", p, is_check);
                    if (Castling[4] != 1 && is_check != 1) {
                        // Long white castling
                        if (Castling[3] != 1 && b[7][1]+b[7][2]+b[7][3] == 0) {
                            // YES
                            continue;
                        }
                        // Short white castling
                        if (Castling[5] != 1 && b[7][5]+b[7][6] == 0) {
                            // YES
                            continue;
                        }
                    }
                }
                // Hindered moves!! TODO: King is in contact with a pieace of the
                // same colour only if that other piece is either not under threat
                // at all or under threat of only one enemy
            }
        }
    }
}

void printBoard_num(int **b) {
    fprintf(stdout, "     [0] [1] [2] [3] [4] [5] [6] [7] \n");
    fprintf(stdout, "    +---+---+---+---+---+---+---+---+\n");
    int i, j;
    for (i = 0; i < 8; i++) {
        fprintf(stdout, "[%d] |", i);
        for (j = 0; j < 8; j++) {
            fprintf(stdout, "%2d |", b[i][j]);
        }
        fprintf(stdout, " %d", 8-i);
        fprintf(stdout, "\n");
        fprintf(stdout, "    +---+---+---+---+---+---+---+---+\n");
    }
    fprintf(stdout, "      a   b   c   d   e   f   g   h  \n");
}

void printBoard_txt(int **b) {
    fprintf(stdout, "     [0] [1] [2] [3] [4] [5] [6] [7] \n");
    fprintf(stdout, "    +---+---+---+---+---+---+---+---+\n");
    int i, j;
    for (i = 0; i < 8; i++) {
        fprintf(stdout, "[%d] |", i);
        for (j = 0; j <8; j++) {
            fprintf(stdout, "%3s|", pText[ b[i][j] ] );
        }
        fprintf(stdout, " %d", 8-i);
        fprintf(stdout, "\n");
        fprintf(stdout, "    +---+---+---+---+---+---+---+---+\n");
    }
    fprintf(stdout, "      a   b   c   d   e   f   g   h  \n");
}

void printCM(int **cm) {
    int i, j;
    fprintf(stdout, "   ");
    for (i = 1; i < 33; i++) fprintf(stdout, "%2d ", i);
    fprintf(stdout, "\n");
    for (i = 1; i < 33; i++) {
        fprintf(stdout, "%2d ", i);
        for (j = 1; j < 33; j++) {
            fprintf(stdout, "%2d ", cm[i][j]);
        }
        fprintf(stdout, "\n");
    }
}
