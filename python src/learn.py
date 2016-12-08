#/*Set up/ running the learning stuff!!!
import numpy as np



def estimate_Q(n_from, n_to):

    best = Q[n_from][0][n_to]

    for i in range(nlinks[n_from]):
        current = Q[n_from][i][n_to]
        if current < best:
            best = current

    return(best)
}


def do_learn(n, link, dest, new_est):
    Q[n][link][dest] = Q[n][link][dest] + (new_est-out)*eta


#Lets a node process one packet.  Chooses a link, sends the packet,
#uses the revised estimate to adjust routing scheme.
def  bump_packet_Q(e):

  n = events[e].node
  dest = events[e].dest

  if learntype == VL:

    # For every possible link, get a better estimate.
    for i in range(nlinks[n]):
        new_est = pseudosend(e, n, i)
        #if vprob == 1.0) or (withprob(vprob))) as vprob == 1.0
	    do_learn(n, i, dest, new_est)

  #We need to pick an outgoing link.
  link = choose_link(n, dest)

  # We also ask the new node for an estimate of the arrival time. */
  new_est = send(e, n, link)

  # Now, independent of whether the send worked, we can update the */
  # Q value for that node, link, destination triple by making it */
  # closer to the new estimate. */
  do_learn(n, link, dest, new_est)

}


#Lets a node process one packet.  Chooses a link, sends the packet, */
#uses the revised estimate to adjust routing scheme. */
def bump_packet_sarsa(e):

    last_node = events[e].node
    dest = events[e].dest

    old_link = events[e].current_link

    if learntype == VL or learntype == SARSA:
    # For every possible link, get a better estimate.
    for i in range(nlinks[n]):
        new_est = pseudosend(e, n, i)
        #if vprob == 1.0) or (withprob(vprob))) as vprob == 1.0
        do_learn(n, i, dest, new_est)

       for j in range(nlinks[links[last_node][i]]):
           new_est = pseudosend(e, last_node, j)

           #if ((vprob == 1.0)||(withprob(vprob)))
           do_learn(last_node, j, dest, new_est)



    #Take action A and get back a weird
    #reward/value-to-the-end-from-the-state-action-a-took-us-to estimate back */
    #printf("this is last_node: %d\n",last_node );
    new_est = send(e, last_node, old_link)

    next_node = events[e].node

    #printf("this is next_node and  links[last_node][link]:%d,  %d\n",next_node, links[last_node][old_link]);
    new_link = choose_link(next_node, dest)

    new_est = new_est + Q[next_node][new_link][dest]


    if new_link != old_link:
      do_learn_sarsa(last_node, dest, old_link, next_node, new_link, new_est)

    events[e].current_link = new_link;


  # /* Now, independent of whether the send worked, we can update the */
  # /* Q value for that node, link, destination triple by making it */
  # /* closer to the new estimate. */
def do_learn_sarsa(last_node, dest,  old_link, next_node,  next_link,  new_est):

    Q[last_node][old_link][dest]  = Q[last_node][old_link][dest]+ eta*(new_est + Q[next_node][next_link][dest]
                                                - Q[last_node][old_link][dest]);


  #
  #
  # That's because in real Q learning it would look like this:
  # init Q:
  # init s_0
  #
  # Loop:
  # -  be in state s_t and use epsilon greedy evaluation to get action a_t
  #
  # -  take action a_t and get r_t and get/update s_t+1 (or however you want to index them)
  #
  # -  update Q(s_t, a_t) = Q(s_t,a_t) + alpha*[r_t + gamma*(max over all a_t+1 of: {Q(s_t+1,a_t+1)}) - Q(s_t, a_t)]
  #
  #
  #
  # Instead we do this:
  # init Q
  # init s_0
  #
  # loop:
  # - be in state (events[e].node, events[e].dest)  (s_t is a tuple of ints in our case)
  #
  # - a_t = choose_link(s_t) (this uses epsilon greedy evalution to get a_t)
  #
  # - take action a_t and update the state to s_t+1 ( or in code we update events[e].node = links[events[e].node][link])
  #       and get new_est which is currently a "negative reward":
  #           [r:=(queuing time in state s_t(q) and transmission delay to state s_t+1 (s)]
  #       plus "value estimate from the next state" or technically
  #           (min over all a_t+1 Q(s_t+1, a_t+1) reffered to very stupidly as t in the paper) )
  #       [so in code  t:= estimate_Q(s_t+1)]
  #
  #
  # - then lastly we call do_learn which updates the Q matrix as follows:
  #
  #       Q(s_t, a_t) = Q(s_t,a_t) + alpha*[(q + s + t) - Q(s_t, a_t) ]
  #
  # or in code:
  # if "s_t" was (events[e].node,events[e].dest) at the beginning of the episode then to do the q update we define some variables
  #
  # last_node = events[e].node (at the beginning of the episode)
  # link = choose_link(events[e].node, events[e].dest)
  # next_node = events[e].node (after we call send())
  #
  #       Q[events[e].node][link][events[e].dest] += alpha[ [(events[e].etime - events[e].qtime-internode)
  #                                                     + (internode) + estimate_Q(links[events[e].node][link], events[e].dest)
  #                                                   ]
  #                                                   - Q[events[e].node][link][events[e].dest]
  #                                                 ]
  #
  #
  # So if we want to convert this to SARSA
  # init Q
  # init s_0
  # - a_0 = choose_link(s_0) (this uses epsilon greedy evalution to get a_0)
  #
  # loop:
  # - take action a_t and update the state to s_t+1 ( or in code we update events[e].node = links[events[e].node][link])
  #       and get new_est which is currently a "negative reward":
  #           [r:=(queuing time in state s_t(q) and transmission delay to state s_t+1 (s)]
  #       plus "value estimate from the next state" or technically
  #           (min over all a_t+1 Q(s_t+1, a_t+1) reffered to very stupidly as t in the paper) )
  #       [so in code  t:= estimate_Q(s_t+1)]
  #
  # - a_t+1 = choose_link(s_t+1) (this uses epsilon greedy evalution to get a_t)
  #
  #
  # - then we call do_learn which updates the Q matrix as follows:
  #
  #       Q(s_t, a_t) = Q(s_t,a_t) + alpha*[(q + s) + Q(s_t+1, a_t+1) - Q(s_t, a_t) ]
  #
  # or in code:
  #
  # last_node = events[e].node (at the beginning of the episode)
  # old_link = choose_link(events[e].node, events[e].dest)
  # next_node = events[e].node (after we call send())
  # new_link = choose_link(events[e].node, events[e].dest) (after we've called send that is)
  #
  # *cell  += alpha*(((events[e].etime - events[e].qtime-internode)
  #                                               + (internode) + Q[current_node][new_link][dest_node]
  #                                             )
  #                                             - Q[last_node][old_link][dest_node]
  #                                           )
  #
  #
  #
  #
  #
  # lastly we have to update the action a_t = a+t+1
  #
  # */
