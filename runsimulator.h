/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/
/*Actually run the simulator. (do_sim, bump_packet (next state), choose_link, etc..)
/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/

void do_sim() //double timelimit)
{
  int e;	/* Event to handle. */
  double curtime;
  int i;

  while(1) {	/* Until time runs out... */
    /* Epoch. */
    e = pop_event();
    curtime = events[e].etime;

    if (events[e].source == INJECT) {
      events[e].etime += poisson(callmean);

      push_event(e, curtime);
      e = start_packet(curtime);	/* Nil on failure. */
    }
    if ((e != Nil) && (events[e].source == REPORT)) {

      /* Recursively descend heap? */
      interactive_report(curtime, e);

#ifdef OLDREPORT

      printf("%6d %3d %4d %4d %4d %5.5f %6d\n",
	     time, npackets, total_starts, failed_starts, routed_packets,
	     DOUBLE(total_routing_time)/DOUBLE(routed_packets), max_age);
      routed_packets = 0;	/* Reset. */
      total_starts = 0;
      failed_starts = 0;
      max_age = 0;
      total_routing_time = 0;
#endif
    }
    else if (e != Nil){
      if(learntype == SARSA) bump_packet_sarsa(e);

      else
        bump_packet_Q(e);
    }
  }
}


/* Starts another packet going. */
int start_packet(double time)
{
  int n_from, n_to;		/* From and to are uniform over nodes. */
  int e;

  if (active_packets >= maxpackets) return(Nil);

  n_to = create(nnodes); //this is where the randomness comes in

  /* Find a node (different from starting loc). */
  for (n_from = create(nnodes); n_from==n_to; n_from = create(nnodes));

  /* Reject packet, leaving one space for the system (if full). */
  if (nenqueued[n_from] >= queuelimit-1) {
    queue_full++;
    return(Nil);
  }
  else nenqueued[n_from]++;	/* Plop it onto the queue. */

  active_packets++;

  e = create_event(time, n_to);		/* Make the new packet. */

  if (e == Nil) {
    fprintf(stderr, "Too many events, failing...\n");
    exit(-1);
  }

  /* Now that we know where this packet began, set source. */
  events[e].source = events[e].node = n_from;

  /* for SARSA save first link*/
  events[e].current_link = choose_link(n_from, n_to);

  return(e);
}


/* Makes a new heap out of two heaps. */
int meldheap(int h1, int h2)
{
  int h3;

  if (h1 == Nil) return (h2);
  if (h2 == Nil) return (h1);

  /* So h1 is the smaller one. */
  if (events[h1].etime > events[h2].etime) {	/* Swap. */
    h3 = h1; h1 = h2; h2 = h3;
  }

  /* So branches are equally likely. */
  if (one_in(2)) {
    h3 = events[h1].left; events[h1].left = events[h1].right;
    events[h1].right = h3;
  }

  events[h1].right = meldheap(events[h1].right, h2);

  return(h1);
}
