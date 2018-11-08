import os
import unittest
import subprocess
from collections import Counter
import re
import ast
import logging

logging.basicConfig(level=logging.INFO)

# shakespeare.txt Texts from Shakespeare.
# test1.txt       20K normal lines
# test2.txt       20K normal lines but the last line without newline.
# test3.txt       1 normal line
# test4.txt       Empty file
# test5.txt       A file only with one newline.
# test6.txt       single very long lines.
# test7.txt       500 long lines, some of which are more then 4KB.

class TestBigtext(unittest.TestCase):

    FILES = ['shakespeare.txt'] + ['test%d.txt' % i for i in range(1, 8)]
    OUTPUT_FILES = ['result.txt', 'result2.txt']

    @classmethod
    def tearDownClass(cls):
        cls._remove_output()

    def test_help(self):
        self._run_command()
        self.assertIn('List of commands', self.command_result)

    def test_version(self):
        self._run_command('version')
        self.assertIn('Copyright (C)', self.command_result)

    def test_count_quick(self):
        for source_fname in self.FILES:
            self._run_command('count %s' % source_fname)
            self.assertFileIsCountedFrom(source_fname, False)

    def test_count_full(self):
        for source_fname in self.FILES:
            self._run_command('count -c %s' % source_fname)
            self.assertFileIsCountedFrom(source_fname, True)

    def test_vocab(self):
        for source_fname in self.FILES:
            self._run_command('vocab %s -o result.txt' % source_fname)
            self.assertFileIsVocabOf('result.txt', source_fname)

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
            if source_fname == 'test7.txt':
                self.assertFileIsSampledFrom('result2.txt', source_fname, 0.0, False)
            else:
                self.assertFileIsSampledFrom('result2.txt', source_fname, 0.2, False)

    def test_sample_double_num_all(self):
        for source_fname in self.FILES:
            self._run_command('sample %s -n 1000 result.txt -o result2.txt' % source_fname)
            self.assertFileIsSampledFrom('result.txt', source_fname, 1000, False)
            self.assertFileIsSampledFrom('result2.txt', source_fname, -1000, False)

    def test_shuffle_single_all_no_crash(self):
        for opt in ['-s ', '-s -c 3 ']:
            for source_fname in self.FILES:
                self._run_command('sample %s%s -o result.txt' % (opt, source_fname))
                self.assertTrue(os.path.exists('result.txt'))
                #self.assertFileIsSampledFrom('result.txt', source_fname, 0, True)

    def test_shuffle_single_all(self):
        for opt in ['-s ', '-s -c 3 ']:
            for source_fname in ['shakespeare.txt', 'test1.txt', 'test3.txt', 'test6.txt', 'test7.txt']:
                self._run_command('sample %s%s -o result.txt' % (opt, source_fname))
                self.assertFileIsSampledFrom('result.txt', source_fname, 0, True)
                self.assertFileIsShuffledFrom('result.txt', source_fname)

    def test_shuffle_single_num_rate(self):
        for opt in ['-s ', '-s -c 3 ']:
            for source_fname in ['shakespeare.txt', 'test1.txt', 'test3.txt', 'test6.txt', 'test7.txt']:
                self._run_command('sample %s%s -n 100 result.txt -r 0.2 result2.txt' % (opt, source_fname))
                self.assertFileIsSampledFrom('result.txt', source_fname, 100, True)
                self.assertFileIsSampledFrom('result2.txt', source_fname, 0.2, False)

    def assertFileIsCountedFrom(self, source_fname, exact):
        res = self.parsed_result
        self.assertIn(source_fname, res)
        if exact:
            actual_len = res[source_fname]['LineCount']
        else:
            actual_len = res[source_fname]['EstLineCount']
        expected_len = count_lines(source_fname)
        if exact or expected_len == 0:
            logging.info("%d lines counted, expected exact %d lines.", actual_len, expected_len)
        else:
            logging.info("%d lines counted, expected around %d lines.", actual_len, expected_len)
            diff = abs(actual_len / expected_len - 1)
            self.assertLess(diff, .2)

    def assertFileIsVocabOf(self, actual_fname, source_fname):
        actual = read_vocab(actual_fname)
        expected = get_vocab(source_fname)
        actual_len = len(actual)
        expected_len = len(expected)
        logging.info("%d words counted, expected %d words.", actual_len, expected_len)
        if len(actual) != len(expected):
            pass #raise ValueError()
        for k, v in actual.items():
            if k not in expected: raise AssertionError('%s is not found in the source file.' % k[:10])
            if v != expected[k]: raise AssertionError('Count of %s is %d in %s. Expected %d.' % (k[:10], v, source_fname, expected[k]))

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
                if expected_len > source_len:
                    logging.info('Expected number of lines %d is more than number of lines in the source %d.' % (expected_len, source_len))
                    expected_len = source_len
            elif rate_or_number < 0:
                expected_len = source_len + rate_or_number
                if expected_len < 0:
                    logging.info('There are no remaining lines.')
                    expected_len = 0
            else:
                expected_len = source_len
        else:
            raise ValueError()
        if expected_len == 0:
            logging.info("%d lines sampled, the source contains no line.", actual_len)
            self.assertEqual(0, actual_len)
        elif source_len == 1:
            logging.info("%d lines sampled, the source only contains single line.", actual_len)
        elif exact:
            logging.info("%d lines sampled, expected exact %d lines.", actual_len, expected_len)
            self.assertEqual(expected_len, actual_len)
        else:
            logging.info("%d lines sampled, expected around %d lines.", actual_len, expected_len)
            diff = abs(actual_len / expected_len - 1)
            self.assertLess(diff, .2)
        #self.assertLessEqual(actual, source)
        actual = set(actual)
        source = set(source)
        if not (actual <= source):
            #print(list(source)[0])
            x = list(actual - source)[0]
            raise AssertionError('%s is not included in the source.' % x[:100])

        if source_fname.startswith('test') and os.path.getsize(source_fname) > 2:
            self.assertSampleLinesAreCorrect(actual_fname)

    def assertSampleLinesAreCorrect(self, actual_fname):
        with open(actual_fname, 'r') as f:
            logging.info("Veryfying lines are correct.")
            for i, line in enumerate(f):
                try:
                    s = line.rstrip('\r\n').split('\t')
                    s = [int(x) for x in s]
                except:
                    raise AssertionError("Exception at line %d" % (i + 1))
                if s[0] != sum(s[1:]):
                    raise AssertionError("Wrong data at line %d" % (i + 1))

    def assertFileIsShuffledFrom(self, actual_fname, source_fname):
        logging.info("Veryfying files contain identical set of lines.")
        actual = read_sample(actual_fname)
        source = read_sample(source_fname)
        actual_len = len(actual)
        source_len = len(source)
        self.assertEqual(source_len, actual_len)
        actual.sort()
        source.sort()
        self.assertSequenceEqual(source, actual)

    def _run_command(self, args=[]):
        self._remove_output()
        logging.info("Running command with `%s'.", args)
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
        x = f.read()
    # Remove BOM
    if x.startswith(b'\xef\xbb\xbf'):
        x = x[3:]
    c.update(x.split())
    return c

def read_vocab(fname):
    prev_count = None
    num_pat = re.compile(b'^\\d+$')
    res = {}
    with open(fname, 'rb') as f:
        for x in f.readlines():
            x = x.rstrip(b'\r\n').split(b'\t')
            if len(x) != 2: raise ValueError(x)
            word, count = x
            if not num_pat.match(count): raise ValueError(x)
            count = int(count)
            if prev_count is not None:
                if prev_count < count: raise ValueError(x)
            prev_count = count
            if word in res: raise ValueError(x)
            res[word] = count
    return res

def read_sample(fname):
    with open(fname, 'rb') as f:
        return f.readlines()

if __name__ == '__main__':
    unittest.main()
