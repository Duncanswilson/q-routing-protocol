import collections
import numpy as np
from random import random

class networkTabularQAgent(object):
    """
    Agent implementing tabular Q-learning for the NetworkSimulatorEnv.
    """

    def __init__(self, num_nodes, num_actions, distance, nlinks):
        self.config = {
            "init_mean" : 0.0,      # Initialize Q values with this mean
            "init_std" : 0.0,       # Initialize Q values with this standard deviation
            "learning_rate" : 0.7,
            "eps": 0.1,            # Epsilon in epsilon greedy policies
            "discount": 1,
            "n_iter": 10000000}        # Number of iterations
        self.q = np.zeros((num_nodes,num_nodes,num_actions))

        for src in range(num_nodes):
            for dest in range(num_nodes):
                for action in range(nlinks[src]):
                    self.q[src][dest][action] = distance[src][dest]



    def act(self, state, nlinks,  best=False):
        n = state[0]
        dest = state[1]

        if best is True:
            best = self.q[n][dest][0]
            best_action = 0
            for action in range(nlinks[n]):
                if self.q[n][dest][action] < best:  #+ eps:
                    best = self.q[n][dest][action]
                    best_action = action
        else:
            best_action = int(np.random.choice((0.0, nlinks[n])))

        return best_action


    def learn(self, current_event, next_event, reward, action, done, nlinks):

        n = current_event[0]
        dest = current_event[1]

        n_next = next_event[0]
        dest_next = next_event[1]

        future = self.q[n_next][dest][0]
        for link in range(nlinks[n_next]):
            if self.q[n_next][dest][link] < future:
                future = self.q[n_next][dest][link]

        #Q learning
        self.q[n][dest][action] += (reward + self.config["discount"]*future - self.q[n][dest][action])* self.config["learning_rate"]
