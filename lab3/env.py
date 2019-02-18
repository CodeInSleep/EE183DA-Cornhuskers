from utils import *
import numpy as np
from robot import *
import pdb
import time
# max iteration for value function
max_iteration = 20

class State:
    def __init__(self, x, y, heading, reward):
        # x: x coordinate in the grid-view
        # y: y coordinate in the grid-view
        self.x = x
        self.y = y
        self.heading = heading
        self.reward = reward

        self.iden = self.x+self.y*L+heading*(W*L)

    def __eq__(self, other):
        print('equating')
        # only compare the x and y coordinates of state
        return self.x == other.x and self.y == other.y

    def __repr__(self):
        return "State(grid_x: {}, grid_y: {}, heading: {}, reward: {}, \
            id: {})".format(self.x, self.y, self.heading, self.reward, self.iden)

    def __str__(self):
        return "({}, {}, {}, {})".format(self.x, self.y, self.heading, self.reward)

class Environment:
    def __init__(self, W, L, rewards, robot):
        self.W = W
        self.L = L
        
        # states in the environment in array-view
        self.states = []
        self.goal_state = None
        
        for h in headings:
            y_states = []
            for y in range(self.W):
                x_states = []
                for x in range(self.L):
                    # find goal state
                    state = State(x, self.W-1-y, h, rewards[y*self.W+x])
                    x_states.append(state)
                    if rewards[y*self.W+x] == 1:
                        self.goal_state = state
                y_states.append(x_states)
            self.states.append(y_states)

        if not self.goal_state:
            raise ValueError('There is no goal state in rewards')

        self.robot = robot

    def flattenStates(self):
        flatten = []
        for h in headings:
            for col in reversed(self.states[h]):
                flatten += col
        return flatten

    def stateAt(self, x, y, heading):
        # return the state at x and y (in grid view)
        return self.states[heading][self.W-1-y][x] 

    def action_to_take(self, state, goal_state):
        if state.x != goal_state.x or state.y != goal_state.y:
            goal_x = goal_state.x
            goal_y = goal_state.y
            # actions = [STAY, FORWARD_NOROT, FORWARD_CLK, FORWARD_CCLK,
            # BACKWARD_NOROT, BACKWARD_CLK, BAC_CCLK]
            if state.heading in UP:
                if state.x < goal_x and state.y < goal_y:
                    return Action.FORWARD_CLK
                elif state.x == goal_x and state.y < goal_y:
                    return Action.FORWARD_NOROT
                elif state.x > goal_x and state.y < goal_y:
                    return Action.FORWARD_CCLK
                elif state.x < goal_x and state.y == goal_y:
                    return Action.FORWARD_CLK
                elif state.x > goal_x and state.y == goal_y:
                    return Action.FORWARD_CCLK
                elif state.x < goal_x and state.y > goal_y:
                    return Action.BACKWARD_CCLK
                elif state.x == goal_x and state.y > goal_y:
                    return Action.BACKWARD_NOROT
                elif state.x > goal_x and state.y > goal_y:
                    return Action.BACKWARD_CLK            
            elif state.heading in LEFT:
                if state.x < goal_x and state.y < goal_y:
                    return Action.BACKWARD_CCLK
                elif state.x == goal_x and state.y < goal_y:
                    return Action.FORWARD_CLK
                elif state.x > goal_x and state.y < goal_y:
                    return Action.FORWARD_CLK
                elif state.x < goal_x and state.y == goal_y:
                    return Action.BACKWARD_NOROT
                elif state.x > goal_x and state.y == goal_y:
                    return Action.FORWARD_NOROT
                elif state.x < goal_x and state.y > goal_y:
                    return Action.BACKWARD_CLK
                elif state.x == goal_x and state.y > goal_y:
                    return Action.FORWARD_CCLK
                elif state.x > goal_x and state.y > goal_y:
                    return Action.FORWARD_CCLK
            elif state.heading in RIGHT:
                if state.x < goal_x and state.y < goal_y:
                    return Action.FORWARD_CCLK
                elif state.x == goal_x and state.y < goal_y:
                    return Action.FORWARD_CCLK
                elif state.x > goal_x and state.y < goal_y:
                    return Action.BACKWARD_CLK
                elif state.x < goal_x and state.y == goal_y:
                    return Action.FORWARD_NOROT
                elif state.x > goal_x and state.y == goal_y:
                    return Action.BACKWARD_NOROT
                elif state.x < goal_x and state.y > goal_y:
                    return Action.FORWARD_CLK
                elif state.x == goal_x and state.y > goal_y:
                    return Action.FORWARD_CLK
                elif state.x > goal_x and state.y > goal_y:
                    return Action.BACKWARD_CCLK
            elif state.heading in DOWN:
                if state.x < goal_x and state.y < goal_y:
                    return Action.BACKWARD_CCLK
                elif state.x == goal_x and state.y < goal_y:
                    return Action.BACKWARD_NOROT
                elif state.x > goal_x and state.y < goal_y:
                    return Action.BACKWARD_CLK
                elif state.x < goal_x and state.y == goal_y:
                    return Action.FORWARD_CCLK
                elif state.x > goal_x and state.y == goal_y:
                    return Action.FORWARD_CLK
                elif state.x < goal_x and state.y > goal_y:
                    return Action.FORWARD_CCLK
                elif state.x == goal_x and state.y > goal_y:
                    return Action.FORWARD_NOROT
                elif state.x > goal_x and state.y > goal_y:
                    return Action.FORWARD_CLK 
        else:
            return Action.STAY;  

    def get_init_policy(self):
        # populate an init policy that moves closer to goal state
        init_policy = np.array([ [ [ self.action_to_take(self.stateAt(x, y, h), self.goal_state)
            for x in range(L) ]for y in range(W)] for h in headings])
        
        return init_policy

    def get_p(self, s, s_new, a):
        #100 percent chance to stay in current spot
        if a == Action.STAY:
            if s.x == s_new.x and s.y == s_new.y and s.heading == s_new.heading:
                return 1
            else:
                return 0

        #If robot moves
        else:
            #prerotation chances
            chance = np.array([self.robot.p_e, 1.0 - 2.0*self.robot.p_e, self.robot.p_e])
            head = np.array([(s.heading - 1) % 12, s.heading, (s.heading + 1) % 12])

            #get candidate states by x,y
            s_primes = np.zeros([3,3])
            for idx,h in enumerate(head):
                if h in UP:
                    if a in fw_actions and s.y < W-1:
                        s_primes[idx][:] = np.array([s.x, s.y+1, h])
                    elif a in bw_actions and s.y > 0:
                        s_primes[idx][:] = np.array([s.x, s.y-1, h])
                    else:
                        s_primes[idx][:] = np.array([s.x, s.y, h])
                elif h in LEFT:
                    if a in fw_actions and s.x > 0:
                        s_primes[idx][:] = np.array([s.x-1, s.y, h])
                    elif a in bw_actions and s.x < L-1:
                        s_primes[idx][:] = np.array([s.x+1, s.y, h])
                    else:
                        s_primes[idx][:] = np.array([s.x, s.y, h])
                elif h in RIGHT:
                    if a in fw_actions and s.x < L-1:
                        s_primes[idx][:] = np.array([s.x+1, s.y, h])
                    elif a in bw_actions and s.x > 0:
                        s_primes[idx][:] = np.array([s.x-1, s.y, h])
                    else:
                        s_primes[idx][:] = np.array([s.x, s.y, h])
                elif h in DOWN:
                    if a in fw_actions and s.y > 0:
                        s_primes[idx][:] = np.array([s.x, s.y-1, h])
                    elif a in bw_actions and s.y < W - 1:
                        s_primes[idx][:] = np.array([s.x, s.y+1, h])
                    else:
                        s_primes[idx][:] = np.array([s.x, s.y, h])

            #update candidate states with new heading if necessary     
            if(a in clk_actions):
                s_primes[:,2] += 1
            elif(a in cclk_actions):
                s_primes[:,2] += -1
            s_primes[:,2] %= 12

            #return nonzero chance if any candidates match the argument
            for idx,row in enumerate(s_primes):
                if s_new.x == row[0] and s_new.y == row[1] and s_new.heading == row[2]:
                    return chance[idx]
                else:
                    continue
        return 0

    def get_next_state(self, a):
        pmf = []
        pos = []
        for st in self.flattenStates():
            p = self.get_p(State(self.robot.x, self.robot.y, self.robot.heading, -1), st, a)

            if p > 0:
                pmf.append(p)
                pos.append(st)

        idx = np.random.choice(len(pmf), p=pmf);
        return pos[idx]

    def get_possible_next_states(self, state, policy):
        possible_states = []
        for st in self.flattenStates():
            
            p = self.get_p(state, st, policy[state.heading][state.y][state.x])

            if p > 0:
                possible_states.append((st, p))
        return possible_states

    def get_possible_states_from_action(self, state, action):
        possible_states = []
        for st in self.flattenStates():
            p = self.get_p(state, st, action)

            if p > 0:
                possible_states.append((st, p))
        return possible_states

    def get_reward_at(self, x, y, h):
        return self.stateAt(x,y,h).reward
    
    def get_reward_at(self, s_prime):
        return self.stateAt(s_prime.x,s_prime.y,s_prime.h).reward

    def policy_eval(self, policy, gamma, theta=10):
        V = np.zeros((len(headings), W, L))

        while True:
            start = time.time()
            delta = 0
            it = 0
            print("Begin evaluation...")
            for s in self.flattenStates():
                it = it + 1
                v = 0
                possible_states = self.get_possible_next_states(s, policy)
                for new_s, prob in possible_states:
                    v += prob*(new_s.reward+gamma*V[new_s.heading][new_s.y][new_s.x])
                delta = max(delta, np.abs(v - V[s.heading][s.y][s.x]))
                V[s.heading][s.y][s.x] = v
                # Stop evaluating once our value function change is below a threshold
            print("delta: ", delta)
            print("Evaluation {0:.2f} seconds".format(time.time()-start))
            if delta < theta:
                break
        return V

    def find_opt_policy(self, init_policy, gamma):
        policy = init_policy.copy()

        def one_step_look(state, V):
            A = np.zeros(len(actions))
            for idx,a in enumerate(actions):
                possible_states = self.get_possible_states_from_action(state, a)
                for (s_new, prob) in possible_states:
                    A[idx] += prob * (s_new.reward + gamma*V[s_new.heading][s_new.y][s_new.x])
            return A

        while True:
            start = time.time()
            V = self.policy_eval(policy, gamma);
            stable = True
            old_policy = policy.copy()
            printProgressBar(0, 100, prefix = 'Progress:', suffix = 'Complete', length = 50)
            for idx,st in enumerate(self.flattenStates()):
                action = policy[st.heading][st.y][st.x]
                action_values = one_step_look(st, V)
                opt_a = actions[np.argmax(action_values)]

                if(action != opt_a):
                    stable = False
                policy[st.heading][st.y][st.x] = opt_a
                printProgressBar(((idx+1)/432.0)*100, 100, prefix = 'Recalculating Progress:', suffix = 'Complete', length = 50)
            print("Recalculation {0:.2f} seconds".format(time.time()-start))
            if not stable:
                print("Number of policies changed {}".format(np.sum(np.not_equal(policy, old_policy))))
            if stable:
                return policy, V