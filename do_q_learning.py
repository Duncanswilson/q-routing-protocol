from main_q import NetworkSimulatorEnv
from q_agent import networkTabularQAgent
import gym
import q_agent
import numpy as np
from gym import spaces, envs

def main():
    callmean = 1.0
    for i in range(10):
        callmean += 1.0
        env = NetworkSimulatorEnv()
        state_pair = env.reset()
        env.callmean = callmean
        agent = networkTabularQAgent(env.nnodes, env.nedges, env.distance, env.nlinks)
        done = False
        r_sum = 0
        config = agent.config

        for t in range(10001):
            if not done:

                current_state = state_pair[1]
                n = current_state[0]
                dest = current_state[1]

                for action in xrange(env.nlinks[n]):
                    reward, next_state = env.pseudostep(action)
                    agent.learn(current_state, next_state, reward, action, done, env.nlinks)

                action_est, action  = agent.act(current_state, env.nlinks)
                state_pair, reward, done, _ = env.step(action)

                next_state = state_pair[0]
                agent.learn(current_state, next_state, reward, action, done, env.nlinks)

                if t%10000 == 0:
                    if env.routed_packets != 0:
                        print "q learning with callmean:{} time:{}, average delivery time:{}, length of average route:{}, r_sum:{}".format(i, t, float(env.total_routing_time)/float(env.routed_packets), float(env.total_hops)/float(env.routed_packets), r_sum)


if __name__ == '__main__':
    main()
