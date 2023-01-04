#ifndef GBCPU_H
#define GBCPU_H

#include <stdint.h>

#define ZERO_FLAG 0x80
#define NEG_FLAG 0x40
#define HALFCARRY_FLAG 0x20
#define CARRY_FLAG 0x10
#define ALL_FLAGS 0xF0

static struct {
	// Registers
	struct {
		uint8_t A, F;
		union {
			uint16_t BC;
			struct { uint8_t C, B; };
		};
		union {
			uint16_t DE;
			struct { uint8_t E, D; };
		};
		union {
			uint16_t HL;
			struct { uint8_t L, H; };
		};
		uint16_t SP;
		uint16_t PC;

	} reg;

	// Memory
	uint8_t mem[0x10000];


} cpu;

uint8_t rdMem(uint16_t address);
void wrMem(uint16_t address, uint8_t value);

void init();
void executeNextOp();

#endif
