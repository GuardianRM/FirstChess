/*
   FirstChess - Freeware, by Pham Hong Nguyen
   Version: beta
 */
/*
   * BASIC PARTS:                          *
   * Some definitions                      *
   * Board representation and main varians *
   * Move generator                        *
   * Evaluation for current position       *
   * Make and Take back a move, IsInCheck  *
   * Search function - a typical alphabeta *
   * Utility                               *
   * Main program                          *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <ctime>
#include <string>
#include <cstdlib>
#include <random>
#include <thread>
#include <cstdio>
#include <ctime>
#include <future>
#include <thread>
#include <iomanip>
#include <algorithm>

#define TIMEALLOC (55000)

typedef enum mybool {
false = 0, true = 1} mybool;
/*
   ****************************************************************************
   * Some definitions                                                         *
   ****************************************************************************
 */
#define   PAWN    0
#define   KNIGHT  1
#define   BISHOP  2
#define   ROOK    3
#define   QUEEN   4
#define   KING    5
#define   EMPTY   6
#define   WHITE   0
#define   BLACK   1

#define   VALUE_PAWN      100
#define   VALUE_KNIGHT    300
#define   VALUE_BISHOP    300
#define   VALUE_ROOK      500
#define   VALUE_QUEEN     900
#define   VALUE_KING      10000

#define   MATE            10000   /* equal value of King, losing King==mate */

/*
   Some board constants to make moves easier to read
 */
#define ONE_RANK (8)
#define TWO_RANKS (16)
#define UP (-8)
#define DOWN (8)
#define LEFT (-1)
#define RIGHT (1)
#define RIGHT_UP (RIGHT+UP)
#define LEFT_UP (LEFT+UP)
#define RIGHT_DOWN (RIGHT+DOWN)
#define LEFT_DOWN (LEFT+DOWN)

#define COL(pos) ((pos)&7)
#define ROW(pos) (((unsigned)pos)>>3)
/*
   ****************************************************************************
   * Board representation and main varians                                    *
   ****************************************************************************
 */
///* Board representation */
/* Board representation */
int             initial_piece[64] =
{
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
};

int             piece[64];

int             initial_color[64] =
{
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE
};

int             color[64];

int             side;           /* side to move, value = BLACK or WHITE */

/* For move generation */
#define MOVE_TYPE_NONE                  0
#define MOVE_TYPE_NORMAL                1
#define MOVE_TYPE_CASTLE                2
#define MOVE_TYPE_ENPASANT              3
#define MOVE_TYPE_PROMOTION_TO_QUEEN    4
#define MOVE_TYPE_PROMOTION_TO_ROOK     5
#define MOVE_TYPE_PROMOTION_TO_BISHOP   6
#define MOVE_TYPE_PROMOTION_TO_KNIGHT   7


typedef struct tag_MOVE {
    int             from,
                    dest,
                    type;
}               MOVE;
/* For storing all moves of game */
typedef struct tag_HIST {
    MOVE            m;
    int             cap;
}               HIST;

HIST            hist[6000];     /* Game length < 6000 */

int             hdp;            /* Current move order */

/* For searching */
int             nodes;          /* Count all visited nodes when searching */
int             ply;            /* ply of search */

/*
   ****************************************************************************
   * Move generator                                                           *
   * Lack: no enpassant, no castle                                            *
   ****************************************************************************
 */
void            Gen_Push(int from, int dest, int type, MOVE * pBuf, int *pMCount)
{
    MOVE            move;
    move.from = from;
    move.dest = dest;
    move.type = type;
    pBuf[*pMCount] = move;
    *pMCount = *pMCount + 1;
}

void            Gen_PushNormal(int from, int dest, MOVE * pBuf, int *pMCount)
{
    Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
}

