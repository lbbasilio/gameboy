#include <Windows.h>
#include <stdio.h>
#include "cpu.h"

#define CPU_FREQ 4194304

LARGE_INTEGER frequency;

void loadRom(char* path) {
	FILE* f = fopen(path, "rb");
	if (!f) exit(-1);
	fseek(f, 0, SEEK_END);
	uint32_t len = ftell(f);
	rewind(f);
	fread(cpu.mem, len, 1, f);
	fclose(f);
}

int main() {
	init();
	//loadRom("C:\\Users\\luca\\Downloads\\PokeAka.gb");
	//loadRom("C:\\Users\\luca\\Downloads\\Tetris.gb");
	loadRom("C:\\Users\\luca\\Downloads\\boot_rom.gb");
	while (1) executeNextOp();
	return 0;
}

uint8_t rdMem(uint16_t address) {
	return cpu.mem[address];
}

void wrMem(uint16_t address, uint8_t value) {
	cpu.mem[address] = value;
}

void init() {
	// Timing
	QueryPerformanceFrequency(&frequency);

	memset(cpu.mem, 0, 0x10000);

	cpu.reg.A = 0;
	cpu.reg.F = 0;
	cpu.reg.BC = 0;
	cpu.reg.DE = 0;
	cpu.reg.HL = 0;
	cpu.reg.SP = 0xFFFE;
	cpu.reg.PC = 0;
}

