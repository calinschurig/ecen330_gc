tone_buffer_length = sample_hz/LOWEST_FREQ;

so if sample_hz = 400hz, and LOWEST_FREQ = 20hz, then to represent a full period of LOWEST_FREQ, we need a buffer length of at least 20. The minimum buffer length is 2, which is sample_hz of 40hz. The highest frequency we can play is then sample_hz / 2, for an up and down. 