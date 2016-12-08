# My current thoughts about how to formulate this as a RL problem

So due to how the original authors set up the problem it is not immediately obvious that for some network G with N nodes and E edges, we have N Q learning agents, and each one's state space is the destination of the packet at time t (some natural number n in [0,N]) and the action is some link (or some e in the set neigh(n) for node n. ~I think??, check this assumption when implementing~).

But we can consider an equivalent formulation, only using one Q learning agent. The agent then is the protocol for the entire network, not just one node's protocol and thus the state space consists of (node, dest) tuples and the action space is still a link as specified above.

--------------------------------------------------------------------------------

Previously we have been thinking about the agent as wrapped up in the simulator but it will be helpful to decouple those.

To do that we'll be using Open AI's Gym protocol. A general introduction to the gym can be found here:
https://gym.openai.com/docs

## Environment Specs
- the environment is defined to be the network and a time-series of packet injections (controlled in the c code by randomness for (src,dest) tuples and a poisson distribution around the parameter callmean for injection rate or "network" load as they nebulously refer to it in the paper)

- stepping the environment will be handling the top event in the queue (for now) and the process will complete if the (total_routing_time)/(routed_packets) < 10. (this is 100% arbitrary so we should play around with it, but whatevs for now)

- upon init the environment will the first state action pair (generally this should be (some random node[source], some random node[dest]) but for now we're going to use a deterministic scenario to test stuff, see Current Development Outline for details)

### What the Environment Returns
for definitional clarity suppose we're at time t and in state (x,d) or s_t, our policy has returned link 3 as action a_t which leads to node y. let q_x be the queueing delay at node x and t be the transmission delay of the network.   

- the state that is returned comes in a few varieties.
possible states the environment can return are:
  - (x,d) if:
    - no such link exists
    - the link is down
  - (y,d) otherwise

- the reward (in order to match the original formulation) is currently defined as r:= -(q+t)
  - this seems like low hanging fruit to improve and should be our first target once everything works (:  


## Agent Specs
let alpha be the learning rate (a scalar) and gamma be the discount factor. (if you're confused about gamma, it's due to the original authors not using it! you can find more out about it here: https://en.wikipedia.org/wiki/Q-learning#Discount_factor)

- our agent can be seen as a network-specific learned routing protocol

  - initially the agent will be a simple Q-learning agent, so at a given timestep, the environment is in state s:

    - it will use epsilon greedy maximization for policy derivation to choose an action
     something like this:
     ```
     a = np.argmax(self.Q[observation.item()]) if np.random.random() > eps else self.action_space.sample()
     ```

    - take action a which comes from our policy and observe the reward and the state the environment returns

    - and update the Q matrix as follows:
      ```
      self.Q[s][a] += + alpha*(r + gamma*max_a'{self.Q[s][a']} - self.Q[s][a])
      ```
      (note the max function is pseudocode above and that this equation can totally be changed, see the section Duncan's Wild and Crazy Dreams for the Agent)



## Current Development Outline
  - Duncan:
~    - Understand the simulator better. For instance what is an episode? An epoch?~

~    - Understand how the c code reads in the graph file and read in the 6x6.net file in Python (in its own file g, come on...)~

~    - Create a deterministic time-series of ((src,dest),injection_time) tuples for testing our environment~

    ~- Finish writing the environment class~

I've finished the env and agent class. still issues in the environment with it never getting to the source. Could both be an issue with the policy or my simulator. I'll be refactoring all of it tomorrow morning!  :) 

  - Catherine:
    - Read over all of the code in the repo at a high level. Run some tests and make sure that you somewhat get what the C simulator is doing.

    - Pull the most recent version of Open AI's gym repo and read over their introduction documentation and this q learning agent https://github.com/openai/gym/blob/master/examples/agents/tabular_q_agent.py and a general reference to the example agents in their repo can be found here: https://github.com/openai/gym/blob/master/docs/agents.md.

    - Touch base with Duncan about what state the simulator environment is in and start developing the q learning agent class, based off of the one linked above. This will involve creating a class out of the shitty implementation I did in learn.py. def rely on the stuff in the gym repo and the C simulator more than what I wrote.

    - If the agent is easy and we're really ahead on time projections, it would be cool to make a visualization of the problem (like the other gym envs have) possibly start looking into doing this?   


## Duncan's Wild and Crazy Dreams for the Environment (sorted in order of pragmatic desire)

- Look into the old paper and other papers that use RL in networking situations and see if there are better reward function set ups. (& how they do policy evaluation, esp. if they're Q learning agents)

- A simple more realistic refactoring. Doing stuff like:
  - a setting for adding randomness to the delays
  - a parameterized setting for breaking links randomly (a simple 0-10 parameter, 10 being utter chaos)
  - researching realistic transmission delays
  - possibly incorporating other simulated delay parameters??  

- Write a visualization of the network using the same style of code as the other Open AI envs

- Create an incredibly basic GUI to break and repair links and change the load on the fly

- Look into a simulator like NS2 or Castalia and see if there are other ways of making the simulator more realistic.    


## Duncan's Wild and Crazy Dreams for the Agent (sorted in order of pragmatic desire)

- Play around with all of the different ways to approximate Q* (the optimal Q- function) see: https://gym.openai.com/docs/rl#q-learning

- Train a black box cross entropy agent


### we shouldn't actually implement this, but we should present it in the last section of the paper  
  - an interesting generalization exists if we were super dedicated. we could generate a huge dataset of networks and try and learn generally optimal protocols, potentially as an optimization for parameter initialization for a network specific protocol
