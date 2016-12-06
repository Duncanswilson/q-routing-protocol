import collections
import numpy as np
from random import random

class networkTabularQAgent(object):
    """
    Agent implementing tabular Q-learning for the NetworkSimulatorEnv.
    """

    def __init__(self, num_nodes, num_actions):
        self.config = {
            "init_mean" : 0.0,      # Initialize Q values with this mean
            "init_std" : 0.0,       # Initialize Q values with this standard deviation
            "learning_rate" : 0.7,
            "eps": 0.00,            # Epsilon in epsilon greedy policies
            "discount": 1,
            "n_iter": 10000000}        # Number of iterations

        self.q = {}

        for src in range(num_nodes):
            for dest in range(num_nodes):
                self.q[(src,dest)] = {}
                for action in range(num_actions):
                    self.q[(src,dest)][action] = 0.0



    def act(self, observation, nlinks,  eps=None):
        n = observation[0]
        dest = observation[1]


        # if eps is None:
        #     eps = self.config["eps"]
        # # not really epsilon greedy.
        # for action in range(nlinks[n]):
        #     if self.q[observation][action] <= best + eps:
        #         best_action = action

        min_value = min(self.q[observation].itervalues())
        print "this is min val:{}".format(min_value)
        for k, v in self.q[observation].iteritems():
            if v == min_value:
                return(k)


    def learn(self, observation, new_observation, reward, action, done):
        #
        #
        future = min(self.q[new_observation].itervalues())
        #
        # print "this is future{}".format(future)
        # total = (reward + future - self.q[observation][action])
        #
        # self.q[observation][action] += total* self.config["learning_rate"]
