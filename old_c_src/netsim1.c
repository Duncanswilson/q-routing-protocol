/* netsim1.c: talks to tk/tcl code ML 11/24/93
/* net13.c: let internode vary by node. ML/JB 11/13/93
/* net12.c allows switching between the 2
/* net11.c: going to try V values instead of Q values.  ML/JB 10/93 */
/* net7.c: Now at Brown (I know how to do Poisson now). ML 9/93.
/* net7.c: "Poisson" (nope, we couldn't figure it out). 
/* net5.c: reintroduce queue size limit
/* net4.c: better discrete event simulator (priority queue).
/* net3.c: getting set for CMU experiments.  ML, JB 5/93. */
/* net2.c: better support for epochs.
/* net.c: an attempt at an adaptive switching network. ML 10/91.
 */

/* Includes. */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "./standalone.h"

/* Macros. */
#define DOUBLE(x) ((double)(x))

/* Defines. */
#define MAXNODES	128
#define MAXLINKS	  9
#define MAXEVENTS      5000
#define QUEUELEVELS	 60
#define MAXEDGES (MAXNODES*MAXLINKS)

/* Strategies. */
#define RANDOM 0		/* Pick a link, any link. */
#define LEARN 1			/* Best according to learning. */
#define BEST 2			/* Any (random) shortest path. */
#define BESTATIC 3		/* One (fixed) shortest path. */
#define PRIOR 4			/* Learning starting from BEST. */

#define QL 5
#define VL 6

/* Graph variables. */
int nnodes;				/* How many nodes? */
int links[MAXNODES][MAXLINKS];		/* Who connected to? */
int nlinks[MAXNODES];			/* How many connected to? */
double xnode[MAXNODES],ynode[MAXNODES];	/* Coordinates of nodes. */

/* For tcl stuff. */
int link_activity[MAXNODES][MAXNODES];
int down[MAXNODES][MAXNODES];
int edge_from[MAXEDGES];
int edge_to[MAXEDGES];
int nedges;


/* Event structure. */
struct {
  int dest;		/* Where headed? */
  int source;		/* Where created or special event type. */ 
  int node;		/* Where now? */
  double birth;		/* When originally created? */
  int hops;		/* Count number of requeues. */
  int left;		/* Left heap or free list. */
  int right;		/* Right heap. */
  double qtime;		/* When inserted. */
  double etime;		/* When removed. */
} events[MAXEVENTS];
int queuetop;
int freelist;
double enqueued[MAXNODES];		/* Time of last in line. */
int nenqueued[MAXNODES];		/* How many in queue? */

/* Special events. */
#define INJECT -1
#define REPORT -2
#define END_SIM -3
#define UNKNOWN -4

/* Define. */
#define Nil -1

/* Learning variables. */
double Q[MAXNODES][MAXLINKS][MAXNODES];	/* How long will it take? */

/* Represent shortest path. */
int shortest[MAXNODES][MAXNODES];	/* Returns link. */
int distance[MAXNODES][MAXNODES];	/* Returns steps. */
int interqueuen[MAXNODES];		/* Initialized to interqueue. */

/* User variables. */
int strat = LEARN;			/* How choose actions? */
int learntype = VL;			/* What learning type (V/Q)? */
char *graphname = "lata_network";	/* Where get the network? */
double interreport = 100.0;		/* Time between reports. */
double interqueue = 1.0;		/* Time between handlings. (init) */
double internode = 1.0;			/* Time in transit. */
double epsilon = 0.0;			/* Tolerance in Q for link choice. */
int maxpackets = 1000;			/* Packet limit in network. */
int quit_criteria = 3;			/* How many successful reports */
					/* before quitting? */
int queuelimit = 1000;			/* Max elements per queue. */
int queuelevels = 10;			/* Queue levels to perceptron. */
double eta = 0.7;			/* How fast to learn? */
double vprob = 1.0;			/* prob of random echo packet. */
double callmean = 1.0;			/* When next call? */
double callstd = 0.2;
char *polfile = "";			/* Place to store the policy. */