/* Pawn can promote */
void            Gen_PushPawn(int from, int dest, MOVE * pBuf, int *pMCount)
{
    if (dest > 7 && dest < 56)
        Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
    else {
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_QUEEN, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_ROOK, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_BISHOP, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_KNIGHT, pBuf, pMCount);
    }
}

/* Gen all moves of current_side to move and push them to pBuf, return number of moves */
int             Gen(int current_side, MOVE * pBuf)
{
    int             i,
                    k,
                    y,
                    row,
                    col,
                    movecount;
    movecount = 0;

    for (i = 0; i < 64; i++)    /* Scan all board */
        if (color[i] == current_side) {
            switch (piece[i]) {
            case PAWN:
                col = COL(i);
                row = ROW(i);
                if (current_side == BLACK) {
                    if (color[i + ONE_RANK] == EMPTY)
                        Gen_PushPawn(i, i + ONE_RANK, pBuf, &movecount);
                    if (row == 1 && color[i + ONE_RANK] == EMPTY && color[i + TWO_RANKS] == EMPTY)
                        Gen_PushNormal(i, i + TWO_RANKS, pBuf, &movecount);
                    if (col && color[i + 7] == WHITE)
                        Gen_PushNormal(i, i + 7, pBuf, &movecount);
                    if (col < 7 && color[i + 9] == WHITE)
                        Gen_PushNormal(i, i + 9, pBuf, &movecount);
                } else {
                    if (color[i - ONE_RANK] == EMPTY)
                        Gen_PushPawn(i, i - ONE_RANK, pBuf, &movecount);
                    if (row == 6 && color[i - ONE_RANK] == EMPTY && color[i - TWO_RANKS] == EMPTY)
                        Gen_PushNormal(i, i - TWO_RANKS, pBuf, &movecount);
                    if (col && color[i - 9] == BLACK)
                        Gen_PushNormal(i, i - 9, pBuf, &movecount);
                    if (col < 7 && color[i - 7] == BLACK)
                        Gen_PushNormal(i, i - 7, pBuf, &movecount);
                }
                break;

            case QUEEN:         /* == BISHOP+ROOK */
            case BISHOP:
                for (y = i - 9; y >= 0 && COL(y) != 7; y -= 9) {        /* go left up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (y = i - 7; y >= 0 && COL(y) != 0; y -= 7) {        /* go right up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (y = i + 9; y < 64 && COL(y) != 0; y += 9) {        /* go right down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (y = i + 7; y < 64 && COL(y) != 7; y += 7) {        /* go right down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                if (piece[i] == BISHOP)
                    break;

                /* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
            case ROOK:
                col = COL(i);
                for (k = i - col, y = i - 1; y >= k; y--) {     /* go left */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (k = i - col + 7, y = i + 1; y <= k; y++) { /* go right */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (y = i - 8; y >= 0; y -= 8) {       /* go up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                for (y = i + 8; y < 64; y += 8) {       /* go down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;

                }
                break;

            case KNIGHT:
                col = COL(i);
                y = i - 6;
                if (y >= 0 && col < 6 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 10;
                if (y >= 0 && col > 1 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 15;
                if (y >= 0 && col < 7 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 17;
                if (y >= 0 && col > 0 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 6;
                if (y < 64 && col > 1 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 10;
                if (y < 64 && col < 6 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 15;
                if (y < 64 && col > 0 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 17;
                if (y < 64 && col < 7 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                break;

            case KING:
                col = COL(i);
                if (col && color[i - 1] != current_side)
                    Gen_PushNormal(i, i - 1, pBuf, &movecount); /* left */
                if (col < 7 && color[i + 1] != current_side)
                    Gen_PushNormal(i, i + 1, pBuf, &movecount); /* right */
                if (i > 7 && color[i - 8] != current_side)
                    Gen_PushNormal(i, i - 8, pBuf, &movecount); /* up */
                if (i < 56 && color[i + 8] != current_side)
                    Gen_PushNormal(i, i + 8, pBuf, &movecount); /* down */
                if (col && i > 7 && color[i - 9] != current_side)
                    Gen_PushNormal(i, i - 9, pBuf, &movecount); /* left up */
                if (col < 7 && i > 7 && color[i - 7] != current_side)
                    Gen_PushNormal(i, i - 7, pBuf, &movecount); /* right up */
                if (col && i < 56 && color[i + 7] != current_side)
                    Gen_PushNormal(i, i + 7, pBuf, &movecount); /* left down */
                if (col < 7 && i < 56 && color[i + 9] != current_side)
                    Gen_PushNormal(i, i + 9, pBuf, &movecount); /* right down */
                break;
            default:
                puts("piece type unknown");
                assert(false);
            }
        }
    return movecount;
}

/*
   ****************************************************************************
   * Evaluation for current position - main "brain" function                  *
   * Lack: almost no knowlegde                                                *
   ****************************************************************************
 */
int             Eval()
{
    int             value_piece[6] =
    {VALUE_PAWN, VALUE_KNIGHT, VALUE_BISHOP, VALUE_ROOK, VALUE_QUEEN, VALUE_KING};
    int             i,
                    score = 0;
    for (i = 0; i < 64; i++) {
        if (color[i] == WHITE)
            score += value_piece[piece[i]];
        else if (color[i] == BLACK)
            score -= value_piece[piece[i]];
    }
    if (side == WHITE)
        return score;
    return -score;
}

/*
   ****************************************************************************
   * Make and Take back a move, IsInCheck                                     *
   ****************************************************************************
 */
/* Check and return 1 if side is in check */
int             IsInCheck(int current_side)
{
    int             k,
                    h,
                    y,
                    row,
                    col,
                    xside;
    xside = (WHITE + BLACK) - current_side;     /* opposite current_side, who
                                                 * may be checking */
    /* Find King */
    for (k = 0; k < 64; k++)
        if (piece[k] == KING && color[k] == current_side)
            break;

    row = ROW(k), col = COL(k);
    /* Check attacking of Knight */
    if (col > 0 && row > 1 && color[k - 17] == xside && piece[k - 17] == KNIGHT)
        return 1;
    if (col < 7 && row > 1 && color[k - 15] == xside && piece[k - 15] == KNIGHT)
        return 1;
    if (col > 1 && row > 0 && color[k - 10] == xside && piece[k - 10] == KNIGHT)
        return 1;
    if (col < 6 && row > 0 && color[k - 6] == xside && piece[k - 6] == KNIGHT)
        return 1;
    if (col > 1 && row < 7 && color[k + 6] == xside && piece[k + 6] == KNIGHT)
        return 1;
    if (col < 6 && row < 7 && color[k + 10] == xside && piece[k + 10] == KNIGHT)
        return 1;
    if (col > 0 && row < 6 && color[k + 15] == xside && piece[k + 15] == KNIGHT)
        return 1;
    if (col < 7 && row < 6 && color[k + 17] == xside && piece[k + 17] == KNIGHT)
        return 1;
    /* Check horizontal and vertical lines for attacking of Queen, Rook, King */
    /* go down */
    y = k + 8;
    if (y < 64) {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY)
            for (y += 8; y < 64; y += 8) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY)
                    break;

            }
    }
    /* go left */
    y = k - 1;
    h = k - col;
    if (y >= h) {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY)
            for (y--; y >= h; y--) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go right */
    y = k + 1;
    h = k - col + 7;
    if (y <= h) {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY)
            for (y++; y <= h; y++) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go up */
    y = k - 8;
    if (y >= 0) {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY)
            for (y -= 8; y >= 0; y -= 8) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* Check diagonal lines for attacking of Queen, Bishop, King, Pawn */
    /* go right down */
    y = k + 9;
    if (y < 64 && COL(y) != 0) {
        if (color[y] == xside) {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y += 9; y < 64 && COL(y) != 0; y += 9) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go left down */
    y = k + 7;
    if (y < 64 && COL(y) != 7) {
        if (color[y] == xside) {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y += 7; y < 64 && COL(y) != 7; y += 7) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;

            }
    }
    /* go left up */
    y = k - 9;
    if (y >= 0 && COL(y) != 7) {
        if (color[y] == xside) {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y -= 9; y >= 0 && COL(y) != 7; y -= 9) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;

            }
    }
    /* go right up */
    y = k - 7;
    if (y >= 0 && COL(y) != 0) {
        if (color[y] == xside) {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y -= 7; y >= 0 && COL(y) != 0; y -= 7) {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    return 0;
}

int             MakeMove(MOVE m)
{
    int             r;
    hist[hdp].m = m;
    hist[hdp].cap = piece[m.dest];
    piece[m.dest] = piece[m.from];
    piece[m.from] = EMPTY;
    color[m.dest] = color[m.from];
    color[m.from] = EMPTY;
    if (m.type >= MOVE_TYPE_PROMOTION_TO_QUEEN) {       /* Promotion */
        switch (m.type) {
        case MOVE_TYPE_PROMOTION_TO_QUEEN:
            piece[m.dest] = QUEEN;
            break;

        case MOVE_TYPE_PROMOTION_TO_ROOK:
            piece[m.dest] = ROOK;
            break;

        case MOVE_TYPE_PROMOTION_TO_BISHOP:
            piece[m.dest] = BISHOP;
            break;

        case MOVE_TYPE_PROMOTION_TO_KNIGHT:
            piece[m.dest] = KNIGHT;
            break;

        default:
            puts("impossible to get here...");
            assert(false);
        }
    }
    ply++;
    hdp++;
    r = !IsInCheck(side);
    side = (WHITE + BLACK) - side;      /* After making move, give turn to
                                         * opponent */
    return r;
}

void            TakeBack()
{                               /* undo what MakeMove did */
    side = (WHITE + BLACK) - side;
    hdp--;
    ply--;
    piece[hist[hdp].m.from] = piece[hist[hdp].m.dest];
    piece[hist[hdp].m.dest] = hist[hdp].cap;
    color[hist[hdp].m.from] = side;
    if (hist[hdp].cap != EMPTY)
        color[hist[hdp].m.dest] = (WHITE + BLACK) - side;
    else
        color[hist[hdp].m.dest] = EMPTY;
    if (hist[hdp].m.type >= MOVE_TYPE_PROMOTION_TO_QUEEN)       /* Promotion */
        piece[hist[hdp].m.from] = PAWN;
}

/*
   ****************************************************************************
   * Search function - a typical alphabeta, main search function              *
   * Lack: no any technique for move ordering                                 *
   ****************************************************************************
 */

int             Search(int alpha, int beta, int depth, MOVE * pBestMove)
{
    int             i,
                    value,
                    havemove,
                    movecnt;
    MOVE            moveBuf[200],
                    tmpMove;

    nodes++;                    /* visiting a node, count it */
    havemove = 0;
    pBestMove->type = MOVE_TYPE_NONE;
    movecnt = Gen(side, moveBuf);       /* generate all moves for current
                                         * position */
    /* loop through the moves */
    for (i = 0; i < movecnt; ++i) {
        if (!MakeMove(moveBuf[i])) {
            TakeBack();
            continue;
        }
        havemove = 1;
        if (depth - 1 > 0)      /* If depth is still, continue to search
                                 * deeper */
            value = -Search(-beta, -alpha, depth - 1, &tmpMove);
        else                    /* If no depth left (leaf node), go to
                                 * evalute that position */
            value = Eval();
        TakeBack();
        if (value > alpha) {
            /* This move is so good and caused a cutoff */
            if (value >= beta)
                return beta;
            alpha = value;
            *pBestMove = moveBuf[i];    /* so far, current move is the best
                                         * reaction for current position */
        }
    }
    if (!havemove) {            /* If no legal moves, that is checkmate or
                                 * stalemate */
        if (IsInCheck(side))
            return -MATE + ply; /* add ply to find the longest path to lose
                                 * or shortest path to win */
        else
            return 0;
    }
    return alpha;
}

MOVE
ComputerThink(int max_depth)
{
    MOVE            m;
    int             score;
    /* reset some values before searching */
    ply = 0;
    nodes = 0;
    /* search now */
    score = Search(-MATE, MATE, max_depth, &m);
    /* after searching, print results */
    printf("Search result: move = %c%d%c%d; nodes = %d, score = %d\n",
           'a' + COL(m.from),
           8 - ROW(m.from),
           'a' + COL(m.dest),
           8 - ROW(m.dest),
           nodes,
           score
        );
    return m;
}

/*
   ****************************************************************************
   * Utilities                                                                *
   ****************************************************************************
 */
void            PrintBoard()
{
    char            pieceName[] = "PNBRQKpnbrqk";
    int             i;
    for (i = 0; i < 64; i++) {
        if ((i & 7) == 0) {
            printf("   +---+---+---+---+---+---+---+---+\n");
            if (i <= 56) {
                printf(" %d |", 8 - (((unsigned) i) >> 3));
            }
        }
        if (piece[i] == EMPTY)
            printf("   |");
        else {
            printf(" %c |", pieceName[piece[i] + (color[i] == WHITE ? 0 : 6)]);
        }
        if ((i & 7) == 7)
            printf("\n");
    }
    printf("   +---+---+---+---+---+---+---+---+\n     a   b   c   d   e   f   g   h\n");
}


void            initboard(void)
{
    memcpy(piece, initial_piece, sizeof piece);
    memcpy(color, initial_color, sizeof color);
}

/*
BUGBUG:DRC -->FIXME
globals are for the lazy and insane.
*/
int             xboard = 0;
int             computer_side = -1;
int             selfplay = 0;
int             nocomp = 0;
int             no_go = 0;
int             playing = 0;
int             player = WHITE;
int             maxtime = 1000;
int             max_depth = 5;
int             max_moves = 40;

/*
Shamelessly cobbled start of a Winboard interface
stolen from Jim Ablett's work on Vchess.

"Il pense à sa punition méritée."
-- dcorbit
*/
MOVE            getmove(int side, MOVE * moveBuf)
{


    MOVE            pmove = {0};

    int             moveind;
    int             movecnt;

    int             from,
                    dest,
                    i;
    char            cmd[256];
    char            inp[256];
    moveind = -1;

    while (moveind == -1) {
        if (!xboard) {
            puts("fc> ");
        } else {
            printf("\n");
        }
        if (!fgets(inp, sizeof inp, stdin))
            return pmove;
        if (inp[0] == '\n')
            continue;
        sscanf(inp, "%s", cmd);
        if (!strcmp(cmd, "go")) {
            pmove.type = MOVE_TYPE_NONE;
            pmove.from = -2;
            return (pmove);
            continue;
        } else if (!strcmp(cmd, "self")) {
            pmove.type = MOVE_TYPE_NONE;
            pmove.from = -3;
            return (pmove);
            continue;
        } else if (!strcmp(cmd, "back")) {
            pmove.type = MOVE_TYPE_NONE;
            pmove.from = -4;
            return (pmove);
            continue;
        } else if (!strcmp(cmd, "undo")) {
            pmove.type = MOVE_TYPE_NONE;
            pmove.from = -4;
            return (pmove);
            continue;
        } else if (!strcmp(cmd, "black")) {
            computer_side = BLACK;
            selfplay = false;
            nocomp = false;
            no_go = true;
            continue;
        } else if (!strcmp(cmd, "white")) {
            computer_side = WHITE;
            selfplay = false;
            nocomp = false;
            no_go = false;
            continue;
        } else if (!strcmp(cmd, "new")) {
            computer_side = BLACK;
            selfplay = false;
            nocomp = false;
            initboard();
            playing = true;
            player = WHITE;
            continue;
        } else if (!strcmp(cmd, "protover")) {
            printf("feature colors=1 myname=\"FirstChess Xboard 1.0\"");
            printf(" time=1 pause=0 ping=1 sigint=0 sigterm=0");
            printf(" name=1 reuse=0 done=1\n");
            continue;
        } else if (!strcmp(cmd, "force")) {
            nocomp = true;
            continue;
        } else if (!strcmp(cmd, "quit")) {
            exit(EXIT_SUCCESS);
        } else if (!strcmp(cmd, "easy")) {
            printf("easy (ponder off)\n");
            continue;
        } else if (!strcmp(cmd, "hard")) {
            printf("hard (ponder not implemented)\n");
            continue;
        } else if (!strcmp(cmd, "ping")) {
            int             ping;
            sscanf(inp, "%*s %d", &ping);
            printf("pong %d\n", ping);
            continue;
        } else if (!strcmp(cmd, "st")) {
            scanf(inp, "st %d", &maxtime);
            maxtime = (maxtime * 1000);
            max_depth = 32;
            continue;
        } else if (!strcmp(cmd, "time")) {
            sscanf(inp, "time %d", &maxtime);
            maxtime *= 14;
            maxtime /= (TIMEALLOC);
            max_depth = 15;
            continue;
        } else if (!strcmp(cmd, "level")) {
            sscanf(inp, "level %d %d", &max_moves, &maxtime);
            if (max_moves == 1);
            max_moves = (TIMEALLOC / 1000);
            maxtime *= 14;
            maxtime /= (max_moves * 1000);
            max_depth = 15;
            continue;
        } else if (!strcmp(cmd, "sd")) {
            sscanf(inp, "sd %d", &max_depth);
            maxtime = 1 << 25;
            continue;
        } else if (!strcmp(cmd, "otim")) {
            continue;
        } else if (!strcmp(cmd, "result")) {
            char            result[256];
            sscanf(inp, "result %s", result);
            nocomp = true;
            continue;
        } else if (!strcmp(cmd, "xboard")) {
            xboard = true;
            printf("feature done=0\n");
            continue;
        }
        /* user entered a move: */
        from = cmd[0] - 'a';
        from += 8 * (8 - (cmd[1] - '0'));
        dest = cmd[2] - 'a';
        dest += 8 * (8 - (cmd[3] - '0'));
        ply = 0;
        movecnt = Gen(side, moveBuf);
        /* loop through the moves to see if it's legal */
        for (i = 0; i < movecnt; i++) {
            if (moveBuf[i].from == from && moveBuf[i].dest == dest) {
                if (piece[from] == PAWN && (dest < 8 || dest > 55)) {   /* Promotion move? */
                    switch (cmd[4]) {
                    case 'q':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
                        break;

                    case 'r':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_ROOK;
                        break;

                    case 'b':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_BISHOP;
                        break;

                    case 'n':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_KNIGHT;
                        break;

                    default:

                        puts("promoting to a McGuffin..., I'll give you a queen");
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
                    }
                }
                if (!MakeMove(moveBuf[i])) {
                    TakeBack();
                    printf("Illegal move.\n");
                }
                break;

                if (xboard == false && moveind == -1)
                    printf("Error (unknown command): %s\n", cmd);
            }
            return (moveBuf[moveind]);
        } /* For loop over movecount */
    } /* While loop over moveind == -1 */
   assert(false);
   return pmove;
}

/*
   ****************************************************************************
   * Main program                                                             *
   ****************************************************************************
 */
int             main()
{
    char            s[256];
    int             from,
                    dest,
                    i;
    MOVE            moveBuf[200];
    int             movecnt;
    printf("First Chess, by Pham Hong Nguyen\n");
    printf("Help\n d: display board\n MOVE: make a move (e.g. b1c3, a7a8q)\n quit: exit\n\n");
    side = WHITE;
    computer_side = BLACK;      /* Human is white side */
    max_depth = 5;
    hdp = 0;
    initboard();
    for (;;) {
        if (side == computer_side) {    /* computer's turn */
            /* Find out the best move to react the current position */
            MOVE            bestMove = ComputerThink(max_depth);
            MakeMove(bestMove);
            continue;
        }
        /* get user input */
        printf("fc> ");
        if (scanf("%s", s) == EOF)      /* close program */
            return EXIT_FAILURE;
        if (!strcmp(s, "d")) {
            PrintBoard();
            continue;
        }
        if (!strcmp(s, "quit")) {
            printf("Good bye!\n");
            return EXIT_SUCCESS;
        }
        /* maybe the user entered a move? */
        from = s[0] - 'a';
        from += 8 * (8 - (s[1] - '0'));
        dest = s[2] - 'a';
        dest += 8 * (8 - (s[3] - '0'));
        ply = 0;
        movecnt = Gen(side, moveBuf);
        /* loop through the moves to see if it's legal */
        for (i = 0; i < movecnt; i++)
            if (moveBuf[i].from == from && moveBuf[i].dest == dest) {
                if (piece[from] == PAWN && (dest < 8 || dest > 55)) {   /* Promotion move? */
                    switch (s[4]) {
                    case 'q':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
                        break;

                    case 'r':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_ROOK;
                        break;

                    case 'b':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_BISHOP;
                        break;

                    case 'n':
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_KNIGHT;
                        break;

                    default:

                        puts("promoting to a McGuffin..., I'll give you a queen");
                        moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
                    }
                }
                if (!MakeMove(moveBuf[i])) {
                    TakeBack();
                    printf("Illegal move.\n");
                }
                break;

            }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//"stdio.h"
///////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
//defs.h
/////////////////////////////////////////////////////////////////////////////////////

// search.c
extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info);


////////////////////////////////////////////////////////////////////////////////////////
// misc.c
////////////////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "defs.h"

#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#include "sys/select.h"
#include "unistd.h"
#include "string.h"
#endif

int GetTimeMs() {
#ifdef WIN32
  return GetTickCount();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}

/* 
// http://home.arcor.de/dreamlike/chess/
int InputWaiting()
{
#ifndef WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
   static int init = 0, pipe;
   static HANDLE inh;
   DWORD dw;

   if (!init) {
     init = 1;
     inh = GetStdHandle(STD_INPUT_HANDLE);
     pipe = !GetConsoleMode(inh, &dw);
     if (!pipe) {
        SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(inh);
      }
    }
    if (pipe) {
      if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
      return dw;
    } else {
      GetNumberOfConsoleInputEvents(inh, &dw);
      return dw <= 1 ? 0 : dw;
	}
#endif
}

void ReadInput(S_SEARCHINFO *info) {
  int             bytes;
  char            input[256] = "", *endc;

    if (InputWaiting()) {
		info->stopped = TRUE;
		do {
		  bytes=read(fileno(stdin),input,256);
		} while (bytes<0);
		endc = strchr(input,'\n');
		if (endc) *endc=0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4))    {
			  info->quit = TRUE;
			}
		}
		return;
    }
}
*/

////////////////////////////////////////////////////////////////////////////////////////
// search.c
////////////////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "defs.h"


int rootDepth;

static void CheckUp(S_SEARCHINFO *info) 

{
	// .. check if time up, or interrupt from GUI
	if(info->timeset == TRUE && GetTimeMs() > info->stoptime) {
		info->stopped = TRUE;
}

	/* 
ReadInput(info);
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const S_BOARD *pos) {

	int index = 0;

	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	pos->HashTable->overWrite=0;
	pos->HashTable->hit=0;
	pos->HashTable->cut=0;
	pos->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}
*/

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	
	// Проверка информации по узлам
	if(( info->nodes & 2047 ) == 0) 
	{
	CheckUp(info);
	}

	info->nodes++;
/* 
	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	int Score = EvalPosition(pos);

	ASSERT(Score>-INFINITE && Score<INFINITE);

	if(Score >= beta) {
		return beta;
	}

	if(Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
        TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
		}
    }

	ASSERT(alpha >= OldAlpha);

	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	if(depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}
*/
	if(( info->nodes & 2047 ) == 0) 
	{
		CheckUp(info);
	}
/*
	info->nodes++;

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);

	if(InCheck == TRUE) {
		depth++;
	}

	int Score = -INFINITE;
	int PvMove = NOMOVE;

	if( ProbeHashEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE ) {
		pos->HashTable->cut++;
		return Score;
	}

	if( DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
		MakeNullMove(pos);
		Score = -AlphaBeta( -beta, -beta + 1, depth-4, pos, info, FALSE);
		TakeNullMove(pos);
*/
		if(info->stopped == TRUE) 
		{
			return 0;
		}
/*
		if (Score >= beta && abs(Score) < ISMATE) {
			info->nullCut++;
			return beta;
		}
	}

	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INFINITE;

	Score = -INFINITE;

	if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -AlphaBeta( -beta, -alpha, depth-1, pos, info, TRUE);
		TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}
		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			if(Score > alpha) {
				if(Score >= beta) {
					if(Legal==1) {
						info->fhf++;
					}
					info->fh++;

					if(!(list->moves[MoveNum].move & MFLAGCAP)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}

					StoreHashEntry(pos, BestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = Score;

				if(!(list->moves[MoveNum].move & MFLAGCAP)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				}
			}
		}
    }

	if(Legal == 0) {
		if(InCheck) {
			return -INFINITE + pos->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=OldAlpha);

	if(alpha != OldAlpha) {
		StoreHashEntry(pos, BestMove, BestScore, HFEXACT, depth);
	} else {
		StoreHashEntry(pos, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) {

	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos,info);

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
							// alpha	 beta
		rootDepth = currentDepth;
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);

		if(info->stopped == TRUE) {
			break;
		}

		pvMoves = GetPvLine(currentDepth, pos);
		bestMove = pos->PvArray[0];
		if(info->GAME_MODE == UCIMODE) {
			printf("info score cp %d depth %d nodes %ld time %d ",
				bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
		} else if(info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE) {
			printf("%d %d %d %ld ",
				currentDepth,bestScore,(GetTimeMs()-info->starttime)/10,info->nodes);
		} else if(info->POST_THINKING == TRUE) {
			printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
				bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
		}
		if(info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {
			pvMoves = GetPvLine(currentDepth, pos);
			if(!info->GAME_MODE == XBOARDMODE) {
				printf("pv");
			}
			for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s",PrMove(pos->PvArray[pvNum]));
			}
			printf("\n");
		}

		//printf("Hits:%d Overwrite:%d NewWrite:%d Cut:%d\nOrdering %.2f NullCut:%d\n",pos->HashTable->hit,pos->HashTable->overWrite,pos->HashTable->newWrite,pos->HashTable->cut,
		//(info->fhf/info->fh)*100,info->nullCut);
	}

	if(info->GAME_MODE == UCIMODE) {
		printf("bestmove %s\n",PrMove(bestMove));
	} else if(info->GAME_MODE == XBOARDMODE) {
		printf("move %s\n",PrMove(bestMove));
		MakeMove(pos, bestMove);
	} else {
		printf("\n\n***!! Vice makes move %s !!***\n\n",PrMove(bestMove));
		MakeMove(pos, bestMove);
		PrintBoard(pos);
	}

}

	
	*/ 
	
//////////////////////////////////////////////////////////////////////////////////////////