void executeNextOp () {
	LARGE_INTEGER startTime, endTime, uSecs;
	QueryPerformanceCounter(&startTime);

	uint8_t opcode = rdMem(cpu.reg.PC);
	uint8_t immByte = rdMem(cpu.reg.PC + 1);
	uint16_t immWord = (rdMem(cpu.reg.PC + 2) << 8) | immByte; 

	uint8_t memVal;
	uint32_t cycles;
	uint32_t size;
	switch (opcode)	{
		/* HHHHHHH */
		/* CONTROL */
		/* HHHHHHH */
		case 0x00: cycles = 4; size = 1; break;	// NOP
		case 0x10: cycles = 4; size = 2; break; // STOP TODO: Implement STOP mode
		case 0x76: cycles = 4; size = 1; break; // HALT TODO: Implement HALT mode

		/* HHHHHHHHHHHHHH */
		/* EXECUTION FLOW */
		/* HHHHHHHHHHHHHH */
		case 0x18: cpu.reg.PC += (int8_t)immByte; cycles = 12; size = 0; break;
		case 0x20: 
				   if (!(cpu.reg.F & ZERO_FLAG)) {
					   cpu.reg.PC += (int8_t)immByte;
					   cycles = 12;
					   size = 2;
				   } 
				   else {
					   cycles = 8;
					   size = 2;
				   }
				   break;
		case 0x28: 
				   if (cpu.reg.F & ZERO_FLAG) {
					   cpu.reg.PC += (int8_t)immByte;
					   cycles = 12;
					   size = 2;
				   } 
				   else {
					   cycles = 8;
					   size = 2;
				   }
				   break;
		case 0x30: 
				   if (!(cpu.reg.F & CARRY_FLAG)) {
					   cpu.reg.PC += (int8_t)immByte;
					   cycles = 12;
					   size = 2;
				   } 
				   else {
					   cycles = 8;
					   size = 2;
				   }
				   break;
		case 0x38: 
				   if (cpu.reg.F & CARRY_FLAG) {
					   cpu.reg.PC += (int8_t)immByte;
					   cycles = 12;
					   size = 2;
				   } 
				   else {
					   cycles = 8;
					   size = 2;
				   }
				   break;

		case 0xCD: cpu.reg.PC += 3;
				   wrMem(cpu.reg.SP--, cpu.reg.PC >> 8);
				   wrMem(cpu.reg.SP--, (cpu.reg.PC & 0xFF) << 8);
				   cpu.reg.PC = ((immWord & 0xFF) << 8) | (immWord >> 8);
				   cycles = 12;
				   size = 3;
				   break;

		case 0xC5: wrMem(cpu.reg.SP--, cpu.reg.C);
				   wrMem(cpu.reg.SP--, cpu.reg.B);
				   cycles = 16;
				   size = 1;
				   break;

		/* HHHHHHHHHHHHHHHHHHHHH */
		/* 8-bit LOAD/STORE/MOVE */
		/* HHHHHHHHHHHHHHHHHHHHH */

		// LD Accumulator to memory
		case 0x02: wrMem(cpu.reg.BC, cpu.reg.A); cycles = 8; size = 1; break;
		case 0x12: wrMem(cpu.reg.DE, cpu.reg.A); cycles = 8; size = 1; break;
		case 0x22: wrMem(cpu.reg.HL, cpu.reg.A); cpu.reg.HL++; cycles = 8; size = 1; break;
		case 0x32: wrMem(cpu.reg.HL, cpu.reg.A); cpu.reg.HL--; cycles = 8; size = 1; break;

		// LD immediate to memory
		case 0x36: wrMem(cpu.reg.HL, immByte); cycles = 12; size = 2; break;

		// LD immediate to 8-bit register
		case 0x06: cpu.reg.B = immByte; cycles = 8; size = 2; break;
		case 0x0E: cpu.reg.C = immByte; cycles = 8; size = 2; break;
		case 0x16: cpu.reg.D = immByte; cycles = 8; size = 2; break;
		case 0x1E: cpu.reg.E = immByte; cycles = 8; size = 2; break;
		case 0x26: cpu.reg.H = immByte; cycles = 8; size = 2; break;
		case 0x2E: cpu.reg.L = immByte; cycles = 8; size = 2; break;
		case 0x3E: cpu.reg.A = immByte; cycles = 8; size = 2; break;

		// LD Accumulator from memory
		case 0x0A: cpu.reg.A = rdMem(cpu.reg.BC); cycles = 8; size = 1; break;
		case 0x1A: cpu.reg.A = rdMem(cpu.reg.DE); cycles = 8; size = 1; break;
		case 0x2A: cpu.reg.A = rdMem(cpu.reg.HL); cpu.reg.HL++; cycles = 8; size = 1; break;
		case 0x3A: cpu.reg.A = rdMem(cpu.reg.HL); cpu.reg.HL--; cycles = 8; size = 1; break;

		// 8-bit register moves
		case 0x40: cpu.reg.B = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x41: cpu.reg.B = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x42: cpu.reg.B = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x43: cpu.reg.B = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x44: cpu.reg.B = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x45: cpu.reg.B = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x47: cpu.reg.B = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x48: cpu.reg.C = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x49: cpu.reg.C = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x4A: cpu.reg.C = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x4B: cpu.reg.C = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x4C: cpu.reg.C = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x4D: cpu.reg.C = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x4F: cpu.reg.C = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x50: cpu.reg.D = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x51: cpu.reg.D = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x52: cpu.reg.D = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x53: cpu.reg.D = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x54: cpu.reg.D = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x55: cpu.reg.D = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x57: cpu.reg.D = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x58: cpu.reg.E = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x59: cpu.reg.E = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x5A: cpu.reg.E = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x5B: cpu.reg.E = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x5C: cpu.reg.E = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x5D: cpu.reg.E = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x5F: cpu.reg.E = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x60: cpu.reg.H = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x61: cpu.reg.H = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x62: cpu.reg.H = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x63: cpu.reg.H = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x64: cpu.reg.H = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x65: cpu.reg.H = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x67: cpu.reg.H = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x68: cpu.reg.L = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x69: cpu.reg.L = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x6A: cpu.reg.L = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x6B: cpu.reg.L = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x6C: cpu.reg.L = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x6D: cpu.reg.L = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x6F: cpu.reg.L = cpu.reg.A; cycles = 4; size = 1; break;
		case 0x78: cpu.reg.A = cpu.reg.B; cycles = 4; size = 1; break;
		case 0x79: cpu.reg.A = cpu.reg.C; cycles = 4; size = 1; break;
		case 0x7A: cpu.reg.A = cpu.reg.D; cycles = 4; size = 1; break;
		case 0x7B: cpu.reg.A = cpu.reg.E; cycles = 4; size = 1; break;
		case 0x7C: cpu.reg.A = cpu.reg.H; cycles = 4; size = 1; break;
		case 0x7D: cpu.reg.A = cpu.reg.L; cycles = 4; size = 1; break;
		case 0x7F: cpu.reg.A = cpu.reg.A; cycles = 4; size = 1; break;
				   
		// LD 8-bit register from memory
		case 0x46: cpu.reg.B = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x4E: cpu.reg.C = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x56: cpu.reg.D = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x5E: cpu.reg.E = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x66: cpu.reg.H = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x6E: cpu.reg.L = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;
		case 0x7E: cpu.reg.A = rdMem(cpu.reg.HL); cycles = 8; size = 1; break;

		// STORE 8-bit register
		case 0x70: wrMem(cpu.reg.HL, cpu.reg.B); cycles = 8; size = 1; break;
		case 0x71: wrMem(cpu.reg.HL, cpu.reg.C); cycles = 8; size = 1; break;
		case 0x72: wrMem(cpu.reg.HL, cpu.reg.D); cycles = 8; size = 1; break;
		case 0x73: wrMem(cpu.reg.HL, cpu.reg.E); cycles = 8; size = 1; break;
		case 0x74: wrMem(cpu.reg.HL, cpu.reg.H); cycles = 8; size = 1; break;
		case 0x75: wrMem(cpu.reg.HL, cpu.reg.L); cycles = 8; size = 1; break;
		case 0x77: wrMem(cpu.reg.HL, cpu.reg.A); cycles = 8; size = 1; break;

		// Special LD/STORE
		case 0xE0: wrMem(0xFF00 + immByte, cpu.reg.A); cycles = 12; size = 1; break;
		case 0xE2: wrMem(0xFF00 + cpu.reg.C, cpu.reg.A); cycles = 8; size = 1; break;
		case 0xF0: cpu.reg.A = rdMem(0xFF00 + immByte); cycles = 12; size = 1; break;
		case 0xF2: cpu.reg.A = rdMem(0xFF00 + cpu.reg.C); cycles = 8; size = 1; break;

		/* HHHHHHHHHHHHHHHHHHHHHH */
		/* 16-bit LOAD/STORE/MOVE */
		/* HHHHHHHHHHHHHHHHHHHHHH */

		// LD immediate 16 bits to reg
		case 0x01: cpu.reg.BC = immWord; cycles = 12; size = 3; break;
		case 0x11: cpu.reg.DE = immWord; cycles = 12; size = 3; break;
		case 0x21: cpu.reg.HL = immWord; cycles = 12; size = 3; break;
		case 0x31: cpu.reg.SP = immWord; cycles = 12; size = 3; break;

		// LD SP to memory
		case 0x08: wrMem(immWord, cpu.reg.SP); cycles = 20; size = 3; break;
	
		/* HHHHHHHHHHHHHHHHH */
		/* 8-bit ARITH/LOGIC */
		/* HHHHHHHHHHHHHHHHH */

		// Increment 8-bit registers
		case 0x04: 
				   cpu.reg.B++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.B & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.B)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x0C: 
				   cpu.reg.C++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.C & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.C)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x14: 
				   cpu.reg.D++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.D & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.D)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x1C: 
				   cpu.reg.E++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.E & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.E)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x24: 
				   cpu.reg.H++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.H & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.H)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x2C: 
				   cpu.reg.L++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.L & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.L)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x34: 
				   memVal = rdMem (cpu.reg.HL);
				   memVal++;
				   wrMem (cpu.reg.HL, memVal);
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(memVal & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(memVal)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 12;
				   size = 1;
				   break;
		case 0x3C: 
				   cpu.reg.A++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.A & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.A)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		// Decrement 8-bit registers
		case 0x05: 
				   cpu.reg.B--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.B & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.B)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x0D: 
				   cpu.reg.C--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.C & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.C)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x15: 
				   cpu.reg.D--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.D & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.D)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x1D: 
				   cpu.reg.E--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.E & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.E)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x25: 
				   cpu.reg.H--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.H & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.H)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x2D: 
				   cpu.reg.L--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.L & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.L)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x35: 
				   memVal = rdMem (cpu.reg.HL);
				   memVal--;
				   wrMem (cpu.reg.HL, memVal);
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((memVal & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(memVal)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 12;
				   size = 1;
				   break;
		case 0x3D: 
				   cpu.reg.A--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.A & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.A)) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		// 8-bit ADDs
		case 0x80:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.B & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.B) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.B;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x81:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.C & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.C) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.C;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x82:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.D & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.D) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.D;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x83:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.E & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.E) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.E;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x84:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.H & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.H) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.H;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x85:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.L & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.L) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.L;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x86:
				   cpu.reg.F &= ~ALL_FLAGS;
				   memVal = rdMem(cpu.reg.HL);
				   if (((cpu.reg.A & 0x0F) + (memVal & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + memVal) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += memVal;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x87:
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.A & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.A) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.A;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x88:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.B & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.B) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.B;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x89:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.C & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.C) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.C;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8A:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.D & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.D) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.D;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8B:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.E & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.E) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.E;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8C:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.H & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.H) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.H;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8D:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.L & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.L) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.L;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8E:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   memVal = rdMem(cpu.reg.HL);
				   if (((cpu.reg.A & 0x0F) + (memVal & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + memVal) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += memVal;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x8F:
				   cpu.reg.A += (cpu.reg.F & CARRY_FLAG) << 4;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (((cpu.reg.A & 0x0F) + (cpu.reg.A & 0x0F)) & 0x10)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint16_t)(cpu.reg.A + cpu.reg.A) & 0x100)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.A += cpu.reg.A;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		// 8-bit XORs
		case 0xA8:
				   cpu.reg.A ^= cpu.reg.B;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xA9:
				   cpu.reg.A ^= cpu.reg.C;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAA:
				   cpu.reg.A ^= cpu.reg.D;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAB:
				   cpu.reg.A ^= cpu.reg.E;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAC:
				   cpu.reg.A ^= cpu.reg.H;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAD:
				   cpu.reg.A ^= cpu.reg.L;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAE:
				   cpu.reg.A ^= rdMem(cpu.reg.HL);
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0xAF:
				   cpu.reg.A ^= cpu.reg.A;
				   cpu.reg.F &= ~ALL_FLAGS;
				   if (!cpu.reg.A)
					   cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		// Rotations
		case 0x07: // RLCA
				   cpu.reg.F &= ~ALL_FLAGS;  
				   cpu.reg.F |= (cpu.reg.A & 0x80) >> 3;
				   cpu.reg.A = (cpu.reg.A << 1) | ((cpu.reg.A & 0x80) >> 7); 
				   cycles = 4;
				   size = 1;
				   break;
		case 0x0F: // RRCA
				   cpu.reg.F &= ~ALL_FLAGS;  
				   cpu.reg.F |= (cpu.reg.A & 0x01) << 4;
				   cpu.reg.A = (cpu.reg.A >> 1) | ((cpu.reg.A & 0x01) << 7); 
				   cycles = 4;
				   size = 1;
				   break;
		case 0x17: // RLA
				   memVal = cpu.reg.F;
				   cpu.reg.F &= ~ALL_FLAGS;
				   cpu.reg.F |= (cpu.reg.A & 0x80) >> 3;
				   cpu.reg.A = (cpu.reg.A << 1) | ((memVal & CARRY_FLAG) >> 4);
				   cycles = 4;
				   size = 1;
				   break;
		case 0x1F: // RRA
				   memVal = cpu.reg.F;
				   cpu.reg.F &= ~ALL_FLAGS;
				   cpu.reg.F |= (cpu.reg.A & 0x01) << 4;
				   cpu.reg.A = (cpu.reg.A >> 1) | ((memVal & CARRY_FLAG) << 3);
				   cycles = 4;
				   size = 1;
				   break;

		/* HHHHHHHHHHHHHHHHHH */
		/* 16-bit ARITH/LOGIC */
		/* HHHHHHHHHHHHHHHHHH */

		// Increment 16-bit registers
		case 0x03: cpu.reg.BC++; cycles = 8; size = 1; break;
		case 0x13: cpu.reg.DE++; cycles = 8; size = 1; break;
		case 0x23: cpu.reg.HL++; cycles = 8; size = 1; break;
		case 0x33: cpu.reg.SP++; cycles = 8; size = 1; break;

		// 16-bit Add
		case 0x09: 
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG | CARRY_FLAG);
				   if (((cpu.reg.HL & 0xFFF) + (cpu.reg.BC & 0xFFF)) >> 12)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint32_t)(cpu.reg.HL + cpu.reg.BC) & 0x10000)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.HL += cpu.reg.BC;
				   cycles = 8;
				   size = 1;
				   break;
		case 0x19: 
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG | CARRY_FLAG);
				   if (((cpu.reg.HL & 0xFFF) + (cpu.reg.DE & 0xFFF)) >> 12)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint32_t)(cpu.reg.HL + cpu.reg.DE) & 0x10000)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.HL += cpu.reg.DE;
				   cycles = 8;
				   size = 1;
				   break;
		case 0x29: 
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG | CARRY_FLAG);
				   if (((cpu.reg.HL & 0xFFF) + (cpu.reg.HL & 0xFFF)) >> 12)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint32_t)(cpu.reg.HL + cpu.reg.HL) & 0x10000)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.HL += cpu.reg.HL;
				   cycles = 8;
				   size = 1;
				   break;
		case 0x39: 
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG | CARRY_FLAG);
				   if (((cpu.reg.HL & 0xFFF) + (cpu.reg.SP & 0xFFF)) >> 12)
					   cpu.reg.F |= HALFCARRY_FLAG;
				   if ((uint32_t)(cpu.reg.HL + cpu.reg.SP) & 0x10000)
					   cpu.reg.F |= CARRY_FLAG;
				   cpu.reg.HL += cpu.reg.SP;
				   cycles = 8;
				   size = 1;
				   break;

		// Decrement 16-bit registers
		case 0x0B: cpu.reg.BC--; cycles = 8; size = 1; break;
		case 0x1B: cpu.reg.DE--; cycles = 8; size = 1; break;
		case 0x2B: cpu.reg.HL--; cycles = 8; size = 1; break;
		case 0x3B: cpu.reg.SP--; cycles = 8; size = 1; break;

		/* HHHHHHHHH */
		/* CB PREFIX */
		/* HHHHHHHHH */
		case 0xCB: {
					   switch (immByte) {
						   case 0x30: 
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.B = ((cpu.reg.B & 0xF) << 4) | (cpu.reg.B >> 4);
									  if (!cpu.reg.B)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x31:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.C = ((cpu.reg.C & 0xF) << 4) | (cpu.reg.C >> 4);
									  if (!cpu.reg.C)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x32:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.D = ((cpu.reg.D & 0xF) << 4) | (cpu.reg.D >> 4);
									  if (!cpu.reg.D)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x33:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.E = ((cpu.reg.E & 0xF) << 4) | (cpu.reg.E >> 4);
									  if (!cpu.reg.E)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x34:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.H = ((cpu.reg.H & 0xF) << 4) | (cpu.reg.H >> 4);
									  if (!cpu.reg.H)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x35:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.L = ((cpu.reg.L & 0xF) << 4) | (cpu.reg.L >> 4);
									  if (!cpu.reg.L)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x36:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  memVal = rdMem(cpu.reg.HL);
									  memVal = ((memVal & 0xF) << 4) | (memVal >> 4);
									  if (!memVal)
										  cpu.reg.F |= ZERO_FLAG;
									  wrMem(cpu.reg.HL, memVal);
									  cycles = 16;
									  size = 2;
									  break;
						   case 0x37:
							   		  cpu.reg.F &= ~ALL_FLAGS;
									  cpu.reg.A = ((cpu.reg.A & 0xF) << 4) | (cpu.reg.A >> 4);
									  if (!cpu.reg.A)
										  cpu.reg.F |= ZERO_FLAG;
									  cycles = 8;
									  size = 2;
									  break;

						   // BIT
						   case 0x40:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x41:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x42:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x43:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x44:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x45:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x46:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x47:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x1) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x48:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x49:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4A:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4B:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4C:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4D:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4E:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x4F:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x2) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x50:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x51:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x52:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x53:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x54:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x55:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x56:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x57:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x3) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x58:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x59:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5A:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5B:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5C:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5D:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5E:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x5F:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x4) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x60:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x61:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x62:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x63:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x64:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x65:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x66:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x67:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x5) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x68:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x69:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6A:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6B:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6C:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6D:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6E:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x6F:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x6) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x70:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x71:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x72:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x73:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x74:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x75:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x76:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x77:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x7) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x78:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.B & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x79:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.C & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7A:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.D & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7B:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.E & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7C:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.H & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7D:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.L & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7E:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(rdMem(cpu.reg.HL) & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;
						   case 0x7F:
									  cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG);
									  cpu.reg.F |= (!(cpu.reg.A & 0x8) << 7) | HALFCARRY_FLAG;
									  cycles = 8;
									  size = 2;
									  break;

						   default:
								   	  printf("Uninmplemented Opcode 0xCB%02x at 0x%04x\n", immByte, cpu.reg.PC);
									  break;
					   }
				   }
				   break;

		/* HHHH */
		/* MISC */
		/* HHHH */
		case 0x27: // DAA
				   if (cpu.reg.F & NEG_FLAG) {
					   if (cpu.reg.F & CARRY_FLAG) cpu.reg.A -= 0x60;
					   if (cpu.reg.F & HALFCARRY_FLAG) cpu.reg.A -= 0x06;
				   }
				   else {
					   if ((cpu.reg.F & CARRY_FLAG) || (cpu.reg.A > 0x99)) {
						   cpu.reg.A += 0x60;
						   cpu.reg.F |= CARRY_FLAG;
					   }
					   if ((cpu.reg.F & HALFCARRY_FLAG) || ((cpu.reg.A & 0x0F) > 0x09)) {
						   cpu.reg.A += 0x06;
					   }
				   }
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG);
				   if (!cpu.reg.A) cpu.reg.F |= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		case 0x2F: // CPL
				   cpu.reg.A = ~cpu.reg.A;
				   cpu.reg.F |= (NEG_FLAG | HALFCARRY_FLAG);
				   cycles = 4;
				   size = 1;
				   break;

		case 0x37: // SCF
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG);
				   cpu.reg.F |= CARRY_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		case 0x3F: // CCF
				   cpu.reg.F &= ~(NEG_FLAG | HALFCARRY_FLAG);
				   cpu.reg.F ^= CARRY_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		default:
				   printf("Uninmplemented Opcode 0x%02x at 0x%04x\n", opcode, cpu.reg.PC);
				   break;
	}	

	// Update Program Counter
	cpu.reg.PC += size;

	// Timing
	uSecs.QuadPart = 0;
	while (uSecs.QuadPart < cycles * 1000000 / CPU_FREQ) {
		QueryPerformanceCounter (&endTime);
		uSecs.QuadPart = endTime.QuadPart - startTime.QuadPart;
		uSecs.QuadPart *= 1000000;
		uSecs.QuadPart /= frequency.QuadPart;
	}
}