/* Report variables. */
int routed_packets = 0;
int total_routing_time = 0;
int active_packets = 0;
int converge = 0;
double best_transit = 10000.0;
int queue_full = 0;
int necho_packets = 0;
int total_hops = 0;
int send_fail = 0;

/* Functions. */
void get_globals();		/* Processes cmd line args. */
void init_graph();		/* Setup graph parameters. */
void init_Q();			/* Setup learning parameters. */
void do_sim_setup();		/* Prepare to run the simulation. */
void do_sim();			/* Run the simulation for awhile. */
int start_packet();		/* Starts another packet going. */
int bump_packet();		/* Lets a node process one packet. */
double send();			/* Send node's packet over given link. */
double pseudosend();		/* Pretend to send node's packet over given link. */
void do_learn();		/* Incorporate a new estimate. */
int choose_link();		/* Given a node and dest, choose a link. */
int push_event();		/* Add an event to the queue. */
int pop_event();		/* Remove an entry.  Return packet. */
int create_event();		/* Returns a new event struct. */
void free_event();		/* Add event to free list. */
double estimate();		/* How long from node to dest? */
void compute_best();		/* All shortest paths. */
void interactive_report();	/* Get some stats.  Get set again. */
double poisson();		/* Get next arrival time. */
void dump_dest();		/* Policy for a given dest. */

/* Main. */
void
main(argc, argv)
     int argc;
     char *argv[];
{
  get_globals(argc, argv);

#if 0|COMMENT
  fprintf(stderr, "strat %d\teta %lf\tinterreport %f\tmaxpackets %d\n", 
	  strat, eta, interreport, maxpackets);
  fprintf(stderr,"interqueue %f\tgraphname \"%s\"\n", 
	  interqueue, graphname);
  fprintf(stderr,"quit_criteria %d\tinternode %f\tqueuelimit %d\tetaq %f\n", 
	  quit_criteria, internode, queuelimit, etaq);
  fprintf(stderr,"callmean %f\tcallstd %f\tqueuelevels %d\tepsilon %f\n", 
	  callmean, callstd, queuelevels, epsilon);
  fprintf(stderr,"vprob %f\n", 
	  vprob);
  fprintf(stderr, "learntype %d, polfile \"%s\"\n", learntype, polfile);
#endif

  randomize();		/* Set it up. */

  init_graph(graphname);	/* Setup graph parameters. */

  if ((strat == BEST)||(strat == BESTATIC)||(strat == PRIOR))
    compute_best();		/* Initialize shortest paths. */

  init_Q();			/* Setup learning parameters. */

  do_sim_setup();

  do_sim();
}

/* Given an argc and an argv, process the arguments and set global */
 /* variables.  Some of these only make sense at startup.  Others */
 /* don't make sense at start up. */
void
get_globals(argc, argv)
     int argc;
     char *argv[];
{
  /* Command line params. */
  while (argc > 1) {
    /* Two argument switches. */
    if (argc == 2) {
      fprintf(stderr, "\"%s\" argument needs a value.\n", argv[1]);
      exit(-1);
    }
    if (!strcmp(argv[1], "strat")) {
      if (!strcmp(argv[2], "RANDOM"))
	strat = RANDOM;
      else if (!strcmp(argv[2], "LEARN"))
	strat = LEARN;
      else if (!strcmp(argv[2], "BEST"))
	strat = BEST;
      else if (!strcmp(argv[2], "BESTATIC"))
	strat = BESTATIC;
      else if (!strcmp(argv[2], "PRIOR"))
	strat = PRIOR;
      else {
	fprintf(stderr, "strat \"%s\" not recognized.\n", argv[2]);
	exit(-1);
      }
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "learntype")) {
      if (!strcmp(argv[2], "QL"))
	learntype = QL;
      else if (!strcmp(argv[2], "VL"))
	learntype = VL;
      else {
	fprintf(stderr, "learntype \"%s\" not recognized.\n", argv[2]);
	exit(-1);
      }
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "interreport")) {
      interreport = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "quit_criteria")) {
      quit_criteria = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "queuelimit")) {
      queuelimit = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "maxpackets")) {
      maxpackets = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "graphname")) {
      graphname = argv[2];
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "eta")) {
      eta = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "vprob")) {
      vprob = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "internode")) {
      internode = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "callmean")) {
      callmean = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "callstd")) {
      callstd = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "epsilon")) {
      epsilon = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "queuelevels")) {
      queuelevels = atoi(argv[2]);
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "interqueue")) {
      interqueue = DOUBLE(atof(argv[2]));
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "polfile")) {
      polfile = argv[2];
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "up")) {	/* no sense at startup. */
      int edge, n1, n2;
      edge = atoi(argv[2]);
      n1 = edge_from[edge];
      n2 = edge_to[edge];
      down[n1][n2] = 0;
      down[n2][n1] = 0;
      argc -= 2;
      argv += 2;
    }
    else if (!strcmp(argv[1], "down")) { /* no sense at startup. */
      int edge, n1, n2;
      edge = atoi(argv[2]);
      n1 = edge_from[edge];
      n2 = edge_to[edge];
      down[n1][n2] = 1;
      down[n2][n1] = 1;
      argc -= 2;
      argv += 2;
    }
    else {
      fprintf(stderr, "Switch not recognized: %s\n", argv[1]);
      exit(-1);
    }
  }
}

