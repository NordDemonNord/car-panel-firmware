#ifndef FDCAN_H
#define FDCAN_H

#define CAN_FRAME_DATA_LENGTH_MAX 8U

#include <stdint.h>

/* Clock fdcan2 pins(PB12/PB13) */
void fdcan2_init(void);

/* get ID, DLC and data from CAN frame */
int fdcan2_receive(uint32_t *ID, uint32_t *DLC, uint8_t *DATA);

#endif