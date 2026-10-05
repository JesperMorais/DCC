#ifndef SAMPLER_H
#define SAMPLER_H

#include <stdint.h>

/* Takes one reading from the ambient sensor now. 0 on success, or a negative errno. */
int sampler_read(int32_t *milli_c);

#endif /* SAMPLER_H */
