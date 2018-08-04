# -*- coding: utf-8 -*-

import sys
import random

def generate():
    with open('test.txt', 'w') as f:
        for i in range(20000):
            n = random.randrange(1, 11)
            s = [random.randrange(100) for i in range(n)]
            s = [sum(s)] + s
            s = [str(x) for x in s]
            f.write('\t'.join(s) + '\n')

def verify():
    i = 1
    with open('result.txt', 'r') as f:
        for line in f:
            try:
                s = line.rstrip('\r\n').split('\t')
                s = [int(x) for x in s]
            except:
                raise Exception("Exception at line %d" % i)
            if s[0] != sum(s[1:]):
                raise Exception("Wrong data at line %d" % i)

            i += 1

if __name__ == '__main__':
    c = sys.argv[1]
    if c == 'generate':
        generate()
    elif c == 'verify':
        verify()
    else:
        raise Exception("Unknown command")