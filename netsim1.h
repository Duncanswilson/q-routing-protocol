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
#define MAXEVENTS      100000
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
#define SARSA 7

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
  int current_link;
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
int learntype = SARSA;			/* What learning type (V/Q)? */
char *graphname = "lata.net";	/* Where get the network? */
double interreport = 100.0;		/* Time between reports. */
double interqueue = 1.0;		/* Time between handlings. (init) */
double internode = 1.0;			/* Time in transit. */
double epsilon = 0.0;			/* Tolerance in Q for link choice. */
int maxpackets = 10000;			/* Packet limit in network. */
int quit_criteria = 3;			/* How many successful reports */
					/* before quitting? */
int queuelimit = 1000;			/* Max elements per queue. */
int queuelevels = 10;			/* Queue levels to perceptron. */
double eta = 0.7;			/* How fast to learn? */
double vprob = 1.0;			/* prob of random echo packet. */
double callmean = 1;			/* When next call? */
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
void do_learn_sarsa();		/* Incorporate a new estimate. */
int choose_link();		/* Given a node and dest, choose a link. */
int push_event();		/* Add an event to the queue. */
int pop_event();		/* Remove an entry.  Return packet. */
int create_event();		/* Returns a new event struct. */
void free_event();		/* Add event to free list. */
double estimate();		/* How long from node to dest? */
void compute_best();		/* All shortest paths. */
void interactive_report();	/* Get some stats.  Get set again. */
void dump_dest();		/* Policy for a given dest. */

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

/* Given a used event structure, return it to the free list. */
void
free_event(e)
     int e;
{
  events[e].left = freelist;
  freelist = e;
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
