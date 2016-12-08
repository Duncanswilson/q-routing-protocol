import collections
import numpy as np
from random import random

class networkTabularSARSAAgent(object):
    """
    Agent implementing tabular Q-learning for the NetworkSimulatorEnv.
    """

    def __init__(self, num_nodes, num_actions, distance, nlinks):
        self.config = {
            "init_mean" : 0.0,      # Initialize Q values with this mean
            "init_std" : 0.0,       # Initialize Q values with this standard deviation
            "learning_rate" : 0.7,
            "eps": 0.07,            # Epsilon in epsilon greedy policies
            "discount": 1,
            "n_iter": 10000000}        # Number of iterations
        self.q = np.zeros((num_nodes,num_nodes,num_actions))

        for src in range(num_nodes):
            for dest in range(num_nodes):
                for action in range(nlinks[src]):
                    self.q[src][dest][action] = distance[src][dest] #np.zeros((num_nodes,num_nodes,num_actions))



    def act(self, state, nlinks,  eps=None):
        n = state[0]
        dest = state[1]

        best = self.q[n][dest][0]
        best_action = 0
        for action in range(nlinks[n]):
            if self.q[n][dest][action] < best:  #+ eps:
                best = self.q[n][dest][action]
                best_action = action

        # if np.random.random() <= eps :
        #     best_action = np.random.choice((0.0, nlinks[n]))


        return best, best_action


    def learn(self, current_event, next_event, reward, action, done, nlinks):

        n = current_event[0]
        dest = current_event[1]

        n_next = next_event[0]
        dest_next = next_event[1]

        future, future_action = self.act(next_event, nlinks)

        #Q learning
        self.q[n][dest][action] += (reward + self.config["discount"]*future - self.q[n][dest][action])* self.config["learning_rate"]
        # #