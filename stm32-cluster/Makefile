# STM32H743 instrument cluster firmware

CC = arm-none-eabi-gcc

# -I  CMSIS core headers (core_cm7.h: NVIC, SysTick)
# -I  device headers: H743 register addresses and bit masks
# -D  picks the chip inside stm32h7xx.h
# -mcpu/-mthumb  target core and instruction set
CFLAGS = -I external/CMSIS_6/CMSIS/Core/Include \
	-I external/cmsis_device_h7/Include \
	-D STM32H743xx \
	-mcpu=cortex-m7 -mthumb

STARTUP_DIR = external/cmsis_device_h7/Source/Templates/gcc

# list of files for linking
OBJS = build/main.o build/system_stm32h7xx.o build/startup_stm32h743xx.o

# path to link script ( .ld  )
LDSCRIPT = linker/stm32h743xx_flash.ld

# flags for linking
LDFLAGS = -mcpu=cortex-m7 -mthumb -T $(LDSCRIPT) --specs=nano.specs --specs=nosys.specs

# default target: build every object file
all: build/cluster.elf

# application code
build/main.o: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# SystemInit(): basic chip setup, called by startup before main
build/system_stm32h7xx.o: src/system_stm32h7xx.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# vector table and Reset_Handler: runs first after reset, then calls main
build/startup_stm32h743xx.o: $(STARTUP_DIR)/startup_stm32h743xx.s
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


# linking three object files into one .elf file
build/cluster.elf: $(OBJS) $(LDSCRIPT)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf build

# these names are actions, not files
.PHONY: all clean
