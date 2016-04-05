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
void calcCM(int**,int**,int[],int**);

/* Pieces */
const char *pText[] = {
    "   ",
    "RLb", "BLb", "NLb", "Kb ",  "Qb ",  "NRb", "BRb", "RRb",
    "p8b", "p7b", "p6b", "p5b", "p4b", "p3b", "p2b", "p1b",
    "p1w", "p2w", "p3w", "p4w", "p5w", "p6w", "p7w", "p8w",
    "RLw", "BLw", "NLw", "Kw ",  "Qw ",  "NRw", "BRw", "RRw"
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

    printBoard_num(board);
  //  fprintf(stdout, "\n");
   // printBoard_txt(board);

    int **cm;
    cm = allocCM();
    calcCM(cm, board, promotedPawns, passedPawns);
    //printCM(cm);

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

void calcCM(int **cm, int **b, int sPawns[], int **pPawns) {
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            int p;
            p = b[i][j]; // Get type of piece
            //fprintf(stdout,"%d: ", p);
            if (sPawns[p] != 0) { // Correct type of piece for upgraded pawns
                p = sPawns[p];
            }
            /* CASTLE */
            if (p == 1 || p == 8 || p == 25 || p == 32) {
                fprintf(stdout, "%d(%s) - Castle\n", p, pText[p]);
                int n;
                // Calculate possible moves
                // 1) left
                for (n = j-1; n >= 0; n--) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        cm[p][b[i][n]] = 1;
                        break;
                    }
                }
                // 2) right
                for (n = j+1; n < 8; n++) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        cm[p][b[i][n]] = 1;
                        break;
                    }
                }
                // 3) up
                for (n = i-1; n >= 0; n--) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        cm[p][b[n][j]] = 1;
                        break;
                    }
                }
                // 4) down
                for (n = i+1; n < 8; n++) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        cm[p][b[n][j]] = 1;
                        break;
                    }
                }
            }
            /* KNIGHT */
            else if (p == 2 || p == 7 || p == 26 || p == 31) {
                fprintf(stdout, "%d(%s) - Knight\n", p, pText[p]);
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
                        cm[p][b[x][y]] = 1;
                    }
                }
            }
            /* BISHOP */
            else if (p == 3 || p == 6 || p == 27 || p == 30){
                fprintf(stdout, "%d(%s) - Bishop\n", p, pText[p]);
                int m, n;
                // 1) up left
                for (n = j-1, m = i-1; n >= 0 && m >= 0; n--, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 2) up right
                for (n = j+1, m = i-1; n < 8 && m >= 0; n++, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 3) down left
                for (n = j-1, m = i+1; n >= 0 && m < 8; n--, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 4) down right
                for (n = j+1, m = i+1; n < 8 && m < 8; n++, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
            }
            /* KING */
            else if (p == 4 || p == 28) {
                fprintf(stdout, "%d(%s) - King\n", p, pText[p]);
                // Castling and hindered moves are not considered
                int m, n;
                for (n = j-1; n <= j+1; n++) {
                    for (m = i-1; m <= i+1; m++) {
                        if (n < 0 || n > 7 || m < 0 || m > 7)
                            continue;
                        if (n == j && m == i)
                            continue;
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                    }
                }
            }
            /* QUEEN */
            else if (p == 5 || p == 29) {
                fprintf(stdout, "%d(%s) - Queen\n", p, pText[p]);
                int m, n;
                // Calculate possible moves
                // 1) left
                for (n = j-1; n >= 0; n--) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        cm[p][b[i][n]] = 1;
                        break;
                    }
                }
                // 2) right
                for (n = j+1; n < 8; n++) {
                    if (b[i][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i][n], pText[b[i][n]], i, n);
                        cm[p][b[i][n]] = 1;
                        break;
                    }
                }
                // 3) up
                for (n = i-1; n >= 0; n--) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        cm[p][b[n][j]] = 1;
                        break;
                    }
                }
                // 4) down
                for (n = i+1; n < 8; n++) {
                    if (b[n][j] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[n][j], pText[b[n][j]], n, j);
                        cm[p][b[n][j]] = 1;
                        break;
                    }
                }
                // 5) up left
                for (n = j-1, m = i-1; n >= 0 && m >= 0; n--, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 6) up right
                for (n = j+1, m = i-1; n < 8 && m >= 0; n++, m--) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 7) down left
                for (n = j-1, m = i+1; n >= 0 && m < 8; n--, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
                // 8) down right
                for (n = j+1, m = i+1; n < 8 && m < 8; n++, m++) {
                    if (b[m][n] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[m][n], pText[b[m][n]], m, n);
                        cm[p][b[m][n]] = 1;
                        break;
                    }
                }
            }
            /* BLACK PAWN */
            else if (p >= 9 && p <= 14) {
                fprintf(stdout, "%d(%s) - Black pawn\n", p, pText[p]);
                if (i+1 < 8 && j-1 >=0) {
                    if (b[i+1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i+1][j-1], pText[b[i+1][j-1]], i+1, j-1);
                        cm[p][b[i+1][j-1]] = 1;
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
                    }
                    if (pPawns[i+1][j+1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i+1][j+1], pText[pPawns[i+1][j+1]], i+1, j+1);
                        cm[p][pPawns[i+1][j+1]] = 1;
                    }
                }
            }
            /* WHITE PAWN */
            else if (p >= 9 && p <= 14) {
                fprintf(stdout, "%d(%s) - White pawn\n", p, pText[p]);
                if (i-1 >= 0 && j-1 >=0) {
                    if (b[i-1][j-1] != 0) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", b[i-1][j-1], pText[b[i-1][j-1]], i-1, j-1);
                        cm[p][b[i-1][j-1]] = 1;
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
                    }
                    if (pPawns[i-1][j+1]) {
                        fprintf(stdout, " Contact with %d(%s) in %d:%d\n", pPawns[i-1][j+1], pText[pPawns[i-1][j+1]], i-1, j+1);
                        cm[p][pPawns[i-1][j+1]] = 1;
                    }
                }
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
    for (i = 1; i < 33; i++) {
        for (j = 1; j < 33; j++) {
            fprintf(stdout, "%d ", cm[i][j]);
        }
        fprintf(stdout, "\n");
    }
}
