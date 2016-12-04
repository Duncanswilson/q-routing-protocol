
/* Includes. */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "./standalone.h"
#include "./learn.h"
#include "./simsetup.h"
#include "./action.h"
#include "./policy.h"
#include "./runsimulator.h"
#include "./report.h"

/* Main. */
void
main(int argc,char *argv[])
{
  get_globals(argc, argv);

  randomize();		/* Set it up. */
  init_graph(graphname);	/* Setup graph parameters. */

  if ((strat == BEST)||(strat == BESTATIC)||(strat == PRIOR))
    compute_best();		/* Initialize shortest paths. */

  init_Q();			/* Setup learning parameters. */

  do_sim_setup();

  do_sim();
}
