import gym
from gym import spaces
import numpy as np
import heapq
import collections
from os import path
from os import sys
import math
import random
try:
    import Queue as Q  # ver. < 3.0
except ImportError:
    import queue as Q


sources = [0,1,3]
dests = [2,5,11]

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

# /* Special events. */
INJECT = -1
REPORT = -2
END_SIM = -3
UNKNOWN = -4

# /* Define. */
Nil =  -1


class NetworkSimulatorEnv(gym.Env):

    #We init the network simulator here
    def __init__(self):
        self.viewer = None
        self.graphname = 'lata.net'
        self.done = False
        self.success_count = 0
        self.nnodes = 0
        self.nedges = 0
        self.enqueued = {}
        self.nenqueued = {}
        self.interqueuen = []
        self.event_queue = Q.PriorityQueue()
        self.nlinks = {}
        self.links = collections.defaultdict(dict)
        self.total_routing_time = 0.0
        self.routed_packets = 0
        self.total_hops = 0
        self.current_event = event(0.0, 0) #do I need to do this?
        self.internode = 1.0
        self.interqueue = 1.0
        self.active_packets = 0
        self.queuelimit = 1000
        self.send_fail = 0
        self.callmean = 1 #network load

        self.distance = collections.defaultdict(dict)
        self.shortest = collections.defaultdict(dict)

        self.next_dest = 0
        self.next_source = 0
        self.injections = 0
        self.queue_full = 0


    def _step(self, action):
        # if(self.total_routing_time/self.routed_packets < 10): #totally random, need change
        self.done = False

        current_event = self.current_event
        current_time = current_event.etime
        current_node = current_event.node

        time_in_queue = current_time - current_event.qtime - self.internode


        #if the link wasnt good
        if action < 0 or action not in self.links[current_node]:
            next_node = current_node

        else:
            next_node = self.links[current_node][action]
        #handle the case where next_node is your destination
        if next_node == current_event.dest:

            self.routed_packets +=  1
            self.nenqueued[current_node] -= 1
            self.total_routing_time +=  current_time - current_event.birth + self.internode
            self.total_hops += current_event.hops + 1

            reward = time_in_queue + self.internode  #possibly change? totally random currently
            self.active_packets -= 1

            self.current_event = self.get_new_packet_bump()

            return ((current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}

        else:
            # #if the queue is full at the next node, set destination to self
            if self.nenqueued[next_node] >= self.queuelimit:
                 self.send_fail = self.send_fail + 1
                 next_node = current_node

            current_event.node = next_node #do the send!
            current_event.hops += 1
            next_time = max(self.enqueued[next_node]+self.interqueuen[next_node], current_time + self.internode) #change this to nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode); eventually
            current_event.etime = next_time
            self.enqueued[next_node] = next_time

            self.event_queue.put((current_time, current_event))

            self.nenqueued[next_node] += 1
            self.nenqueued[current_node] -= 1

            reward = time_in_queue
            self.current_event = self.get_new_packet_bump()


            return ((current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}


    def _reset(self):
        self.readin_graph()
        self.compute_best()
        self.done = False
        self.interqueuen = [self.interqueue]*self.nnodes

        self.event_queue = Q.PriorityQueue()
        self.total_routing_time= 0.0

        self.enqueued = [0.0]*self.nnodes
        self.nenqueued = [0]*self.nnodes

        inject_event = event(0.0, 0)
        inject_event.source = INJECT
        inject_event.etime = float(np.random.poisson(self.callmean)) #change to 1.0 for hardcoding

        self.event_queue.put((0.0, inject_event))

        self.current_event = self.get_new_packet_bump()


        return((self.current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest))


    ###########helper functions############################
    # Initializes a packet from a random source to a random destination
    #
    def readin_graph(self):
        self.nnodes = 0
        self.nedges = 0

        graph_file = open(self.graphname, "r")

        for line in graph_file:
            line_contents = line.split()


            if line_contents[0] == '1000': #node declaration

                self.nlinks[self.nnodes] = 0
                self.nnodes = self.nnodes + 1


            if line_contents[0] == '2000': #link declaration

                node1 = int(line_contents[1])
                node2 = int(line_contents[2])

                self.links[node1][self.nlinks[node1]] = node2
                self.nlinks[node1] = self.nlinks[node1] + 1

                self.links[node2][self.nlinks[node2]] = node1
                self.nlinks[node2] = self.nlinks[node2] + 1

                self.nedges = self.nedges + 1


        print "Read in {} nodes and {} edges from file {}".format(self.nnodes, self.nedges, self.graphname)



    def start_packet(self, time):
        source = np.random.random_integers(0,self.nnodes-1) #change to  sources[self.next_source] for hardcoding
        dest = np.random.random_integers(0,self.nnodes-1) #change to dests[self.next_dest]

        #print "STARTING NEW PACKET with src:{} and dest:{}".format(source, dest)
        # self.next_source = self.next_source + 1
        # self.next_dest = self.next_dest + 1

        #make sure we're not sending it to our source
        while source == dest:
            dest = np.random.random_integers(0,self.nnodes-1)

        #is the queue full? if so don't inject the packet
        if self.nenqueued[source] > self.queuelimit - 1:
             self.queue_full += 1
             return(Nil)

        self.nenqueued[source] = self.nenqueued[source] + 1

        self.active_packets = self.active_packets + 1
        current_event = event(time, dest)
        current_event.source = current_event.node = source

        return current_event

    def get_new_packet_bump(self):

        current_event =  self.event_queue.get()[1]
        #print "were popping this from the queue: current_event.source:{}, current_event.node:{}, current_event.dest:{}".format(current_event.source, current_event.node, current_event.dest)

        current_time = current_event.etime

        #make sure the event we're sending the state of back is not an injection
        while current_event.source == INJECT or current_event == Nil:
            #print "WE SHOULD BE STARTING A NEW PACKET."
            current_event.etime += float(np.random.poisson(self.callmean)) #change to 1.0 for hardcoding
            #print "were pushing this onto the queue: current_event.source:{}, current_event.node:{}, current_event.dest:{}".format(current_event.source, current_event.node, current_event.dest)
            #print "with priority:{}".format(current_time)
            self.event_queue.put((current_time, current_event))
            current_event = self.start_packet(current_time)
            if current_event != Nil:
                self.injections = self.injections + 1
            else:
                current_event = self.event_queue.get()[1]

        return current_event

    def pseudostep(self, action):

        current_event = self.current_event
        current_time = self.current_event.etime
        current_node = self.current_event.node

        #if the link wasnt good
        if action < 0 or action not in self.links[current_node]:
            reward = -( (current_event.etime - current_event.qtime) + self.internode)
            return reward, (current_node, current_event.dest)

        else:
            next_node = self.links[current_node][action]

            if next_node == current_event.dest:
                reward =  -(( current_event.etime - current_event.qtime) + self.internode) #possibly change? totally random currently
                return  reward, (next_node, current_event.dest)

            else:
                next_time = max(self.enqueued[next_node]+self.interqueuen[next_node], current_time + self.internode) #change this to nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode); eventually
                reward = -(( current_event.etime - current_event.qtime) + self.internode)
                return reward, (next_node, current_event.dest)


    def compute_best(self):

        changing = True

        for i in xrange(self.nnodes):
            for j in  xrange(self.nnodes):
                if i == j:
                    self.distance[i][j] = 0
                else:
                    self.distance[i][j] = self.nnodes+1
                self.shortest[i][j] = -1

        while changing:
            changing = False
            for i in xrange(self.nnodes):
                for j in  xrange(self.nnodes):
                    #/* Update our estimate of distance for sending from i to j. */
                    if i != j:
                      for k in xrange(self.nlinks[i]):
                        if  self.distance[i][j] >  1 + self.distance[self.links[i][k]][j]:
                          self.distance[i][j] = 1 + self.distance[self.links[i][k]][j]	#/* Better. */
                          self.shortest[i][j] = k
                          changing = True