/* Setup graph parameters.  Abandon netpad. */
void
init_graph(filename)
     char *filename;
{
  char line[300];	/* Line from the file. */
  FILE *graph;
  int link, node, node1, node2;
  int dummy;
  double x, y;

  graph = fopen(filename, "r");	/* Try to open it. */
  if (graph == NULL) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    exit(-1);
  }

  nnodes = 0;
  nedges = 0;
  while (fscanf(graph, "%[^\n]\n", line) != EOF) {
    if(!strncmp(line, "1000 ", 5)) {	/* Node declaration. */
      sscanf(line, "1000 %d %f %f %d", &node, &x, &y, &dummy);
      if (node != nnodes) {
	fprintf(stderr, "bad graph format: got node %d after %d, expected %d.\n",
		node, nnodes-1, nnodes);
	exit(-1);
      }
      /* Check for overflow. */
      if (nnodes == MAXNODES) {
	fprintf(stderr, "MAXNODES too small for this graph.\n");
	exit(-1);
      }
      /* Init new node. */
      xnode[nnodes] = x;
      ynode[nnodes] = y;
      nlinks[nnodes] = 0;
      nnodes++;
    }
    else if (!strncmp(line, "2000 ", 5)) {	/* Link declaration. */
      sscanf(line, "2000 %d %d %d ", &node1, &node2, &dummy);

      /* Check if broken. */
      if ((node1 >= nnodes) || (node2 >= nnodes) || 
	  (node1 < 0) || (node2 < 0)) {
	fprintf(stderr, "Link bad nodes (too big or too small):\n%s\n.", 
		line);
	fprintf(stderr, "parsed: %d, %d\n", node1, node2);
	exit(-1);
      }

      /* Check for link overflow. */
      if (nlinks[node1] == MAXLINKS) {
	fprintf(stderr, "MAXLINKS too small for node: %d\n", node1);
	exit(-1);
      }
      links[node1][nlinks[node1]] = node2;
      nlinks[node1]++;

      /* Add bidirectional link. */
      if (nlinks[node2] == MAXLINKS) {
	fprintf(stderr, "MAXLINKS too small for node: %d\n", node2);
	exit(-1);
      }
      links[node2][nlinks[node2]] = node1;
      nlinks[node2]++;
    
      /* Add the edge.  Should check for nedges overflow. */
      edge_from[nedges] = node1;
      edge_to[nedges] = node2;
      nedges++;
    }

    /* Skip any other type of line. */
  }
  /* Done reading.  What did we get? */
#if 0|COMMENT
  fprintf(stderr, "Read %d nodes\n", nnodes);
  fprintf(stderr, "   time  activ deliv avg xmit avg hop    oldest    mean    queue_  send_\n         pakts pakts   time     time     packet     age     full?   fail?\n");
#endif
}
      
