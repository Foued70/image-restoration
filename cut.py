# -*- coding: utf-8 -*-

from sys import stdin
from collections import defaultdict, deque
from Queue import Queue

import numpy as np
import mahotas as mh
import matplotlib.pyplot as plt
import argparse
import time

def minimum(iterable, default=0):
    try:
        return min(iterable)
    except ValueError:
        return default

class FlowNetwork(object):
    def __init__(self):
        self.neighbour = {}
        self.cap       = {}
        self.flow      = {}

    def add_edge(self, u, v, c):
        if u not in self.neighbour:
            self.neighbour[u] = set()
        if v not in self.neighbour:
            self.neighbour[v] = set()

        self.neighbour[u].add(v)
        self.neighbour[v].add(u)

        if (u,v) not in self.cap:
            self.cap[(u,v)] = 0
        self.cap[(u,v)] += c

        if (v,u) not in self.cap:
            self.cap[(v,u)] = 0

        self.flow[(u,v)] = 0
        self.flow[(v,u)] = 0

    def create_adjacency_lists(self):
        for u in self.neighbour.keys():
            self.neighbour[u] = list(self.neighbour[u])

    def bfs(self, source, sink):
        nodes = len(self.neighbour)

        parent = defaultdict(lambda: -1)
        parent[source] = -2
    
        m = defaultdict(lambda: 10000000) # FIXME: infinity
    
        q = Queue()
    
        q.put(source)
        while not q.empty():
            u = q.get()
            for v in self.neighbour[u]:
                if self.cap[(u,v)] - self.flow[(u,v)] > 0 \
                        and parent[v] == -1:
                    parent[v] = u
                    m[v] = min(m[u], self.cap[(u,v)] - self.flow[(u,v)])
                    if v != sink:
                        q.put(v)
                    else:
                        return parent, m[sink]
    
        return parent, 0

    def min_cut_edmonds_karp(self, source, sink):
        nodes = len(self.neighbour)
    
        while True:
            path, aug_flow = self.bfs(source, sink)
    
            if aug_flow == 0:
                break
    
            v = sink
            while v != source:
                u = path[v]
    
                self.flow[(u,v)] += aug_flow
                self.flow[(v,u)] -= aug_flow
    
                v = u
    
        path, aug_flow = self.bfs(source, sink)
    
        A = set([v for v in path if v != -1 and v != -2])
        A.add(source)

        return A, set(self.neighbour.keys()).difference(A)

    def blocking_flow(self, level, u, source, sink, limit):
        if limit <= 0:
            return 0

        if u == sink:
            return limit

        throughput = 0
        for v in self.neighbour[u]:
            residual = self.cap[(u,v)] - self.flow[(u,v)]
            if level[v] == level[u] + 1 and residual > 0:
                aug = self.blocking_flow(level, v, source, sink,
                        min(limit - throughput, residual)
                        )

                throughput += aug
                self.flow[(u,v)] += aug
                self.flow[(v,u)] -= aug

        if throughput == 0:
            level[u] = -1

        return throughput

    def dfs(self, level, source, sink):
        print "Starting DFS"
        parent = defaultdict(lambda: -1)
        parent[source] = -2
    
        m = defaultdict(lambda: 0)
        m[source] = 100000000 # FIXME: infinity

        q = []
        q.append(source)

        sink_neighbours = []

        while len(q) > 0:
            u = q.pop()
            for v in self.neighbour[u]:
                residual = self.cap[(u,v)] - self.flow[(u,v)]
                if level[v] == level[u] + 1 and residual > 0:
                    if v == sink:
                        sink_neighbours.append(u)
                        continue

                    parent[v] = u
                    m[v] += min(m[u], self.cap[(u,v)] - self.flow[(u,v)])

                    q.append(v)
    
        return parent, 0

    def min_cut_dinic(self, source, sink):
        while True:
            level = defaultdict(lambda: -1)
            level[source] = 0

            nodes = len(self.neighbour)

            q = Queue()
    
            q.put(source)
            while not q.empty():
                u = q.get()
                for v in self.neighbour[u]:
                    if self.cap[(u,v)] - self.flow[(u,v)] > 0 and level[v] == -1:
                        level[v] = level[u] + 1
                        if v != sink:
                            q.put(v)

            if level[sink] == -1:
                break

            self.blocking_flow(level, source, source, sink, 1000000000)
    
        path, aug_flow = self.bfs(source, sink)
    
        A = set([v for v in path if v != -1 and v != -2])
        A.add(source)

        return A, set(self.neighbour.keys()).difference(A)

    def push(self, excess, u, v):
        f = min(excess[u], self.cap[(u,v)] - self.flow[(u,v)])
        excess[u] -= f
        excess[v] += f
        self.flow[(u,v)] += f
        self.flow[(v,u)] -= f

    def relabel(self, height, u):
        height[u] = minimum((height[v] for v in self.neighbour[u] \
                if self.cap[(u,v)] - self.flow[(u,v)] > 0 \
                ), 0) + 1

    def global_relabel(self, height, source, sink):
        print "Doing global relabeling"
        q = Queue()
    
        q.put((sink, 0))
        visited = set()
        while not q.empty():
            # NOTE: Reverse BFS
            v, h = q.get()
            visited.add(v)
            h += 1
            for u in self.neighbour[v]:
                if u in visited:
                    continue
                if self.cap[(u,v)] - self.flow[(u,v)] > 0:
                    q.put((u, h))
                    height[u] = h

    def min_cut_push_relabel(self, source, sink):
        excess = defaultdict(lambda: 0)
        height = defaultdict(lambda: 0)
        current = defaultdict(lambda: 0)
        excess[source] = 1000000

        # FIFO implementation
        q = deque()

        # Source is the only active node
        q.append(source)

        self.global_relabel(height, source, sink)

        print height
        stdin.readline()

        while len(q) > 0:
            u = q[0]
            print "At %r" % u

            for v in self.neighbour[u]:
                if self.cap[(u,v)] - self.flow[(u,v)] > 0:
                    if height[u] == height[v] + 1:
                        self.push(excess, u, v)
                        if v != sink and v != source:
                            q.append(v)

                if excess[u] == 0:
                    break

            current[u] += i

            if v == self.neighbour[u][-1]:
                current[u] = 0
                self.relabel(height, u)
            if excess[u] == 0:
                q.popleft()

        print "Flow done, finding cut"
        path, aug_flow = self.bfs(source, sink)
    
        A = set([v for v in path if v != -1 and v != -2])
        A.add(source)

        #print self.cap
        #print self.flow
        #print excess
        #print height
        return A, set(self.neighbour.keys()).difference(A)

