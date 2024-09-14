# Hyperthymesia

Hyperthymesia is a lightweight C-framework for memory allocations statistics,
that are available from inside the application.

## How to use

Hyperthymesia can be used as cmake subproject, it builds the library libthyperthymesia,
which can be linked to your binary.

```
# Choose your favority backtrace max depth here
set(HT_BT_DEPTH 5)
add_subdirectory(<path to hyperthymesia clone>)

# ...
add_executable(my_app src/main.c)
target_link_libraries(my_app hyperthymesia)
```

And then, if you want to collect statistics of allocation for the thread,
you must do:
- initialize hyperthymesia in specified thread with `ht_thread_table_init`
- use `ht_malloc`, `ht_free`, `ht_realloc` functions instead of standard `malloc`, `free`, `realloc` functions
- destroy thread table after using hyperthymesia with `ht_thread_table_destroy`

See example at https://github.com/rkhapov/hyperthymesia-usage-example
