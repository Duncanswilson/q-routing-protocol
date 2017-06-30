# RL for Network Packet Routing  

This repo is a python port of the c implementation of _Packet Routing in Dynamically Changing Networks: A Reinforcement Learning approach_ (found here:https://www.cs.cmu.edu/~jab/cv/pubs/boyan.q-routing.pdf) 

The network simulator is implemented in `envs/simulator.py` and extends the OpenAI gym environment class. 

To run this code simply choose and agent file (currently we have Q learning and SARSA) and run:

```
python do_TYPE_learning.py
```
so for instance, to run the Q learning agent, type

```
python do_q_learning.py
```

in this directory.


The agent's performance is written to the console.
 
