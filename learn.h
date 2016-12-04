

/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/
/*Set up/ running the learning stuff!!!
/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
#include "./netsim1.h"

/* Setup learning parameters. */
void init_Q()
{
  int i, j, k;

  if (strat == PRIOR) {
    /* Initialize estimates. */
    for (i = 0; i < nnodes; i++) {
      for (j = 0; j < nlinks[i]; j++) {
	for (k = 0; k < nnodes; k++) {
	  Q[i][j][k] = DOUBLE(4*distance[links[i][j]][k]);
	}
      }
    }
  }
  else {
    /* Initialize estimates. */
    for (i = 0; i < nnodes; i++) {
      for (j = 0; j < nlinks[i]; j++) {
	for (k = 0; k < nnodes; k++) {
	  Q[i][j][k] = 0.0;
	  /*	Q[i][j][k] = DOUBLE(2*nnodes);  /* Initialize high. */
	}
      }
    }
  }
}

/* How long from node to dest? */
double estimate_Q(int n_from, int n_to)
{
    double best = Q[n_from][0][n_to];
    double current;
    int i;

    for (i = 1; i < nlinks[n_from]; i++) {
      current = Q[n_from][i][n_to];
      if (current > best) best = current;
    }

    return(best);
}


void
do_learn(n, link, dest, new_est)
     int n, link, dest;
     double new_est;
{
  double *cell = &Q[n][link][dest];	/* Cell in Q to update. */
  double out = (*cell);

  //printf("wtf? newest:  %d, out: %d, eta: %f,  updateval: %f \n", new_est, out, eta, (new_est-out)*eta);
  *cell += (new_est-out)*0.7;  /* Move a fraction to new_est. */
}


/* Lets a node process one packet.  Chooses a link, sends the packet, */
/* uses the revised estimate to adjust routing scheme. */
int
bump_packet_Q(e)
     int e;
{
  int n = events[e].node;
  int dest = events[e].dest;
  int link, i;
  double new_est;

  if (learntype == VL) {
    /* For every possible link, get a better estimate. */
    for (i = 0; i < nlinks[n]; i++) {
      new_est = pseudosend(e, n, i);
      if ((vprob == 1.0)||(withprob(vprob)))
	       do_learn(n, i, dest, new_est);
    }
  }

  /* We need to pick an outgoing link. */
  link = choose_link(n, dest);

  /* We also ask the new node for an estimate of the arrival time. */
  new_est = send(e, n, link);

  /* Now, independent of whether the send worked, we can update the */
  /* Q value for that node, link, destination triple by making it */
  /* closer to the new estimate. */
  do_learn(n, link, dest, new_est);

  return(1);
}


/* Lets a node process one packet.  Chooses a link, sends the packet, */
/* uses the revised estimate to adjust routing scheme. */
void bump_packet_sarsa(int e)
{
  int last_node = events[e].node;
  int dest = events[e].dest;
  int i, j, next_node, old_link, new_link;
  double new_est;

  old_link = events[e].current_link;
  if (learntype == VL || learntype == SARSA) {
    /* For every possible link, get a better estimate. */
    for (i = 0; i < nlinks[last_node]; i++) {
      new_est = pseudosend(e, last_node, i);
      if ((vprob == 1.0)||(withprob(vprob)))
	     do_learn(last_node, i, dest, new_est);

       for (j = 0; j < nlinks[links[last_node][i]]; j++) {
         new_est = pseudosend(e, last_node, j);

         if ((vprob == 1.0)||(withprob(vprob)))
         do_learn(last_node, j, dest, new_est);
       }
    }
  }
  /* Take action A and get back a weird
  reward/value-to-the-end-from-the-state-action-a-took-us-to estimate back */
  //printf("this is last_node: %d\n",last_node );
  new_est = send(e, last_node, old_link);

  next_node = events[e].node;

  //printf("this is next_node and  links[last_node][link]:%d,  %d\n",next_node, links[last_node][old_link]);
  new_link = choose_link(next_node, dest);

  new_est += Q[next_node][new_link][dest];


  if (new_link != old_link)
    do_learn_sarsa(last_node, dest, old_link, next_node, new_link, new_est);

  events[e].current_link = new_link;
}

  /* Now, independent of whether the send worked, we can update the */
  /* Q value for that node, link, destination triple by making it */
  /* closer to the new estimate. */
