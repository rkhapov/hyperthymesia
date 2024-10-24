# Maximum depth of collecting backtraces
# bigger backtraces leads to huge memory consumption
# for allocation stats table and slow
# memory allocation functions, but allow to find leaks more precisely.
if (NOT DEFINED HT_MAX_BT_DEPTH)
    set(HT_MAX_BT_DEPTH 15)
endif()

# Registration of every allocation is expensive
# HT_ALLOC_SAMPLES allows to specify percentage of allocations
# that will be managed (of 100)
# Ex: 50 means that only half allocations will be managed
if (NOT DEFINED HT_ALLOC_SAMPLES)
    set(HT_ALLOC_SAMPLES 100)
endif()

# Sending table by unix socket
# requires traversing it with acquiring the mutexes.
# To reduce the impact of that action, the table is sent by
# parts of size HT_TABLE_BLOCK_WRITE_NSTATS with pause of
# HT_TABLE_BLOCK_WRITE_SLEEP_NS between the parts.
if (NOT DEFINED HT_TABLE_BLOCK_WRITE_SLEEP_NS)
    set(HT_TABLE_BLOCK_WRITE_SLEEP_NS 50000000) # 50 ms
endif()

if (NOT DEFINED HT_TABLE_BLOCK_WRITE_NSTATS)
    set(HT_TABLE_BLOCK_WRITE_NSTATS 128)
endif()

# Table of allocations works just like normal
# hash-table with buckets.
# HT_BUCKETS allows to control number of buckets in table
# HT_BUCKET_LEN allows to control initial bucket size.
if (NOT DEFINED HT_BUCKETS)
    set(HT_BUCKETS 47351)
endif()

if (NOT DEFINED HT_BUCKET_LEN)
    set(HT_BUCKET_LEN 32)
endif()
