
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "polyph.h"

/* general parameters */

int verbosity = 0;
int piplength=50;

/* first-pass streamer parameters - 6 distributions */

int max_voices = 5;                /* The maximum number of black squares in a segment; also the maximum number of squares in an analysis. */
int max_grey = 6;                  /* The maximum number of grey squares in a segment */
int max_collisions = 1;            /* Permit collisions only in analyses with this number of voices or fewer. (If 1, disallow all collisions.) */

double fp_note_prob = 0.4;                /* The prob of a note-onset in a square */
double fp_note_cont = 0.8;
double fp_last_note_cont = 0.5;      /* The P that the last note of a stream will continue into the next square */
double fp_stream_cont = 0.95;      /* The P that a stream will continue into the next square */
double fp_stream_prob = .0001;      /* The P of starting a new stream */
double firstseg_stream_prob = 2.0;

/* metharm parameters - 44 distributions */

int tactus_min=7;
int tactus_max=24;
                      /*   1   2   3   4   5   6   7     8    9   10   11   12   13   14   15   16   17   18   19   20   21   22    23      24   */
double init_tactus[24] = {.0, .0, .0, .0, .0, .0, .001, .05, .08, .08, .08, .08, .08, .08, .08, .08, .08, .08, .07, .05, .02, .008, .0009, .0001};
                      /*   8 through 21 is .99, 7 through 22 is .999 */
//double init_tactus[24] = {.0, .0, .0, .0, .0, .0, .001, .05, .14, .14, .14, .12, .10, .08, .06, .05, .04, .03, .02, .01, .01, .008, .0009, .0001};

double another = 0.95;
double lower_duple_score = 0.8;
//double lower_duple_score = 1.0;

/*                             un          pre    post   both   */
double L0_anchor_score[2][2]={{0.00000001, 0.1}, {0.001, 0.1}};

/*                                   un        post   pre   both */
double L1_duple_anchor_score[2][2]={{0.000001, 0.2}, {0.01, 0.4}};

/*                              un-anchored:   00   01        10      11 post-anchored:  00    01     10      11 */
double L1_triple_anchor_score[2][2][2][2]=
//                                         {{ {{1.0, 0.0001}, {0.0001, 0.0001}},        {{0.5, 0.45}, {0.0001, 0.05}} },
                                           {{ {{1.0, 0.000001}, {0.000001, 0.000001}},        {{0.5, 0.45}, {0.0001, 0.05}} },
/*                              pre-anchored:  00    01      10    11    both-anchored:  00   01     10    11 */
//                                         { {{0.999, 0.001}, {0.005, 0.0001}},             {{0.2, 0.6}, {0.02, 0.18}} }};
                                           { {{0.999, 0.0001}, {0.005, 0.0001}},             {{0.2, 0.6}, {0.02, 0.18}} }};

double nonbeat_note_score = .00000000001;
//double nonbeat_note_score = 0.0;

double tactus_reg_score[5] = {0.32, 0.29, 0.05, 0.00001, 0.0000000001}; 
//double tactus_reg_score[5] = {0.32, 0.29, 0.05, 0.00001, 0.0}; 

double lower_reg_score[3] = {0.8, 0.1, 0.0001};    /* lower_reg_score[x] = P of diff of x between intervals. Used for both L0 and L1. */
//double lower_reg_score[3] = {0.6, 0.2, 0.0001};    /* lower_reg_score[x] = P of diff of x between intervals. Used for both L0 and L1. */
double init_phase_score[5] = {0.6, 0.2, 0.1, 0.0001, 0.0999};

//proximity_profile is set with a normal distribution 
double prox_var = 10.0;
double harmony_profile[12] = {0.24, 0.02, 0.04, 0.16, 0.18, 0.04, 0.02, 0.18, 0.02, 0.04, 0.04, 0.02};  /* .24 + (2 * .18) + .16 + (4 * .04) + (4 * .02) */
//double harmony_profile[12] = {0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08};

double raw_tactus_note_score = 0.6;
double L2_note_score = 0.7;
//double L3_note_score = 0.9;
double L3_note_score = 0.7;

double raw_harm_change = 0.6;
double L2_harm_change = 0.5;
double L3_harm_change = 0.7;
double fifths_move = 0.5;     /* The P that, if the harmony changes, it's a move by fifths (up or down - so P(desc. fifth) would be fifths_move / 2 ) */
//double fifths_move = 0.1818;   
double anch_penalty[12] = {1.0, .0001, .0001, 1.0, 1.0, .0001, .0001, 1.0, .0001, .0001, .1, .0001};
double cont = 0.5;
double nct_cont = .01;
double unanch_nct_cont = .0001;

double stream_prob = 0.1;
double stream_cont = 0.95;