void do_learn_sarsa(int last_node, int dest, int old_link, int next_node, int next_link,  double new_est)
  {

    Q[last_node][old_link][dest]  += eta*(new_est + Q[next_node][next_link][dest]
                                                - Q[last_node][old_link][dest]);

  }



  /*
  That's because in real Q learning it would look like this:
  init Q:
  init s_0

  Loop:
  -  be in state s_t and use epsilon greedy evaluation to get action a_t

  -  take action a_t and get r_t and get/update s_t+1 (or however you want to index them)

  -  update Q(s_t, a_t) = Q(s_t,a_t) + alpha*[r_t + gamma*(max over all a_t+1 of: {Q(s_t+1,a_t+1)}) - Q(s_t, a_t)]



  Instead we do this:
  init Q
  init s_0

  loop:
  - be in state (events[e].node, events[e].dest)  (s_t is a tuple of ints in our case)

  - a_t = choose_link(s_t) (this uses epsilon greedy evalution to get a_t)

  - take action a_t and update the state to s_t+1 ( or in code we update events[e].node = links[events[e].node][link])
        and get new_est which is currently a "negative reward":
            [r:=(queuing time in state s_t(q) and transmission delay to state s_t+1 (s)]
        plus "value estimate from the next state" or technically
            (min over all a_t+1 Q(s_t+1, a_t+1) reffered to very stupidly as t in the paper) )
        [so in code  t:= estimate_Q(s_t+1)]


  - then lastly we call do_learn which updates the Q matrix as follows:

        Q(s_t, a_t) = Q(s_t,a_t) + alpha*[(q + s + t) - Q(s_t, a_t) ]

  or in code:
  if "s_t" was (events[e].node,events[e].dest) at the beginning of the episode then to do the q update we define some variables

  last_node = events[e].node (at the beginning of the episode)
  link = choose_link(events[e].node, events[e].dest)
  next_node = events[e].node (after we call send())

        Q[events[e].node][link][events[e].dest] += alpha[ [(events[e].etime - events[e].qtime-internode)
                                                      + (internode) + estimate_Q(links[events[e].node][link], events[e].dest)
                                                    ]
                                                    - Q[events[e].node][link][events[e].dest]
                                                  ]


  So if we want to convert this to SARSA
  init Q
  init s_0
  - a_0 = choose_link(s_0) (this uses epsilon greedy evalution to get a_0)

  loop:
  - take action a_t and update the state to s_t+1 ( or in code we update events[e].node = links[events[e].node][link])
        and get new_est which is currently a "negative reward":
            [r:=(queuing time in state s_t(q) and transmission delay to state s_t+1 (s)]
        plus "value estimate from the next state" or technically
            (min over all a_t+1 Q(s_t+1, a_t+1) reffered to very stupidly as t in the paper) )
        [so in code  t:= estimate_Q(s_t+1)]

  - a_t+1 = choose_link(s_t+1) (this uses epsilon greedy evalution to get a_t)


  - then we call do_learn which updates the Q matrix as follows:

        Q(s_t, a_t) = Q(s_t,a_t) + alpha*[(q + s) + Q(s_t+1, a_t+1) - Q(s_t, a_t) ]

  or in code:

  last_node = events[e].node (at the beginning of the episode)
  old_link = choose_link(events[e].node, events[e].dest)
  next_node = events[e].node (after we call send())
  new_link = choose_link(events[e].node, events[e].dest) (after we've called send that is)

  *cell  += alpha*(((events[e].etime - events[e].qtime-internode)
                                                + (internode) + Q[current_node][new_link][dest_node]
                                              )
                                              - Q[last_node][old_link][dest_node]
                                            )





  lastly we have to update the action a_t = a+t+1

  */
