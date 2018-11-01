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
    OUTPUT_FILES = ['result.txt', 'result2.txt']

    @classmethod
    def tearDownClass(cls):
        cls._remove_output()

    def setUp(self):
        self._remove_output()

    def test_help(self):
        self._run_command()
        self.assertIn('List of commands', self.command_result)

    def test_version(self):
        self._run_command('version')
        self.assertIn('Copyright (C)', self.command_result)

    def test_count_quick(self):
        for source_fname in self.FILES:
            self._run_command('count %s' % source_fname)
            self.assertIn(source_fname, self.parsed_result)
            res = self.parsed_result
            logging.info('%s %s %s' % (source_fname, 'EstLineCount', res[source_fname]['EstLineCount']))
            diff = abs(res[source_fname]['EstLineCount'] / count_lines(source_fname) - 1)
            self.assertLess(diff, .2)

    def test_count_full(self):
        for source_fname in self.FILES:
            self._run_command('count -c %s' % source_fname)
            self.assertIn(source_fname, self.parsed_result)
            res = self.parsed_result
            self.assertEqual(res[source_fname]['LineCount'], count_lines(source_fname))

    def test_vocab_full(self):
        for source_fname in self.FILES:
            self._run_command('vocab %s -o result.txt' % source_fname)
            actual = read_vocab('result.txt')
            expected = get_vocab(source_fname)
            compare_dict(expected, actual)

    def test_sample_single_all(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -o result.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 0, True)

    def test_sample_single_rate(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -r 0.2 result.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 0.2, False)

    def test_sample_single_num(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -n 1000 result.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 1000, False)

    def test_sample_double_num_rate(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -n 1000 result.txt -r 0.2 result2.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 1000, False)
            self.assertFileIsSampledFrom('result2.txt', source_fname, 0.2, False)

    def test_sample_double_num_all(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -n 1000 result.txt -o result2.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 1000, False)
            self.assertFileIsSampledFrom('result2.txt', source_fname, -1000, False)

    def assertFileIsSampledFrom(self, actual_fname, source_fname, rate_or_number, exact):
        actual = read_sample(actual_fname)
        source = read_sample(source_fname)
        actual_len = len(actual)
        source_len = len(source)
        if isinstance(rate_or_number, float):
            expected_len = rate_or_number * source_len
        elif isinstance(rate_or_number, int):
            if rate_or_number > 0:
                expected_len = rate_or_number
            elif rate_or_number < 0:
                expected_len = source_len + rate_or_number
            else:
                expected_len = source_len
        else:
            raise ValueError()
        if exact:
            logging.info("%d lines sampled, expected exact %d lines.", len(actual), expected_len)
            self.assertEqual(expected_len, actual_len)
        else:
            logging.info("%d lines sampled, expected around %d lines.", len(actual), expected_len)
            diff = abs(actual_len / expected_len - 1)
            self.assertLess(diff, .2)
        #self.assertLessEqual(actual, source)
        actual = set(actual)
        source = set(source)
        if not (actual <= source):
            print(list(source)[0])
            x = list(actual - source)[0]
            raise AssertionError('%s is not included in the source.' % x)

    def _run_command(self, args=[]):
        self.command_result = exec_command(args)
        self.parsed_result = parse_triple(self.command_result)

    @classmethod
    def _remove_output(cls):
        for output_file in cls.OUTPUT_FILES:
            if os.path.exists(output_file):
                os.unlink(output_file)


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

if __name__ == '__main__':
    unittest.main()