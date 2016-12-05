

#Defines.
MAXNODES = 128
MAXLINKS = 9
MAXEVENTS = 100000
QUEUELEVELS = 60
MAXEDGES = (MAXNODES*MAXLINKS)

#Routing  Strategies.
RANDOM =  0		    #/* Pick a link, any link. */
LEARN =  1			#/* Best according to learning. */
BEST =  2			#/* Any (random) shortest path. *
BESTATIC =  3		#/* One (fixed) shortest path. */
PRIOR  = 4			#/* Learning starting from BEST. */

QL =  5
VL =  6
SARSA =  7

#init q matrix
Q =  np.zeros(MAXNODES, MAXLINKS,  MAXNODES)


links = [None]*MAXNODES[None]*MAXLINKS;		/* Who connected to? */
nlinks = [None]*MAXNODES			/* How many connected to? */
xnode = [None]*MAXNODES
ynode = [None]*MAXNODES  	/* Coordinates of nodes. */



# /* For tcl stuff. */
# int link_activity[MAXNODES][MAXNODES];
# int down[MAXNODES][MAXNODES];
# int edge_from[MAXEDGES];
# int edge_to[MAXEDGES];
# int nedges;
# not sure if needed

# /* Event structure. */
class event:
    def __init__(self, time,  dest):
        #/* Initialize new event. */
        self.dest = dest
        self.source = UNKNOWN
        self.node = UNKNOWN
        self.birth = time
        self.hops = 0
        self.etime = time
        self.qtime = time


events = [None]*MAXEVENTS

# /* Special events. */
INJECT = -1
REPORT = -2
END_SIM = -3
UNKNOWN = -4

# /* Define. */
Nil =  -1


# /* Represent shortest path. */
shortest = [None]*MAXNODES[None]*MAXNODES	#Returns link. */
distance = [None]*MAXNODES[None]*MAXNODES   #Returns steps. */
interqueuen = [None]*MAXNODES		        #Initialized to interqueue. */

# /* User variables. */
strat = LEARN			#/* How choose actions? */
learntype = SARSA		#	/* What learning type (V/Q)? */
graphname = "lata.net"	#/* Where get the network? */
interreport = 100.0		#/* Time between reports. */
interqueue = 1.0		#/* Time between handlings. (init) */
internode = 1.0		#	/* Time in transit. */
epsilon = 0.3			#/* Tolerance in Q for link choice. */
maxpackets = 10000		#	/* Packet limit in network. */
quit_criteria = 3		#	/* How many successful reports */
					       #/* before quitting? */
queuelimit = 1000		#	/* Max elements per queue. */
queuelevels = 10		#	/* Queue levels to perceptron. */
eta = 0.7		    	#/* How fast to learn? */
vprob = 1.0			#/* prob of random echo packet. */
callmean = 1			#/* When next call? */
callstd = 0.2

/* Report variables. */
routed_packets = 0
total_routing_time = 0
active_packets = 0
converge = 0
best_transit = 10000.0
queue_full = 0
necho_packets = 0
total_hops = 0
send_fail = 0


import gym
from gym import spaces
import numpy as np
from os import path
import math
import random


class NetworkSimulatorEnv(gym.Env):

    #We init the network simulator here 
    def __init__(self):
        self.viewer = None

        self.done = False
        self.success_count = 0


        self.interqueuen = [interqueue]*MAXNODES		        #Initialized to interqueue. */
        self.down =  [0][0]*(MAXNODES,MAXNODES)


        self.action_space = spaces.Box(MAXLINKS (1,))
        self.observation_space = spaces.Box(low=np.asarray([0,0]), high=np.asarray([0,0]))

        self.event_queue = []

        e = event(0.0, 0)
        events[e].source = REPORT
        events[e].etime = 0.0
        heappush(event_queue, (0.0, e))

        e = event(0.0, 0)
        events[e].source = INJECT
        events[e].etime = np.random.poisson(callmean)
        heappush(event_queue, (0.0, e))

        enqueued = [0.0]*MAXNODES
        nenqueued = [0]*MAXNODES
  }

        self.reset()
        print("here")


    def _reset(self):

        self.state = np.random.uniform(low=0, high=nnodes, size=(2,))

        self.done = False

        self.interqueuen = [interqueue]*MAXNODES		        #Initialized to interqueue. */
        self.down =  [0][0]*(MAXNODES,MAXNODES)

        self.action_space = spaces.Box(MAXLINKS (1,))
        self.observation_space = spaces.Box(low=np.asarray([0,0]), high=np.asarray([0,0]))

        self.event_queue = []

        e = event(0.0, 0)
        events[e].source = REPORT
        events[e].etime = 0.0
        heappush(event_queue, (0.0, e))

        e = event(0.0, 0)
        events[e].source = INJECT
        events[e].etime = np.random.poisson(callmean)
        heappush(event_queue, (0.0, e))

        enqueued = [0.0]*MAXNODES
        nenqueued = [0]*MAXNODES
  }
        return self.state

# /* Returns a new event number (off the free list). */
def create_event(time, dest):

    if freelist == Nil:
      print("Can't create new event, free list empty")
      return()

    #/* Pop it off the free list. */
    e = freelist
    freelist = events[e].left

    #/* Initialize new event. */
    events[e] = event(time,dest)
    return(e)


# /* Returns 0 on failure. */
def push_event(e, time):

    events[e].qtime = time
    queuetop = meldheap(queuetop, e)

    return(1)

# /* Given a used event structure, return it to the free list. */
def free_event(e):
    events[e].left = freelist
    freelist = e

# /* Remove an entry from the priority queue.  Return the */
#  /* event which was removed.  Returns Nil if event queue empty. */
def pop_event():
    e = queuetop
    if e == Nil:
        return(e)
    queuetop = meldheap(events[e].left, events[e].right)
    events[e].left = Nil
    events[e].right = Nil
    return(e);


def main(argc, *argv[]):

    get_globals(argc, argv)

    randomize()

    init_graph(graphname)

    if (strat == BEST) or (strat == BESTATIC) or (strat == PRIOR):
        compute_best() 		#/* Initialize shortest paths. */

    do_sim_setup()

    do_sim()