/* Setup learning parameters. */
void
init_Q()
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

/* Before calling do_sim. */
void
do_sim_setup()
{
  int e;	/* Event to handle. */
  int i, j;

  /* Set up the "hardware". */
  for (i = 0; i < nnodes; i++) interqueuen[i] = interqueue;

  /* Links are "up" to start. */
  for (i = 0; i < nnodes; i++)
    for (j = 0; j < nnodes; j++) 
      down[i][j] = 0;

  /* Initialize some structures. */
  freelist = Nil;			/* Create free list. */
  for (e = 0; e < MAXEVENTS; e++)
    free_event(e);

  /* Put in initial injection and report events. */
  queuetop = Nil;
  e = create_event(0.0, 0);
  events[e].source = REPORT;
  events[e].etime = 0.0;	/* Do this right away. */
  push_event(e, 0.0);
  e = create_event(0.0, 0);
  events[e].source = INJECT;
  events[e].etime = poisson(callmean);
  push_event(e, 0.0);

  /* Empty all node queues. */
  for (i = 0; i < nnodes; i++) {
    enqueued[i] = 0.0;
    nenqueued[i] = 0;
  }
}

void
do_sim(timelimit)
     double timelimit;
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
    else if (e != Nil) bump_packet(e);
  }
}

/* Starts another packet going. */
int
start_packet(time)
     double time;
{
  int n_from, n_to;		/* From and to are uniform over nodes. */
  int e;
  
  if (active_packets >= maxpackets) return(Nil);

  n_to = create(nnodes);

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

  return(e);
}

/* Lets a node process one packet.  Chooses a link, sends the packet, */
/* uses the revised estimate to adjust routing scheme. */
int
bump_packet(e)
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

/* Returns a new event number (off the free list). */
int
create_event(time, dest)
     double time;
     int dest;
{
  int e;

  if (freelist == Nil) {
    fprintf(stderr, "Can't create new event, free list empty\n");
    return(-1);
  }

  /* Pop it off the free list. */
  e = freelist;
  freelist = events[e].left;

  /* Initialize new event. */
  events[e].dest = dest;
  events[e].source = UNKNOWN;
  events[e].node = UNKNOWN;
  events[e].birth = time;
  events[e].hops = 0;
  events[e].left = Nil;
  events[e].right = Nil;
  events[e].etime = time;
  events[e].qtime = time;
  return(e);
}

/* Returns 0 on failure. */
int
push_event(e, time)
     int e;
     double time;
{
  events[e].qtime = time;
  queuetop = meldheap(queuetop, e);

  return(1);
}

/* Sends node n's top of queue along given link.  If the destination */
 /* node has a full queue, we send the node's packet to itself. */
 /* Returns an estimate of the time to destination.  Packet must get */
 /* consumed if sent to its destination.  Returns a success or failure */
 /* signal as well. */
double
send(e, n, link)
     int e, n, link;
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
  new_est = time_in_queue+internode+
    estimate(n_to, events[e].dest);
  necho_packets++;
  return(new_est);	
}

/* Pretends to send node n's top of queue along given link.   */
 /* Returns an estimate of the time to destination.  Returns a success or failure */
 /* signal as well. */
double
pseudosend(e, n, link)
     int e, n, link;
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
  new_est = time_in_queue+internode+
    estimate(n_to, events[e].dest);
  necho_packets++;
  return(new_est);	
}

/* Given a node, link, and destination, incorporate a new estimate of */
 /* time to arrival. */

void
do_learn(n, link, dest, new_est)
     int n, link, dest;
     double new_est;
{
  double *cell = &Q[n][link][dest];	/* Cell in Q to update. */
  double out = (*cell);
  
  *cell += (new_est-out)*eta;  /* Move a fraction to new_est. */
}

/* Compute an outgoing link on which to send n's topmost packet in */
 /* order to route it to dest. */
