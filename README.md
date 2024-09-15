# Hyperthymesia

Hyperthymesia is a lightweight C-framework for memory allocations statistics,
that are available from inside the application.

It allows to preload `libhyperthymesia` library, which hooks
malloc, realloc, and free, and creates table `backtrace -> allocation stats`,
that are abailable from socket file to read, without application stop.

In the result, table looks like:
```
rkhapov@rkhapov-dev-vm:~/hyperthymesia/build$ nc -U /tmp/ht.sock
addr2line -e ./test_leak 0x12bb (null)
addr2line -e ./test_leak 0x12cf (null)
addr2line -e ./test_leak 0x12f9 (null)
addr2line -e /lib/x86_64-linux-gnu/libc.so.6 0x9ca94 (null)
	allocs = 100 free = 61 total_size = 3900

addr2line -e /lib/x86_64-linux-gnu/libc.so.6 0x851a5 _IO_file_doallocate
addr2line -e /lib/x86_64-linux-gnu/libc.so.6 0x95514 _IO_doallocbuf
addr2line -e /lib/x86_64-linux-gnu/libc.so.6 0x92f80 _IO_file_overflow
addr2line -e /lib/x86_64-linux-gnu/libc.so.6 0x93a9f _IO_file_xsputn
	allocs = 1 free = 0 total_size = 1024
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
- build your application with `-fno-omit-frame-pointer` (optional but highly recommended)
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

Not well-tested, but looks like allocation in app becomes `1-3` times slower for commonly used sizes.
My tests shows:
```
rkhapov@rkhapov-dev-vm:~/hyperthymesia/build$ ./run.py 
allocation for size 8 is slowed from 15.713 to 18.700 times
allocation for size 16 is slowed from 3.943 to 4.569 times
allocation for size 256 is slowed from 1.221 to 1.361 times
allocation for size 512 is slowed from 1.119 to 1.304 times
allocation for size 4096 is slowed from 0.959 to 1.025 times
allocation for size 16384 is slowed from 0.905 to 0.954 times

reallocation for size 8 is slowed from 13.9283 to 15.0428 times
reallocation for size 16 is slowed from 27.0603 to 29.5923 times
reallocation for size 256 is slowed from 1.5029 to 1.6514 times
reallocation for size 512 is slowed from 1.2725 to 1.3692 times
reallocation for size 4096 is slowed from 1.0259 to 1.0309 times
reallocation for size 16384 is slowed from 0.9872 to 0.9971 times
```
