from main import NetworkSimulatorEnv
from q_agent import networkTabularQAgent
import gym
import q_agent
import numpy as np
from gym import spaces, envs

def main():
    env = NetworkSimulatorEnv()
    observation = env.reset()

    agent = networkTabularQAgent(env.nnodes, env.nedges)
    done = False
    r_sum = 0
    config = agent.config

    for t in range(10000000):
        observation = observation[1]
        n = observation[0]
        dest = observation[1]

        # #VL??? QL rn
        for action in xrange(env.nlinks[n]):
            reward, next_state = env.pseudostep(action)
            # print "state:{} action:{}".format((n,dest), action)

            agent.learn((n,dest), next_state, reward, action, done)
            action  = agent.act(observation, env.nlinks)

        #Random
        #action = np.random.choice(env.nlinks[n],1)[0]

        #Precompute Distance Stuff
        # best = env.distance[n][dest]
        # for link in xrange(env.nlinks[n]):
        #     if env.distance[env.links[n][link]][dest] + 1 == best:
        #         action = link

        #print" send from:{} to dest:{}, via action:{}, which takes us to:{}".format( n, dest, action, env.links[n][action])
        new_observation, reward, done, _ = env.step(action)
        # print "state:{} action:{}".format((n,dest), action)
        agent.learn((n,dest), new_observation[0], reward, action, done)
        observation = new_observation
        r_sum = r_sum + reward

        if t%10000 == 0:
            if env.routed_packets != 0:
                print "time:{}, throughput:{}, length of average route:{}, r_sum:{}".format(t, float(env.total_routing_time)/float(env.routed_packets), float(env.total_hops)/float(env.routed_packets), r_sum)
                env.routed_packets = 0
                env.total_routing_time = 0
                env.total_hops = 0


if __name__ == '__main__':
    main()
