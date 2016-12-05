import collections
import numpy as np

class networkTabularQAgent(object):
    """
    Agent implementing tabular Q-learning for the NetworkSimulatorEnv.
    """

    def __init__(self, num_actions):
        # if not isinstance(observation_space, discrete.Discrete):
        #     raise UnsupportedSpace('Observation space {} incompatible with {}. (Only supports Discrete observation spaces.)'.format(observation_space, self))
        # if not isinstance(action_space, discrete.Discrete):
        #     raise UnsupportedSpace('Action space {} incompatible with {}. (Only supports Discrete action spaces.)'.format(action_space, self))
        # self.observation_space = observation_space
        # self.action_space = action_space
        # self.action_n = action_space.n
        self.config = {
            "init_mean" : 0.0,      # Initialize Q values with this mean
            "init_std" : 0.0,       # Initialize Q values with this standard deviation
            "learning_rate" : 0.1,
            "eps": 0.05,            # Epsilon in epsilon greedy policies
            "discount": 0.95,
            "n_iter": 100000}        # Number of iterations
        # self.config.update(userconfig)
        self.q = collections.defaultdict(lambda: self.config["init_std"] * np.random.randn(num_actions) + self.config["init_mean"])


    def act(self, observation, eps=None):
        if eps is None:
            eps = self.config["eps"]
        # epsilon greedy.
        action = np.argmax(self.q[observation]) #if np.random.random() > eps else self.action_space.sample()
        return action


    def learn(self, observation, new_observation, reward, action, done):

        future = 0.0
        if not done:
            future = np.max(self.q[new_observation[0]])

        self.q[observation][action] -= self.config["learning_rate"] * (self.q[observation][action] - reward - self.config["discount"] * future)
