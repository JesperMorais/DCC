#ifndef CHANNEL_H
#define CHANNEL_H

/* Reads an IIO-style channel: (in_temp_raw + in_temp_offset) * in_temp_scale.
 * The offset file is optional. Returns 0, or -1 if a file is missing or
 * doesn't hold a number. */
int channel_read_temp(const char *dir, double *out);

#endif
