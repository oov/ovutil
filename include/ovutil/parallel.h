#pragma once

#include "ovbase.h"

// These functions returns the number of logical/physical cores, not the number of active cores.
// If you have more than 64 cores, you will not get the numbers you expect on Windows.
NODISCARD error get_number_of_logical_cores(size_t *const n);
NODISCARD error get_number_of_physical_cores(size_t *const n);

typedef void (*ovparallel_task)(void *userdata, size_t const index, size_t const total);

struct ovparallel;
NODISCARD error ovparallel_create(struct ovparallel **const pp, size_t const threads);
NODISCARD error ovparallel_destroy(struct ovparallel **const pp);
void ovparallel_for(struct ovparallel *const p, ovparallel_task fn, void *const userdata, size_t const n);
