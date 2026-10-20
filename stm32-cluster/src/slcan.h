#ifndef SLCAN_H
#define SLCAN_H

#define SLCAN_FRAME_MAX_LENGTH 22u

#include <stdint.h>

uint32_t slcan_build_frame(char *out, uint16_t id, const uint8_t *data, uint8_t dlc);

#endif 