int
choose_link(n, dest)
     int n, dest;
{

  switch (strat) {
  case PRIOR:
  case LEARN:	/* True learning. */
    {
      double best = estimate(n, dest);
      int i, best_count = 0, best_link;

      for (i = 0; i < nlinks[n]; i++) {
	if (Q[n][i][dest] <= best+epsilon) {
	  best_count++;
	  if (one_in(best_count)) best_link = i;
	}
      }
      return(best_link);
    }
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

/* Remove an entry from the priority queue.  Return the */
 /* event which was removed.  Returns Nil if event queue empty. */
int
pop_event()
{
  int e;

  e = queuetop;
  if (e == Nil) return(e);
  queuetop = meldheap(events[e].left, events[e].right);
  events[e].left = Nil;
  events[e].right = Nil;
  return(e);
}

/* Given a used event structure, return it to the free list. */
void
free_event(e)
     int e;
{
  events[e].left = freelist;
  freelist = e;
}

/* How long from node to dest? */
double
estimate(n_from, n_to)
     int n_from, n_to;
{
  double best = Q[n_from][0][n_to];
  double current;
  int i;
  
  for (i = 1; i < nlinks[n_from]; i++) {
    current = Q[n_from][i][n_to];
    if (current < best) best = current;
  }

  return(best);
}

/* Compute all shortest paths. */
void
compute_best()
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


/* Makes a new heap out of two heaps. */
int
meldheap(h1, h2)
     int h1, h2;
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

/* Report stuff. */

int nodes_alive;
double oldest_node;
double total_age;
#define MEAN(x) (DOUBLE(x)/DOUBLE(nodes_alive))

void
report0(h1, time)
     int h1;
     double time;
{
  double age;

  if (h1 == Nil) return;
  if (events[h1].source >= 0) {
    nodes_alive++;
    age = time - events[h1].birth;
    if (age > oldest_node) oldest_node = age;
    total_age += age;
  }
  report0(events[h1].left, time);
  report0(events[h1].right, time);
}

double
poisson(arr)
     double arr;
{
  return(-log(bran(0.0,1.0))/arr);
}

void
interactive_report(time, e)
     double time;
     int e;
{
  int argc;
  char *argv[100];
  char commands[1000], word[1000];
  double next_report = time+1000.0;
  int i, badcmd;
  int n1, n2, edge;
  char *ptr;

  if (time > 0.0) {
    printf("{%f %f %f} ", time, DOUBLE(total_routing_time)/DOUBLE(routed_packets),
	   DOUBLE(total_hops)/DOUBLE(routed_packets));
    printf("{");
    for (i = 0; i < nedges; i++)
      printf("%d ", link_activity[edge_from[i]][edge_to[i]]
	     +link_activity[edge_to[i]][edge_from[i]]); 
    printf("}\n"); 
    fflush(stdout);
  }
  
  routed_packets = 0;	/* Reset. */
  total_routing_time = 0;
  total_hops = 0;
  for (i = 0; i < nedges; i++)
    link_activity[edge_from[i]][edge_to[i]] =
      link_activity[edge_to[i]][edge_from[i]] = 0;

  badcmd = 1;
  /* loop until a continuation command appears. */
  while (badcmd) {
    gets(commands);
    /* Parse into argc, argv (trailing blank counts as an argument). */
    argc = 1;	/* argv[0] doesn't count. */
    ptr = commands;
    while (*ptr) {
      while (*ptr && *ptr == ' ') ptr++;	/* Skip leading spaces. */
      argv[argc] = ptr;			/* That's the start. */
      while (*ptr && *ptr != ' ') ptr++;	/* Skip non-spaces. */
      if (*ptr) *(ptr++) = '\0';		/* Mark the end. */
      argc++;
    }
    /* Catch policy request.  (ignores other commands on line). */
    if ((argc >= 3) && (!strcmp("pol", argv[1]))) {
      dump_dest(atoi(argv[2]));
    }
    else badcmd = 0;
  }

  get_globals(argc, argv);

  events[e].etime += interreport;
  push_event(e, time);
}

void
dump_dest(node)
     int node;
{
  int j, l;
  
  printf("{");
  for (j = 0; j < nnodes; j++) {
    l = choose_link(j, node);
    printf("%d ", links[j][l]);
  }
  printf("}\n");
  fflush(stdout);
}

