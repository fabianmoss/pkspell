
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "polyph.h"

int main(int argc, char *argv[]) {

    int n, j;
    struct Snote * sn;
    struct Stream * s;

    char *parameter_file = NULL, *input_file = NULL;
    FILE *in_file;
    char line[1000];
    char noteword[10];
    char junk[10];

    int param_file_specified = 0;

    verbosity=1;   /* set verbosity */
    
    for (j=1; j<argc; j++) {
	if (strcmp(argv[j], "-p") == 0) {
	    parameter_file = argv[j+1];
	    param_file_specified = 1;
	    j++;
	}
	else if (strcmp(argv[j], "-v") == 0) {
	    j++;
	    sscanf(argv[j], "%d", &verbosity);
	}
	else if (input_file == NULL) {
	    /* assume it's a file */
	    input_file = argv[j];
	}
    }
    
    //read_parameter_file (parameter_file, param_file_specified);
    
    if (input_file != NULL) {
	in_file = fopen(input_file, "r");
	if (in_file == NULL) {
	    printf("I can't open that file\n");
	    exit(1);
	}
    } else {
      in_file = stdin;
    }

    if(tactus_min < 6) {
	printf("Error: tactus_min must be at least 6\n");
	exit(1);
    }

    n=0;
    while (fgets(line, sizeof(line), in_file) !=NULL) {            /* read in Notes */
	if(line[0] == '\n' || line[0] == '%') continue;
	sscanf(line, "%s", noteword);
	if(strcmp(noteword, "Note") != 0) {
	    //printf("Bad line found in input: %s", line);
	    continue;
	}
	if(sscanf (line, "%s %d %d %d %s", noteword, &znote[n].ontime, &znote[n].offtime, &znote[n].pitch, junk) != 4) {
	    //printf("Bad line found in input: %s", line);
	    continue;
	}
	sscanf (line, "%s %d %d %d", noteword, &znote[n].ontime, &znote[n].offtime, &znote[n].pitch);
	//printf("Note %d %d %d\n", znote[n].ontime, znote[n].offtime, znote[n].pitch); 
	n++;
    }
    numnotes = n;

    if(verbosity>=0) printf("Creating segments and streams...\n");

    adjust_notes();

    create_segments();

    create_streams();

    //display_streams();

    /*
    printf("Notes as output by streamer:\n");
    for(n=0; n<numnotes; n++) {
	printf("Note %d %d %d %d\n", note[n].ontime, note[n].offtime, note[n].pitch, note[n].stream);
	}  */

    create_streamlists();

    //print_streams(streamlist, -1, -1, 1);

    make_profiles();

    create_pipstreams();

    if(verbosity>=0) printf("Creating spans...\n");

    create_subspans();

    create_tactus_spans();

    prune_tactus_spans();

    if(verbosity>=0) printf("Doing tactus search...\n");

    make_tables();

    traceback();

    if(verbosity>=0) printf("Finishing up...\n");

    find_upper_level();

    final_meter();

    stream_probs();

    if(verbosity>0) graphic_display();

    if(verbosity==-1) meter_stats();

    if(verbosity==-2) print_chords();

    if(verbosity==-3) {
	print_notes_and_beats(1);    // notes, beats, and stream numbers
	print_chords();
    }

    // printf("Time = %.3f\n", ((double) clock())/CLOCKS_PER_SEC);
}
