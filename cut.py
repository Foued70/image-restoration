# -*- coding: utf-8 -*-

from sys import stdin
from collections import defaultdict
from Queue import Queue

import numpy as np
import mahotas as mh
import pylab

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
            #print "%r has %d neighbours" % (u, len(self.neighbour[u]))
            #print self.neighbour[u]
            for v in self.neighbour[u]:
                if self.cap[(u,v)] - self.flow[(u,v)] > 0 and parent[v] == -1:
                    parent[v] = u
                    m[v] = min(m[u], self.cap[(u,v)] - self.flow[(u,v)])
                    if v != sink:
                        q.put(v)
                    else:
                        return parent, m[sink]
    
        return parent, 0

    def min_cut(self, source, sink):
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
    
        print "Minimal cut found!"
    
        path, aug_flow = self.bfs(source, sink)
    
        A = set([v for v in path if v != -1 and v != -2])
        A.add(source)

        return A, set(self.neighbour.keys()).difference(A)

    def blocking_flow(level, u, source, sink, limit):
        if limit <= 0:
            return 0

        if node == sink:
            return limit

        throughput = 0
        for v in self.neighbour[u]:
            residual = self.cap[(u,v)] - self.flow[(u,v)]
            if level[v] == level[u] + 1 and residual > 0:
                aug = blocking_flow(level, v, source, sink,
                        min(limit - throughput, residual)
                        )

                throughput += aug
                flow[(u,v)] += aug
                flow[(v,u)] -= aug

        if throughput == 0:
            level[u] = -1

        return throughput

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

            self.blocking_flow(level, source, sink)
    
        path, aug_flow = self.bfs(source, sink)
    
        A = set([v for v in path if v != -1 and v != -2])
        A.add(source)

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

    A, B = network.min_cut_dinic(source, sink)

    print A, B

def f(u, v):
    return abs(u - v)**2

def Ei(img, p, u):
    return (f(131, img[p[0]][p[1]])
            - f(130, img[p[0]][p[1]])) * (1 - u)

def Eij(up, uq):
    return 1 * ((1 - 2 * uq) * up + uq)

def lena_test():

    img = mh.imread('img/lenaeye.bmp')
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

    A, B = network.min_cut('s', 't')

    A.remove('s')
    print A, B
    for x, y in A:
        img[w-1-x][y] = 100

    B.remove('t')
    for x, y in B:
        img[w-1-x][y] = 0

    pylab.imshow(img, cmap=pylab.gray())
    pylab.show()

simple_test()
#lena_test()

