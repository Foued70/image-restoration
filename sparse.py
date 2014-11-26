import numpy as np
import math
from sys import stdin
from matplotlib.patches import Ellipse
import matplotlib.pyplot as plt

def main(M):
    e = np.array([0, 1])
    f = np.array([1, 0])

    while True:
        e, f = (f, e - round(e.dot(M.dot(f)) / (f.dot(M.dot(f)))) * f)

        if e.dot(M.dot(e)) <= f.dot(M.dot(f)):
            break

    if e.dot(M.dot(f)) > 0:
        f = -f

    g = -e -f

    evals, evecs = np.linalg.eig(M)
    evecs = evecs.T
    print "Fo real yo"
    print evals, evecs

    plt.figure()
    ax = plt.gca()

    ellipse = Ellipse(xy=(0, 0), width=evals[1], height=evals[0], 
            edgecolor='r', fc='None', lw=2,
            angle=math.degrees(math.atan2(evecs[1][1], evecs[1][0])))

    ax.add_patch(ellipse)
    U, V = zip(*[e, f, g, -e, -f, -g])
    ax.quiver(
            [0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0],
            U, V,
            angles='xy',
            scale_units='xy',
            scale=1)

    ax.set_xlim([-3,3])
    ax.set_ylim([-3,3])

    plt.draw()
    plt.show(block=False)

if __name__ == "__main__":
    while True:
        ints = map(float, stdin.readline().split())

        u = np.array([ints[0], ints[1]])
        M = np.outer(u, u)
        print "First matrix: ", M

        print "Its eigens: ", np.linalg.eig(M)
        evals, evecs = np.linalg.eig(M)

        g = 20.0
        ev = np.array([3.0 / (1 + (evals[0] - evals[1])**2 / g), 3.0])
        #print ev

        M = np.matrix(evecs) * np.diagflat(ev) * np.matrix(evecs).T
        #print np.matrix(evecs) * np.diagflat(evals) * np.matrix(evecs).T
        print "new matrix: ", M
        print "Its eigens: ", np.linalg.eig(M)

        main(np.array(M))

