import subprocess
import os

RUN_COUNT = 30


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

        print('allocation for size {} is slowed from {:.3f} to {:.3f} times'.format(
            k, h[1] / p[1], h[-2] / p[-2]))


if __name__ == '__main__':
    main()
