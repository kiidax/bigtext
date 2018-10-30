import os
import unittest
import subprocess
from collections import Counter
import re
import ast
import logging

logging.basicConfig(level=logging.INFO)

class TestBigtext(unittest.TestCase):

    FILES = ['shakespeare.txt']

    @classmethod
    def tearDownClass(cls):
        os.unlink('result.txt')

    def test_help(self):
        x = exec_command()
        self.assertIn('List of commands', x)

    def test_version(self):
        x = exec_command('version')
        self.assertIn('Copyright (C)', x)

    def test_count_quick(self):
        for fname in self.FILES:
            res = exec_command('count %s' % fname)
            res = parse_triple(res)
            self.assertIn(fname, res)
            logging.info('%s %s %s' % (fname, 'EstLineCount', res[fname]['EstLineCount']))
            diff = abs(res[fname]['EstLineCount'] / count_lines(fname) - 1)
            self.assertLess(diff, .2)

    def test_count_full(self):
        for fname in self.FILES:
            res = exec_command('count -c %s' % fname)
            res = parse_triple(res)
            self.assertIn(fname, res)
            self.assertEqual(res[fname]['LineCount'], count_lines(fname))

    def test_vocab_full(self):
        for fname in self.FILES:
            res = exec_command('vocab %s -o result.txt' % fname)
            res = parse_triple(res)
            actual = read_vocab('result.txt')
            expected = get_vocab(fname)
            compare_dict(expected, actual)

    def test_sample_single_all(self):
        for fname in self.FILES:
            res = exec_command('sample %s -o result.txt' % fname)
            res = parse_triple(res)
            actual = read_sample('result.txt')
            expected = read_sample(fname)
            self.assertEqual(len(expected), len(actual))
            check_sample(expected, actual)

    def test_sample_single_rate(self):
        for fname in self.FILES:
            res = exec_command('sample %s -r 0.2 result.txt' % fname)
            res = parse_triple(res)
            actual = read_sample('result.txt')
            expected = read_sample(fname)
            actual_len = len(actual)
            expected_len = 0.2 * len(expected)
            logging.info("%d lines sampled, expected around %d lines.", actual_len, expected_len)
            diff = abs(actual_len / expected_len - 1)
            self.assertLess(diff, .2)
            check_sample(expected, actual)

    def test_sample_single_num(self):
        for fname in self.FILES:
            res = exec_command('sample %s -n 1000 result.txt' % fname)
            res = parse_triple(res)
            actual = read_sample('result.txt')
            expected = read_sample(fname)
            actual_len = len(actual)
            expected_len = 1000
            logging.info("%d lines sampled, expected around %d lines.", len(actual), expected_len)
            diff = abs(actual_len / expected_len - 1)
            self.assertLess(diff, .2)
            check_sample(expected, actual)

# Utility functions

bigtext_cmd = r'..\x64\Release\bigtext.exe'

def exec_command(args = []):
    if isinstance(args, str):
        args = args.split()
    return subprocess.getoutput(' '.join([bigtext_cmd] + args))

def parse_triple(text):
    res = {}
    for y in text.split('\n'):
        y = y.split('\t')
        if len(y) == 3:
            o = res.setdefault(y[0], {})
            o[y[1]] = ast.literal_eval(y[2])
    return res

def count_lines(fname):
    with open(fname, 'rb') as f:
        return len(f.readlines())

def get_vocab(fname):
    c = Counter()
    with open(fname, 'rb') as f:
        c.update(f.read()[3:].split())
    return c

def read_vocab(fname):
    prev_count = None
    num_pat = re.compile(b'^\\d+$')
    res = {}
    with open(fname, 'rb') as f:
        for x in f.readlines():
            x = x.rstrip(b'\r\n').split(b'\t')
            if len(x) != 2: raise ValueError()
            word, count = x
            if not num_pat.match(count): raise ValueError()
            count = int(count)
            if prev_count is not None:
                if prev_count < count: raise ValueError()
            prev_count = count
            if word in res: raise ValueError()
            res[word] = count
    return res

def compare_dict(d1, d2):
    if len(d1) != len(d2):
        pass #raise ValueError()
    for k, v in d1.items():
        if v != d2[k]: raise ValueError() 

def read_sample(fname):
    with open(fname, 'rb') as f:
        return f.readlines()

def check_sample(s1, s2):
    s1 = set(s1)
    s2 = set(s2)
    for x in s2:
        if x not in s1:
            raise ValueError()

if __name__ == '__main__':
    unittest.main()