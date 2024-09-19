#ifndef HYPERTHYMESIA_HINTS_H
#define HYPERTHYMESIA_HINTS_H

void ht_call_stack_location_hint(void **begin, void **end);

typedef void (*stack_hintfn_t)(void **begin, void **end);

void ht_register_stack_location_hint(stack_hintfn_t fn);

#endif // HYPERTHYMESIA_HINTS_H
