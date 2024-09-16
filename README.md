# Hyperthymesia

Hyperthymesia is a lightweight C-framework for memory allocations statistics.

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

First:
```
sudo apt-get install libunwind-dev
```

And then build libhyperthymesia:
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

TDB
