from main import NetworkSimulatorEnv
from q_agent import networkTabularQAgent
import gym
import q_agent
from gym import spaces, envs

def main():
    env = NetworkSimulatorEnv()
    observation = env.reset()
    agent = networkTabularQAgent(env.nedges)

    r_sum = 0
    config = agent.config

    for t in xrange(config["n_iter"]):

        action  = agent.act(observation[0])

        new_observation, reward, done, _ = env.step(action)

        agent.learn(observation[0], new_observation[0], reward, action, done)

        observation = new_observation

        r_sum = r_sum + reward

        if t%1000 == 0:
            if env.routed_packets != 0:
                print "time:{}, throughput:{}, length of average route:{}".format(t, env.total_routing_time/env.routed_packets, env.total_hops/env.routed_packets)


if __name__ == '__main__':
    main()
