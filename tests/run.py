#!/usr/bin/env python3

import subprocess
import itertools
import os

RUN_COUNT = 5


def collect_test(mode, is_hooked):
    result = {}

    for _ in range(RUN_COUNT):
        env = dict(os.environ)
        if is_hooked:
            env['LD_PRELOAD'] = './libhyperthymesia.so'

        out = subprocess.check_output(['./benchmark', mode], env=env)
        for line in out.decode('utf-8').split('\n'):
            if len(line) == 0:
                continue
            k, v = list(map(int, line.split(' ')))
            if k not in result:
                result[k] = list()
            result[k].append(int(v))

    return result


def main():
    pure = collect_test('malloc', False)
    hooked = collect_test('malloc', True)

    for k in pure:
        p = pure[k]
        p.sort()

        h = hooked[k]
        h.sort()

        m1 = min(map(lambda x: x[0] / x[1], itertools.product(h, p)))
        m2 = max(map(lambda x: x[0] / x[1], itertools.product(h, p)))

        print('allocation for size {} is slowed from {:.3f} to {:.3f} times'.format(
            k, m1, m2))

    pure = collect_test('realloc', False)
    hooked = collect_test('realloc', True)

    for k in pure:
        p = pure[k]
        p.sort()

        h = hooked[k]
        h.sort()

        p = p[1:-1]
        h = h[1:-1]

        m1 = min(map(lambda x: x[0] / x[1], itertools.product(h, p)))
        m2 = max(map(lambda x: x[0] / x[1], itertools.product(h, p)))

        print('reallocation for size {} is slowed from {:.4f} to {:.4f} times'.format(
            k, m1, m2))


if __name__ == '__main__':
    main()
