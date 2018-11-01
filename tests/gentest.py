# -*- coding: utf-8 -*-

import sys
import random

def makerandomline(n = None):
    if n is None:
        n = random.randrange(1, 11)
    s = [random.randrange(100) for i in range(n)]
    s = [sum(s)] + s
    s = [str(x) for x in s]
    return ('\t'.join(s) + '\n').encode('utf-8')

def generate():
    with open('test1.txt', 'wb') as f:
        for _ in range(20000):
            line = makerandomline()
            f.write(line)

def generatemore():
    with open('test2.txt', 'wb') as f:
        for _ in range(19999):
            line = makerandomline()
            f.write(line)
        line = makerandomline()
        f.write(line[:-1])

    with open('test3.txt', 'wb') as f:
        line = makerandomline(2000)
        f.write(line)

    with open('test4.txt', 'wb') as f:
        pass

    with open('test5.txt', 'wb') as f:
        f.write(b'\n')

    with open('test6.txt', 'wb') as f:
        line = makerandomline(2000000)
        f.write(line)

    with open('test7.txt', 'wb') as f:
        for _ in range(500):
            line = makerandomline(3000)
            f.write(line)

if __name__ == '__main__':
    generate()
    generatemore()