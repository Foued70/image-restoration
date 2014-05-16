#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import sys
import argparse
from scipy import misc

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('ifile')
    parser.add_argument('ofile')
    parser.add_argument('-t', '--type',  default='normal')
    parser.add_argument('-m', '--mean',  type=int, default=0)
    parser.add_argument('-s', '--std',   type=int, default=10)
    parser.add_argument('-d', '--decay', type=int, default=10)

    args = parser.parse_args()

    i_img = misc.imread(args.ifile)

    if args.type == 'normal':
        o_img = i_img + np.random.normal(
                args.mean,
                args.std,
                i_img.shape)

    elif args.type == 'laplace':
        o_img = i_img + np.random.laplace(
                args.mean,
                args.decay,
                i_img.shape)

    o_img[o_img < 0]   = 0
    o_img[o_img > 255] = 255

    print np.max(o_img)
    print np.min(o_img)

    misc.imsave(args.ofile, o_img)

if __name__ == '__main__':
    main()

