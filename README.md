# Hyperthymesia

Hyperthymesia is a lightweight C-framework for memory allocations statistics.

It allows to preload `libhyperthymesia` library, which hooks
malloc, realloc, and free, and creates table `backtrace -> allocation stats`,
that are abailable from socket file to read, without application stop.

In the result, table looks like:
```
0x851a5 (0x7d54202851a5) | _IO_file_doallocate of /lib/x86_64-linux-gnu/libc.so.6
0x95514 (0x7d5420295514) | _IO_doallocbuf of /lib/x86_64-linux-gnu/libc.so.6
0x92f80 (0x7d5420292f80) | _IO_file_overflow of /lib/x86_64-linux-gnu/libc.so.6
	allocs = 1 free = 0 total_size = 1024 bytes

0x280e (0x7d542061080e) | calloc of ./libhyperthymesia.so
0x145ac (0x7d542062f5ac) | _dl_allocate_tls of /lib64/ld-linux-x86-64.so.2
0x9d607 (0x7d542029d607) | pthread_create of /lib/x86_64-linux-gnu/libc.so.6
	allocs = 2 free = 0 total_size = 608 bytes

0x15a5 (0x5d71b4a955a5) | (null) of ./test_leak
0x15b9 (0x5d71b4a955b9) | (null) of ./test_leak
0x15ed (0x5d71b4a955ed) | (null) of ./test_leak
	allocs = 22 free = 22 total_size = 0 bytes

0x1498 (0x5d71b4a95498) | (null) of ./test_leak
0x14ac (0x5d71b4a954ac) | (null) of ./test_leak
0x14e0 (0x5d71b4a954e0) | (null) of ./test_leak
	allocs = 22 free = 15 total_size = 700 bytes
```

## How to use

Build libhyperthymesia:
```
make build_release
or
make build_dbg
```

And then, if you want to collect statistics of allocations for the program,
you must do:
- build your application with `-fno-omit-frame-pointer` and `-fno-optimize-sibling-calls` (optional but highly recommended)
- inside the app, you should call the `ht_register_stack_location_hint` function, to make stack location hints (see benchmark or test_leak example)
- run your app with smth like `HT_SOCKET=/tmp/ht.sock LD_PRELOAD=<full path to libhyperthymesia.so> ./your_app`
- obtain allocation stat with `nc -U /tmp/ht.sock`

## Example
Build an example with: `make build_dbg`
Test it with:
```bash
cd build
HT_SOCKET=/tmp/ht.sock LD_PRELOAD=$(pwd)/libhyperthymesia.so ./test_leak
...

nc -U /tmp/ht.sock
```

## Performance

In my machine the simple test gives the next results:
```
allocation for size 8 is slowed from 15.808 to 18.920 times
allocation for size 16 is slowed from 4.436 to 4.971 times
allocation for size 256 is slowed from 1.243 to 1.422 times
allocation for size 512 is slowed from 1.070 to 1.239 times
allocation for size 4096 is slowed from 0.919 to 0.998 times
allocation for size 16384 is slowed from 0.880 to 0.938 times

reallocation for size 8 is slowed from 22.4573 to 24.4448 times
reallocation for size 16 is slowed from 15.5254 to 18.2348 times
reallocation for size 256 is slowed from 1.6510 to 1.6955 times
reallocation for size 512 is slowed from 1.2874 to 1.3908 times
reallocation for size 4096 is slowed from 1.0283 to 1.0583 times
reallocation for size 16384 is slowed from 0.9949 to 1.0043 times
```

