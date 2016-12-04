
/* Compute single source shortest paths. */
void compute_best()
{
  int changing = 1;
  int i, j, k;

  /* Initialize distances. */
  for (i = 0; i < nnodes; i++)
    for (j = 0; j < nnodes; j++) {
      if (i == j)	distance[i][j] = 0;
      else		distance[i][j] = nnodes+1;
      shortest[i][j] = -1;
    }

  /* Keep updating until no more changes. */
  while (changing) {
    changing = 0;
    for (i = 0; i < nnodes; i++) {
      for (j = 0; j < nnodes; j++) {
	/* Update our estimate of distance for sending from i to j. */
	if (i != j) {		/* Not self. */
	  for (k = 0; k < nlinks[i]; k++) {
	    if (distance[i][j] > 1+distance[links[i][k]][j]) {
	      distance[i][j] = 1+distance[links[i][k]][j];	/* Better. */
	      shortest[i][j] = k;
	      changing = 1;
	    }
	  }	/* Next link. */
	}
      }	/* Next "to" */
    }	/* Next "from" */
  }	/* Next iteration. */

  /* Compute avg. distance. */
#ifdef DEBUG
  k = 0;
  for (i = 0; i < nnodes; i++)
    for (j = 0; j < nnodes; j++)
      k += distance[i][j];
  fprintf(stderr, "Avg internode distance: %f\n",
	  DOUBLE(k)/DOUBLE(nnodes*nnodes));
#endif
  fprintf(stderr, "Done computing best.\n");
}



/* Compute an outgoing link on which to send n's topmost packet in */
 /* order to route it to dest.
    This is where we do the epsilon greedy Q updating
  */
int choose_link(int n, int dest)
{
  switch (strat) {
  case PRIOR:
  case LEARN:	/* True learning. */
  {
    double best = estimate_Q(n, dest);
    int i, best_count = 0, best_link;

    for (i = 0; i < nlinks[n]; i++) {
if (Q[n][i][dest] <= best+epsilon) {
  best_count++;
  if (one_in(best_count)) best_link = i;
}
    }
    return(best_link);
  }
      // int i, best_count = 0, best_link;
      // int best_arr[nlinks[n]];
      // int best_not_found = 1;
      // time_t t;
      // srand((unsigned) time(&t));
      //
      //
      // for (i = 0; i < nlinks[n]; i++) {
	    //    if (Q[n][i][dest] <= best+epsilon)
	    //       best_arr[i] = 1;
      //       best_count++;
      //   }
      //
      // if (one_in(1 - epsilon) || best_count == 0)
      //     best_link = best;
      //
      // else{
      //   while(best_not_found){
      //     if(best_arr[rand()%nlinks[n]]){
      //       best_not_found = 0;
      //       best_link = rand()%nlinks[n];
      //     }
      //   }
      // }
  case RANDOM:	/* Just go. */
    return(create(nlinks[n]));
  case BEST:	/* Always take one of the shortest paths (at random). */
    {
      int best = distance[n][dest];
      int i, best_count = 0, best_link;

      for (i = 0; i < nlinks[n]; i++) {
	if (1+distance[links[n][i]][dest] == best) {
	  best_count++;
	  if (one_in(best_count)) best_link = i;
	}
      }
      return(best_link);
    }
  case BESTATIC:
    return(shortest[n][dest]);
  default:
    fprintf(stderr, "Unknown action strategy: %d\n", strat);
  }
}
