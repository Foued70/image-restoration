# -*- coding: utf-8 -*-

from sys import stdin
from collections import defaultdict
from Queue import Queue

import numpy as np
import mahotas as mh
import matplotlib.pyplot as plt
import argparse

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
            self.neighbour[u] = []
        if v not in self.neighbour:
            self.neighbour[v] = []

        # FIXME: This is a linear lookup :-(
        if v not in self.neighbour[u]:
            self.neighbour[u].append(v)
        if u not in self.neighbour[v]:
            self.neighbour[v].append(u)

        if (u,v) not in self.cap:
            self.cap[(u,v)] = 0
        self.cap[(u,v)] += c

        if (v,u) not in self.cap:
            self.cap[(v,u)] = 0

        self.flow[(u,v)] = 0
        self.flow[(v,u)] = 0

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
                if self.cap[(u,v)] - self.flow[(u,v)] > 0 and parent[v] == -1:
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
            print sum(self.flow[(source,v)] for v in self.neighbour[source])
    
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

    def blocking_flow_nonrecursive(self, level, source, sink, limit):
        q = []
        q.append(source)

        visited = set()
        throughput = defaultset(lambda: 0)
        while len(q) > 0:
            u = q.pop()
            if u not in visited:
                visited.add(u)
                q.append(u)
                q.extend(self.neighbour[u])
            else:
                pass
                #postprocess(u)

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
        #print "Pushing %r from %r to %r" % (f, u, v)
        excess[u] -= f
        excess[v] += f
        self.flow[(u,v)] += f
        self.flow[(v,u)] -= f
        return f != 0

    def relabel(self, height, u):
        old = height[u]
        #print "Relabel %r, height = %r, neighbour = %r" % (u, height[u],
        #        self.neighbour[u])
        #print [height[v] for v in self.neighbour[u] \
        #        if self.cap[(u,v)] - self.flow[(u,v)] > 0 \
        #        ]
        height[u] = minimum((height[v] for v in self.neighbour[u] \
                if self.cap[(u,v)] - self.flow[(u,v)] > 0 \
                ), 0) + 1
        #print "%r == %r" % (height[u], old)
        return height[u] != old

    def min_cut_push_relabel(self, source, sink):
        excess = defaultdict(lambda: 0)
        height = defaultdict(lambda: 0)
        excess[source] = 1000000 # FIXME: Infinity
        height[source] = len(self.neighbour)

        # Initialization
        for v in self.neighbour[source]:
            self.flow[(source,v)] = self.cap[(source,v)]
            self.flow[(v,source)] = -self.cap[(source,v)]
            excess[v] += self.cap[(source,v)]

        old = True
        while old:
            old = False
            for u,v in self.cap.keys():
                if not excess[u] > 0:
                    continue
                if height[u] != height[v] + 1:
                    continue
                new = self.push(excess, u, v)
                old = old or new

            for u in self.neighbour.keys():
                cont = False
                if u == sink or u == source:
                    continue
                if not excess[u] > 0:
                    continue
                for v in self.neighbour[u]:
                    if self.cap[(u,v)] - self.flow[(u,v)] > 0:
                        if height[u] > height[v]:
                            cont = True
                            break
                if cont:
                    continue
                new = self.relabel(height, u)
                old = old or new
            #print "Flow:", self.flow
            #print "Excess:", excess
            #print "Height:", height
            #stdin.readline()

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

    #A, B = network.min_cut_dinic(source, sink)
    A, B = network.min_cut_push_relabel(source, sink)

    print A, B

def f(u, v):
    return abs(u - v)**2

def Ei(img, p, u):
    return (f(131, img[p[0]][p[1]])
            - f(130, img[p[0]][p[1]])) * (1 - u)

def Eij(up, uq):
    return 10 * ((1 - 2 * uq) * up + uq)

def segment(ifile, ofile):

    img = mh.imread(ifile)
    print "Loaded Lena"

    #pylab.imshow(img, cmap=pylab.gray())
    #pylab.show()

    network = FlowNetwork()
    print "Empty network created"

    w = len(img[0]) # x dimension
    h = len(img)    # y dimension

    A = Eij(0, 0)
    B = Eij(0, 1)
    C = Eij(1, 0)
    D = Eij(1, 1)

    for x in xrange(w):
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

    print "Network filled with edges, starting min-cut algorithm"

    A, B = network.min_cut_dinic('s', 't')

    print "Minimal cut found"

    A.remove('s')
    for x, y in A:
        img[x][y] = 100

    B.remove('t')
    for x, y in B:
        img[x][y] = 0

    #pylab.imshow(img, cmap=pylab.gray())
    #pylab.show()

    mh.imsave(ofile, img)

#simple_test()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ifile", help="path of input file")
    parser.add_argument("ofile", help="path of output file")
    args = parser.parse_args()

    segment(args.ifile, args.ofile)

main()

