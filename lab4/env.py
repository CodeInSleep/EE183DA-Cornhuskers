import numpy as np
from matplotlib.ticker import AutoMinorLocator
from rrt import *
import matplotlib.pyplot as plt
import random
from treelib import Node, Tree
from math import *

delta = 3 # Distance the robot can run for 1sec.

class CState:
    def __init__(self, x, y, theta, clear=1):
        self.x = x
        self.y = y
        self.theta = theta
        self.clear = clear

class Obstacle:
    def __init__(self,x,y,w,l):
        self.x = x
        self.y = y
        self.w = w
        self.l = l

class Robot:
    def __init__(self,x,y,theta, radius = 5):
        self.x = x
        self.y = y
        self.theta = theta
        self.radius = radius

class Environment:
    def __init__(self, Nx, Ny, Nt, robot, goal = (0,0), obstacles=None):
        """
        Obstacles format:
        rectangular: x,y,w,l
        """
        x = np.linspace(0,Nx-1,num=Nx)
        y = np.linspace(0,Ny-1,num=Ny)
        theta = np.linspace(0,(360-360/Nt),num=Nt)
        self.Nx = Nx
        self.Ny = Ny
        self.Nt = Nt
        self.robot = robot
        self.C = set()
        self.goal = goal

        openV = []
        closedV = []
        for xx in range(len(x)):
            for yy in range(len(y)):
                for tt in range(len(theta)):
                    if obstacles is None:
                        openV.append((xx,yy,tt,1))
                    else:
                        for o in obstacles:
                            if (xx < o.x or xx > (o.x+o.w)) or (yy < o.y or yy > (o.y+o.l)):
                                if((xx,yy,tt,1) not in openV):
                                    openV.append((xx,yy,tt,1))
                            else:
                                if((xx,yy,tt,0) not in closedV):
                                    closedV.append((xx,yy,tt,0))
        for st in (openV+closedV):
            self.C.add(CState(st[0],st[1],st[2],st[3]))
        
        self.V = set()
        self.V.add(self.stateAt(robot.x, robot.y, robot.theta))

    def show(self, route = None):
        pts = []
        for v in self.C:
            if (v.clear == 0) and ((v.x,v.y) not in pts):
                pts.append((v.x,v.y))
        arena = np.ones((self.Ny,self.Nx))
        for pt in pts:
            arena[pt[1],pt[0]] = 0
        for st in self.V:
            arena[st.y,st.x] = 2
        fig = plt.figure();
        plt.xlim((0,self.Nx))
        plt.ylim((0,self.Ny))
        ax = plt.gca();
        ax.set_xticks(np.arange(0.5, self.Nx+.5, 1));
        ax.set_yticks(np.arange(0.5, self.Ny+.5, 1));
        ax.set_xticklabels(np.arange(0, self.Nx, 1));
        ax.set_yticklabels(np.arange(0, self.Ny, 1));
        ax.pcolor(arena, edgecolors='k', linestyle= 'dashed', linewidths=0.2, cmap='RdYlGn', vmin=0.0, vmax=3.0)
        minor_locator = AutoMinorLocator(2)
        plt.gca().xaxis.set_minor_locator(minor_locator)
        plt.gca().yaxis.set_minor_locator(minor_locator)
        plt.grid(which='minor')
        plt.scatter(self.robot.x+.5, self.robot.y+.5, c='black')
        plt.scatter(self.goal[0]+.5, self.goal[1]+.5, c='green')

        if route is not None:
            for line in route:
                if line:
                    plt.plot([line[0].x+0.5, line[1].x+0.5], [line[0].y+0.5, line[1].y+0.5])

        plt.show()

    def step_from_to(self, from_state, to_state):
        """
        2.2c
        This func makes the robot step 1 sec to target state from initial state
        Args:
        p1: initial state
        p2: target state (with any heading because only when physically turning we use theta)
        Returns:
        p: actual state ended with
        """
        if dist(from_state,to_state) < delta:
            return to_state
        theta = atan2(to_state.y-from_state.y,to_state.x-from_state.x)
        next_state = CState(from_state.x + delta*cos(theta), from_state.y + delta*sin(theta), 0, 0)
        # check for available closest available state
        closest_next_states = nearestNeighbors(self.C-self.V, next_state)

        return np.random.choice(closest_next_states)

    def sampleState(self):
        def get_random_idx(lim):
            return ceil(lim*random_sample())

        rand_state = random.sample(self.C - self.V, 1)[0]
        return rand_state

    def expandTree(self):
        rand_state = self.sampleState()
        
        NNs = nearestNeighbors(self.V, rand_state)
        rand_nn = np.random.choice(NNs)
        next_step = self.step_from_to(rand_nn, rand_state)
        if next_step.clear:
            self.V.add(next_step)
            return (rand_nn, next_step)

        return None

    def stateAt(self,x,y,theta):
        for st in self.C:
            if st.x == x and st.y == y and st.theta == theta:
                return st     

if __name__ == "__main__":
    can = Obstacle(10,10,5,5)
    final = (5,5)
    obs = [can]
    robot = Robot(20,20,10)
    env = Environment(40,60,12, robot, goal=final, obstacles=obs)

    route = []
    for i in range(100):
        route.append(env.expandTree())
    print(route)
    
    env.show(route=route)
    
