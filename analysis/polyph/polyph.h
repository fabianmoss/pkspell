
/* general parameters */

int verbosity;
int piplength;
double final_score;

#define MAXP 128                  /* The high end of the pitch range (middle C = 60) (actually the highest possible pitch is MAXP - 1) */

/* streamer structures */

#define MAXV 10                   /* The maximum number of squares in an analysis - EVER. This is used to 
                                     define the arrays. */
#define MA 1000                   /* The maximum number of analyses for a segment */
#define MT 256                    /* The maximum number of transitions for an analysis */
#define MC 4                      /* The maximum number of voices in an analysis in which collisions are allowed 
				   (if the analysis has MC or fewer voices, allow collisions) */
#define MS 10000                   /* The maximum number of segments allowed. (WARNING: Setting this 
				     number greater than 1000 may exceed the maximum array
				     size allowed on some UNIX machines. */
int max_voices;
int max_grey;
int max_collisions;
double fp_note_prob;              
double fp_last_note_cont;
double fp_note_cont;
double fp_stream_cont;            
double fp_stream_prob;
double firstseg_stream_prob;

char noteword[10];
int display_command;
int numnotes, numbeats;
int total_duration;
int final_timepoint;
int segment_beat_level;
double seglength;

struct note_struct {
  int ontime;
  int offtime;
  int duration;
  int pitch;
  int segment;
  int voice_number;
  int stream;
  int valid;
  int done;
};

struct note_struct znote[10000];     /* Notes as input - not necessarily chronological */

struct note_struct note[10000];      /* Notes sorted chronologically */

/* In what follows, a black square in a segment is a note onset; a blue square is a note continuation; a grey square is one that could be a rest in
   a stream */

struct {                  
  int start;
  int end;
  int duration;
  int inote[20];
  int snote[20];
  int numnotes;        /* number of black, blue, or grey squares in the segment */
  int numdark;         /* number of black or blue squares */
  int numblue; 
  int column[MAXP];
  int numanal;
  int analysis[MA][MAXV];
  int analcard[MA];
  int analblack[MA];
  int analblue[MA];
  int analgrey[MA];
  double analscore[MA];
  int voice_number[MAXV];
} segment[MS];        /* An array storing the notes in each segment. */
int segtotal;              /* total number of segments - 1 */

int canceled[MAXP];    /* This value is 1 for a pitch if another pitch a half-step or whole-step away has occurred more
			   recently than the pitch itself */

int prov_analysis[MAXV+1];

/* The arrays below are only kept for a given pair of segments. Then the data is just overwritten for the next pair of
   segments. */

int prov_trans[MAXV+1];
int ltransition[MA][MT][MAXV];            /* All the transitions for the left side of an analysis pair. 
					      [analysis number][transition number][pitches of transition] */

int ltranscard[MA][MT];                 /* The cardinality of each transition */

int rtransition[MA][MT][MAXV];
int rtranscard[MA][MT];

/* The arrays below keep data for all segments (the entire piece) */

int best_lt, best_rt;
double best_transition_score;
int best_ltransition[MAXV];
int best_rtransition[MAXV];
int best_transcard;

int seg, t, a;
int ltnum[MA], rtnum[MA];                      /* The number of transitions for each analysis */

int best[MS][MA];                              /* The best prior analysis for each analysis */
double global_analysis[MS][MA];
int final[MS];
int final_ltransition[MS][MAXV];
int final_rtransition[MS][MAXV];
int final_transcard[MS];

/* metharm structures */

struct Snote {

    int pitch;
    int ontime;
    int offtime;
    int stream;
    double hp_score[12];
    struct Snote * next;
    struct Snote * prev;
};

struct Stream {
    int index;
    struct Snote * sn;     /* The first note in the stream */
    struct Stream * next;
    int start, end;        /* Ontimes of first and last notes; set for complete streams only */
    struct Stream * cs; /* for partial streams, a pointer to the complete stream that this is part of */
    int t1, t2;             /* temporary variables indicating note status of start and end of beat interval during
			   search */
};

struct Pip {
    struct Stream * stream;
    int hasnote;
    int next_tactus;
    int besth;
    int ph;
    int final_beatlevel;
    int row[MAXP];
};

struct Subtactus {
    int bestc;
    int valid;
    double rscore;
    double pscore[12];
};

struct Subtactus ** subt;

struct Tactus {
    double local_score[2][12];
    int valid;
    int numnotes;
    int validh[12];
    int bestk[2][12];
    int besth2[2][12];
    int bestc[12];
    int bestc1[12];
    int bestc2[12];
    double score[2][12];
};

struct Tactus ** tactus;

int tp[100000];             /* list of tactus pips: tp[0] = first tactus pip, etc. */
int num_tactus_beats;

struct {
    double score[5][12];
    int besth2[5][12];
    int besth;
} upper[1000];

struct Stream * streamlist;

struct Pip * pip;

int num_pips;
int last_offtime;

/* metharm parameters and globals */

double tactus_profile[40][40];
double init_tactus[24]; /* 7 through 24 */
int tactus_min;
int tactus_max;
double another;
double lower_duple_score;

double L0_anchor_score[2][2];
double L1_duple_anchor_score[2][2];
double L1_triple_anchor_score[2][2][2][2];
double nonbeat_note_score;

double tactus_reg_score[5];    
double lower_reg_score[3];    
double init_phase_score[5];

double proximity_profile[MAXP];
double prox_var;
double harmony_profile[12];

double adjusted_profile[MAXP];

double raw_tactus_note_score;
double raw_harm_change;
double L2_note_score;
double L3_note_score;
double L2_harm_change;
double L3_harm_change;
double fifths_move;     
double anch_penalty[12];
double cont, nct_cont, unanch_nct_cont; 

double stream_prob;
double stream_cont;

int first_tactus, last_tactus;
int bestd, best_ph;

void adjust_notes();
void create_segments();
void create_streams();
void create_streamlists();
void make_profiles();
void create_pipstreams();
void create_subspans();
void create_tactus_spans();
void prune_tactus_spans();
void make_tables();
void traceback();
void find_upper_level();
void final_meter();
void stream_probs();
void graphic_display();
void meter_stats();
void print_chords();
void print_notes_and_beats();

