import gym
from gym import spaces
import numpy as np
import heapq
from os import path
from os import sys
import math
import random

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
        self.enqueued = []
        self.nenqueued = []
        self.interqueue = 1.0
        self.interqueuen = []
        self.event_queue = []
        self.nlinks = []
        self.links = [[]]
        self.total_routing_time = 0.0
        self.routed_packets = 0
        self.current_event = event(0.0, 0) #do I need to do this?
        self.internode = 1.0
        self.interqueue = 1.0
        self.active_packets = 0
        self.callmean = 3 #network load




    def _step(self, action):
        # if(self.total_routing_time/self.routed_packets < 10): #totally random, need change
        self.done = False

        current_event = self.current_event
        current_time = self.current_event.etime
        current_node = self.current_event.node

        #if the link was good
        if action < 0 or self.links[current_node][action] == None:
            reward = -( (current_event.etime - current_event.qtime) + self.internode)
            current_event.node = current_event.node #we don't move, line not needed obvi

            heapq.heappush(self.event_queue, (current_time, current_event))
            self.current_event = self.get_new_packet_bump()
            return ((current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}


        else:
            next_node = self.links[current_node][action]

            print "next_node:{} and current_event.dest:{}".format(next_node,current_event.dest)
            #handle the case where next_node is your destination
            if next_node == current_event.dest:
                self.routed_packets = self.routed_packets + 1
                total_routing_time = total_routing_time + current_time - current_event.birth + self.internode
                total_hops = total_hops + events[e].hops + 1

                reward = 1.0 #possibly change? totally random currently
                self.active_packets = self.active_packets - 1
                self.current_event = self.get_new_packet_bump()

                return ((current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}

            else:
                # #if the queue is full at the next node, set destination to self
                # if self.nenqueued[next_node] >= queuelimit:
                #     self.send_fail
                #     next_node = current_node

                current_event.node = next_node #do the send!
                current_event.hops = current_event.hops + 1
                next_time = current_time + self.internode #change this to nexttime = Max(enqueued[n_to]+interqueuen[n_to], curtime+internode); eventually
                current_event.etime = next_time

                self.enqueued[next_node] = next_time
                self.nenqueued[next_node] = self.nenqueued[next_node] + 1
                self.nenqueued[current_node] = self.nenqueued[current_node] - 1

                heapq.heappush(self.event_queue, (current_time, current_event))
                reward = -(( current_event.etime - current_event.qtime) + self.internode)
                self.current_event = self.get_new_packet_bump()


                return ((current_event.node, self.current_event.dest), (self.current_event.node, self.current_event.dest)), reward, self.done, {}


    def _reset(self):
        self.readin_graph()
        self.done = False
        self.interqueuen = [self.interqueue]*self.nnodes		        #Initialized to interqueue. */

        # self.action_space = spaces.Box(MAXLINKS (1,))
        # self.observation_space = spaces.Box(low=np.asarray([0,0]), high=np.asarray([self.nnodes,self.nnodes]))

        self.event_queue = []
        self.enqueued = [0.0]*self.nnodes
        self.nenqueued = [0]*self.nnodes

        # report_event = event(0.0, 0)
        # report_event.source = REPORT
        # report_event.etime = 0.0
        # heappush(self.event_queue, (0.0, report_event))

        inject_event = event(0.0, 0)
        inject_event.source = INJECT
        inject_event.etime = np.random.poisson(self.callmean)
        heapq.heappush(self.event_queue, (0.0, inject_event))

        self.current_event = self.get_new_packet_bump()

        heapq.heappush(self.event_queue, (self.current_event.etime, self.current_event))
        return (self.current_event.node, self.current_event.dest)


    ###########helper functions############################
    # Initializes a packet from a random source to a random destination
    #
    def readin_graph(self):
        self.nnodes = 0
        self.nedges = 0

        self.nlinks = [None]*1000000 # this too
        self.links = [[None]*1000000]*1000000 #please get back and fix this Duncan
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

        source = np.random.random_integers(0,self.nnodes)
        dest = np.random.random_integers(0,self.nnodes)

        #make sure we're not sending it to our source
        while source == dest:
            dest = np.random.random_integers(0,self.nnodes)

        #is the queue full? if so don't inject the packet
        # if self.nenqueued[soure] > queuelimit - 1:
        #     self.queue_full = self.queue_full + 1
        #     return(NIL) #deffo not needed

        self.nenqueued[source] = self.nenqueued[source] + 1

        self.active_packets = self.active_packets + 1
        current_event = event(time, dest)
        current_event.source = current_event.node = source

        return current_event

    def get_new_packet_bump(self):
        current_event =  heapq.heappop(self.event_queue)[1]
        current_time = current_event.etime

        #make sure the event we're sending the state of back is not an injection
        while current_event.source == INJECT:
            current_event.etime += np.random.poisson(self.callmean)
            current_event = self.start_packet(current_time)

        return current_event
