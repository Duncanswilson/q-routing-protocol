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


events = 0

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
NIL = Nil =  -1


class NetworkSimulatorEnv(gym.Env):

    #We init the network simulator here
    def __init__(self):
        self.viewer = None
        self.graphname = 'data/lata.net'
        self.done = False
        self.success_count = 0
        self.nnodes = 0
        self.nedges = 0
        self.enqueued = {}
        self.nenqueued = {}
        self.interqueuen = []
        self.event_queue = []#Q.PriorityQueue()
        self.nlinks = {}
        self.links = collections.defaultdict(dict)
        self.total_routing_time = 0.0
        self.routed_packets = 0
        self.total_hops = 0
        self.current_event = event(0.0, 0) #do I need to do this?
        self.internode = 1.0
        self.interqueue = 1.0
        self.active_packets = 0
        self.queuelimit = 100
        self.send_fail = 0
        self.callmean = 1 #network load

        self.distance = []#np.zeros((self.nnodes,self.nnodes))
        self.shortest =  []#np.zeros((self.nnodes,self.nnodes))


        self.next_dest = 0
        self.next_source = 0
        self.injections = 0
        self.queue_full = 0

        self.events = 0

        self.sources = [52, 63, 21, 9, 96, 1, 89, 0, 102, 67, 21, 48, 110, 16, 68, 112, 88, 24, 6, 64, 28, 25, 0, 72, 63, 45, 18, 23, 33, 86, 30, 25, 56, 35, 81, 107, 0, 56, 71, 3, 46, 47, 24, 53, 20, 95, 53, 67, 58, 60, 28, 99, 14, 16, 28, 25, 7, 65, 100, 52, 4, 100, 13, 42, 59, 15, 35, 77, 89, 63, 35, 42, 56, 69, 61, 74, 73, 24, 58, 7, 0, 113, 5, 14, 37, 103, 63, 10, 69, 59, 63, 55, 25, 77, 65, 34, 0, 41, 9, 46, 18, 54, 35, 75, 41, 105, 8, 113, 32, 108, 55, 19, 64, 19, 19, 95, 47, 40, 114, 14, 17, 10, 51]
        self.dests = [77, 21, 65, 35, 80, 59, 78, 72, 112, 2, 80, 101, 2, 57, 0, 13, 42, 72, 19, 56, 66, 22, 50, 80, 16, 24, 89, 110, 65, 113, 71, 87, 68, 52, 87, 112, 8, 95, 74, 79, 18, 50, 83, 101, 38, 31, 114, 109, 100, 58, 38, 53, 83, 11, 67, 57, 83, 49, 36, 80, 79, 60, 30, 102, 86, 69, 91, 37, 109, 28, 56, 65, 55, 42, 58, 4, 83, 16, 98, 96, 14, 66, 73, 97, 28, 40, 43, 28, 90, 40, 49, 74, 21, 59, 77, 34, 82, 103, 39, 23, 2, 36, 93, 68, 85, 88, 90, 115, 35, 27, 43, 103, 71, 3, 35, 6, 97, 7, 54, 10, 93, 35, 66]

        self.next_source = 0
        self.next_dest = 0

    def _step(self, action):
        # if(self.total_routing_time/self.routed_packets < 10): #totally random, need change

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
            reward = time_in_queue + self.internode  #possibly change? totally random currently

            self.routed_packets +=  1
            self.nenqueued[current_node] -= 1
            self.total_routing_time +=  current_time - current_event.birth + self.internode
            self.total_hops += current_event.hops + 1

            self.active_packets -= 1

            self.current_event = self.get_new_packet_bump()

            if self.current_event == NIL:
                return ((current_event.node, current_event.dest), (current_event.node, current_event.dest)), reward, self.done, {}
            else:
                return ((current_event.node, current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}

        else:
            # #if the queue is full at the next node, set destination to self
            if self.nenqueued[next_node] >= self.queuelimit:
                 self.send_fail = self.send_fail + 1
                 next_node = current_node

            reward =  time_in_queue + self.internode

            current_event.node = next_node #do the send!
            current_event.hops += 1
            next_time = max(self.enqueued[next_node]+self.interqueuen[next_node], current_time + self.internode) #change this to nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode); eventually
            current_event.etime = next_time
            self.enqueued[next_node] = next_time

            current_event.qtime = current_time
            if type(current_event) == int:
                print "this is current_event:{}".format(current_event)
            heapq.heappush(self.event_queue,((current_time+1.0, -self.events), current_event))
            self.events += 1

            self.nenqueued[next_node] += 1
            self.nenqueued[current_node] -= 1


            self.current_event = self.get_new_packet_bump()

            if self.current_event == NIL:
                return ((current_event.node, current_event.dest), (current_event.node, current_event.dest)), reward, self.done, {}
            else:
                return ((current_event.node, current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}


    def _reset(self):
        self.readin_graph()
        self.distance = np.zeros((self.nnodes,self.nnodes))
        self.shortest =  np.zeros((self.nnodes,self.nnodes))
        self.compute_best()
        self.done = False
        self.interqueuen = [self.interqueue]*self.nnodes

        self.event_queue =[] #Q.PriorityQueue()
        self.total_routing_time= 0.0

        self.enqueued = [0.0]*self.nnodes
        self.nenqueued = [0]*self.nnodes


        inject_event = event(0.0, 0)
        inject_event.source = INJECT
        if self.callmean == 1.0:
            inject_event.etime = -math.log(random.random())
        else:
            inject_event.etime = -math.log(1- random.random())*float(self.callmean)


        self.events = 1

        inject_event.qtime = 0.0
        heapq.heappush(self.event_queue,((1.0, -self.events), inject_event))
        self.injections += 1
        self.events += 1

        self.current_event = self.get_new_packet_bump()


        return((self.current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest))


    ###########helper functions############################
    # Initializes a packet from a random source to a random destination
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





    def start_packet(self, time):
        source = np.random.random_integers(0,self.nnodes-1)
        dest = np.random.random_integers(0,self.nnodes-1)


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

        current_event =  heapq.heappop(self.event_queue)[1]
        current_time = current_event.etime

        #make sure the event we're sending the state of back is not an injection
        while current_event.source == INJECT :
             if self.callmean == 1.0 or self.callmean == 0.0:
                 current_event.etime += -math.log(1 - random.random())
             else:
                 current_event.etime += -math.log(1- random.random())*float(self.callmean)

             current_event.qtime = current_time

             heapq.heappush(self.event_queue,((current_time+1.0, -self.events), current_event))
             self.events += 1
             current_event = self.start_packet(current_time)
             if current_event == NIL :
                 current_event =  heapq.heappop(self.event_queue)[1]


        if current_event == NIL :
            current_event =  heapq.heappop(self.event_queue)[1]
        return current_event

    def pseudostep(self, action):

        current_event = self.current_event
        current_time = self.current_event.etime
        current_node = self.current_event.node

        time_in_queue = current_time - current_event.qtime - self.internode

        reward =  time_in_queue + self.internode

        #if the link wasnt good
        if action < 0 or action not in self.links[current_node]:
            return reward, (current_node, current_event.dest)

        else:
            next_node = self.links[current_node][action]
            if next_node != current_event.dest:
                next_time = max(self.enqueued[next_node]+self.interqueuen[next_node], current_time + self.internode) #change this to nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode); eventually

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
