from main_sarsa import NetworkSimulatorEnv
from sarsa_agent import networkTabularSARSAAgent
import gym
import sarsa_agent
import numpy as np
from gym import spaces, envs

def main():

    callmean = 0.0
    for i in range(10):
        env = NetworkSimulatorEnv()
        state_pair = env.reset()
        agent = networkTabularSARSAAgent(env.nnodes, env.nedges, env.distance, env.nlinks)
        done = False
        r_sum = 0
        config = agent.config
        current_state = state_pair[0]
        n = current_state[0]
        dest = current_state[1]

        for action in xrange(env.nlinks[n]):
            reward, next_state = env.pseudostep(action)
            agent.learn(current_state, next_state, reward, action, done, env.nlinks)


        action_est, action = agent.act(current_state, env.nlinks)


        for t in range(100001):
            if not done:

                state_pair, reward, done, _ = env.step(action)

                next_state = state_pair[0]

                agent.learn(current_state, next_state, reward, action, done, env.nlinks)

                current_state = state_pair[1]
                n = current_state[0]
                dest = current_state[1]

                if t%100000 == 0:
                    if env.routed_packets != 0:
                        print "sarsa learning with callmean:{} time:{}, average delivery time:{}, length of average route:{}, r_sum:{}".format(i, t, float(env.total_routing_time)/float(env.routed_packets), float(env.total_hops)/float(env.routed_packets), r_sum)



if __name__ == '__main__':
    main()
