/* Sends node n's top of queue along given link.  If the destination */
 /* node has a full queue, we send the node's packet to itself. */
 /* Returns an estimate of the time to destination.  Packet must get */
 /* consumed if sent to its destination.  Returns a success or failure */
 /* signal as well. */
double send(int e, int n, int link)
{
  double curtime = events[e].etime;	/* What time is it? */
					/* How long waiting? */
  double time_in_queue = curtime-events[e].qtime-internode;
  double nexttime;			/* Next arrival time. */
  int n_to;
  double new_est;

  /* Figure out neighbor. */
  if ((link < 0) || (link >= nlinks[n]))	/* No such link! */
    n_to = n;
  else
    n_to = links[n][link];			/* Neighbor node. */

  if (down[n][n_to]) n_to = n;		/* Link is down! */

  /* Record activity. */
  link_activity[n][n_to]++;

  if (n_to == events[e].dest) {		/* Destination matches neighbor! */

    routed_packets++;
    nenqueued[n]--;
    total_routing_time += curtime-events[e].birth+internode;
    total_hops += events[e].hops + 1;

    /* It has waited and is now done. */
    new_est = time_in_queue+internode;
    active_packets--;
    free_event(e);
    return(new_est);
  }

  /* Is sending node full?  If so, change destination to be the node */
  /* itself. */
  if (nenqueued[n_to] >= queuelimit) {
    send_fail++;
    n_to = n;
  }

  /* Do the send! */
  events[e].node = n_to;
  events[e].hops++;
  /*  nexttime = Max(enqueued[n_to], curtime+internode) + interqueue; */
  /* Is the queue empty (and recouped) when the packet arrives? */
  /* Which comes last?  Queue is ready or packet arrives? */
  nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode);
  events[e].etime = nexttime;
  enqueued[n_to] = nexttime;
  push_event(e, curtime);
  nenqueued[n_to]++;	/* Keep counts. */
  nenqueued[n]--;

  /* Ok, the packet has moved one link. */
  /* It took the wait + the send + however long it will take from there. */
  //printf("Estimate Q at node: %d, %f  \n", n_to, estimate_Q(n_to, events[e].dest));
  if(learntype == SARSA)
    new_est = time_in_queue+internode;
  else
    new_est = time_in_queue+internode+estimate_Q(n_to, events[e].dest);

  necho_packets++;
  return(new_est);
}

/* Pretends to send node n's top of queue along given link.   */
 /* Returns an estimate of the time to destination.  Returns a success or failure */
 /* signal as well. */
double pseudosend(int e, int n, int link)
{
  double curtime = events[e].etime;	/* What time is it? */
					/* How long waiting? */
  double time_in_queue = curtime-events[e].qtime-internode;
  int n_to;
  double new_est;

  /* Figure out neighbor. */
  if ((link < 0) || (link >= nlinks[n]))	/* No such link! */
    n_to = n;
  else
    n_to = links[n][link];			/* Neighbor node. */

  if (down[n][n_to]) n_to = n;		/* Link is down! */

  if (n_to == events[e].dest) {		/* Destination matches neighbor! */

    /* It has waited and is now done. */
    new_est = time_in_queue+internode;
    return(new_est);
  }

  /* Is sending node full?  If so, change destination to be the node */
  /* itself.  */
  if (nenqueued[n_to] >= queuelimit) {
    n_to = n;
  }

  /* Ok, the packet pretended to move one link. */
  /* It took the wait + the send + however long it will take from there. */
  if(learntype == SARSA)
    new_est = time_in_queue+internode;
  else
    new_est = time_in_queue+internode+estimate_Q(n_to, events[e].dest);

  necho_packets++;
  return(new_est);
}
