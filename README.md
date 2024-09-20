# Hyperthymesia

Hyperthymesia is a lightweight C-framework for memory allocations statistics.

It allows to preload `libhyperthymesia` library, which hooks
malloc, realloc, and free, and creates table `backtrace -> allocation stats`,
that are abailable from socket file to read, without application stop.

In the result, table looks like:
```
{
  "allocs": [
    {
      "bt": [
        "0x851a5 (0x71e5098851a5) | _IO_file_doallocate of /lib/x86_64-linux-gnu/libc.so.6",
        "0x95514 (0x71e509895514) | _IO_doallocbuf of /lib/x86_64-linux-gnu/libc.so.6",
        "0x92f80 (0x71e509892f80) | _IO_file_overflow of /lib/x86_64-linux-gnu/libc.so.6"
      ],
      "allocs": 1,
      "free": 0,
      "size_human": "1024 bytes",
      "size": 1024
    },
    {
      "bt": [
        "0x1495 (0x5faa1e929495) | (null) of ./test_leak",
        "0x14a9 (0x5faa1e9294a9) | (null) of ./test_leak",
        "0x14dd (0x5faa1e9294dd) | (null) of ./test_leak"
      ],
      "allocs": 5,
      "free": 4,
      "size_human": "100 bytes",
      "size": 100
    },
    {
      "bt": [],
      "allocs": 0,
      "free": 0,
      "size_human": "0",
      "size": 0
    }
  ],
  "allocation_stacks": 6,
  "buckets": 6
}
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

nc -U /tmp/ht.sock | jq
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