def simple_test():

    arr = np.array([
        [0, 10, 10,  0],
        [0,  0,  1,  8],
        [0,  0,  0, 11],
        [0,  0,  0,  0]
        ])

    nodes = len(arr)

    source = 0
    sink   = 3

    network = FlowNetwork()

    for i in xrange(nodes):
        for j in xrange(nodes):
            if arr[i][j] != 0:
                network.add_edge(i, j, arr[i][j])

    network.create_adjacency_lists()
    #A, B = network.min_cut_dinic(source, sink)
    A, B = network.min_cut_push_relabel(source, sink)

    print A, B

def f(u, v):
    return abs(u - v)**2

def Ei(img, p, u):
    return (f(131, img[p[0]][p[1]])
            - f(130, img[p[0]][p[1]])) * (1 - u)

def Eij(up, uq):
    return 20 * ((1 - 2 * uq) * up + uq)

def segment(ifile, ofile):

    img = mh.imread(ifile)
    print "Loaded Lena"

    start = time.clock()
    network = FlowNetwork()
    print "Empty network created"

    w = len(img[0]) # x dimension
    h = len(img)    # y dimension

    A = Eij(0, 0)
    B = Eij(0, 1)
    C = Eij(1, 0)
    D = Eij(1, 1)

    for x in xrange(w):
        #print "Line: %d" % x
        for y in xrange(h):
            e0 = Ei(img, (x, y), 0)
            e1 = Ei(img, (x, y), 1)
            if e0 < e1:
                network.add_edge('s', (x, y), e1 - e0)
            else:
                network.add_edge((x, y), 't', e0 - e1)

            if x + 1 < w:
                if C - A > 0:
                    network.add_edge('s', (x, y), C - A)
                else:
                    network.add_edge((x, y), 't', C - A)

                if C - D > 0:
                    network.add_edge('s', (x+1, y), C - D)
                else:
                    network.add_edge((x+1, y), 't', C - D)

                network.add_edge((x, y), (x+1, y), B + C - A - D)

            if y + 1 < h:
                if C - A > 0:
                    network.add_edge('s', (x, y), C - A)
                else:
                    network.add_edge((x, y), 't', C - A)

                if C - D > 0:
                    network.add_edge('s', (x, y+1), C - D)
                else:
                    network.add_edge((x, y+1), 't', C - D)

                network.add_edge((x, y), (x, y+1), B + C - A - D)

    network.create_adjacency_lists()
    end = time.clock()
    tot_time = end - start
    print "Network created: %f" % (end - start)
    print "Starting min-cut algorithm"

    start = time.clock()
    #for u in network.neighbour.keys():
    #    for v in network.neighbour[u]:
    #        print "%r: %d" % (v, network.cap[(u,v)]),
    #    print

    #A, B = network.min_cut_edmonds_karp('s', 't')
    #A, B = network.min_cut_dinic('s', 't')
    A, B = network.min_cut_push_relabel('s', 't')

    end = time.clock()
    tot_time += end - start
    print "Minimal cut found: %f" % (end - start)

    print "Min cut took %f%% of the time" \
            % ((end - start) * 100.0 / tot_time)

    A.remove('s')
    for x, y in A:
        img[x][y] = 255

    B.remove('t')
    for x, y in B:
        img[x][y] = 0

    mh.imsave(ofile, img)

simple_test()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ifile", help="path of input file")
    parser.add_argument("ofile", help="path of output file")
    args = parser.parse_args()

    segment(args.ifile, args.ofile)

main()

