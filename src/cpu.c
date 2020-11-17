#include <Windows.h>
#include "cpu.h"

#define CPU_FREQ 4194304

LARGE_INTEGER frequency;

int main ()
{
	init();
	while (1) executeNextOp ();
	return 0;
}

uint8_t rdMem (uint16_t address)
{
	return cpu.ram[address];
}

void wrMem (uint16_t address, uint8_t value)
{
	cpu.ram[address] = value;
}

void init ()
{
	// Timing
	QueryPerformanceFrequency (&frequency);

	memset (cpu.bootRom, 0, 0x100);
	memset (cpu.vram, 0, 0x2000);
	memset (cpu.ram, 0x9, 0x2000);

	cpu.reg.A = 0;
	cpu.reg.F = 0;
	cpu.reg.BC = 0;
	cpu.reg.DE = 0;
	cpu.reg.HL = 0;
	cpu.reg.SP = 0;
	cpu.reg.PC = 0;
}

void executeNextOp ()
{
	LARGE_INTEGER startTime, endTime, uSecs;
	QueryPerformanceCounter (&startTime);

	uint8_t opcode = rdMem(cpu.reg.PC);
	uint8_t immByte = rdMem(cpu.reg.PC + 1);
	uint16_t immWord = (immByte << 8) | rdMem(cpu.reg.PC + 2); 

	uint8_t memVal;
	uint32_t cycles;
	uint32_t size;
	switch (opcode)	
	{
		/* HHHHHHH */
		/* CONTROL */
		/* HHHHHHH */
		case 0x00: cycles = 4; size = 1; break;	// NOP

		/* HHHHHHHH */
		/* BRANCHES */
		/* HHHHHHHH */

		/* HHHHHHHHHHHHHHHHHHHHH */
		/* 8-bit LOAD/STORE/MOVE */
		/* HHHHHHHHHHHHHHHHHHHHH */

		// LD Accumulator to memory
		case 0x02: wrMem(cpu.reg.BC, cpu.reg.A); cycles = 8; size = 1; break;
		case 0x12: wrMem(cpu.reg.DE, cpu.reg.A); cycles = 8; size = 1; break;

		// LD immediate to memory
		case 0x36: wrMem(cpu.reg.HL, immByte); cycles = 12; size = 2; break;

		// LD immediate to 8-bit register
		case 0x06: cpu.reg.B = immByte; cycles = 8; size = 2; break;
		case 0x16: cpu.reg.D = immByte; cycles = 8; size = 2; break;
		case 0x26: cpu.reg.H = immByte; cycles = 8; size = 2; break;

		/* 16-bit LOAD/STORE/MOVE */
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
				   if (!(cpu.reg.B)) cpu.reg.F|= ZERO_FLAG;
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
		case 0x24: 
				   cpu.reg.H++;
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(cpu.reg.H & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.H)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		
		// Increment memory value
		case 0x34: 
				   memVal = rdMem (cpu.reg.HL);
				   memVal++;
				   wrMem (cpu.reg.HL, memVal);
				   cpu.reg.F &= ~(ZERO_FLAG | NEG_FLAG | HALFCARRY_FLAG); 
				   if (!(memVal & 0x0F)) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(memVal)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 12;
				   size = 1;
				   break;

		// Decrement 8-bit registers
		case 0x05: 
				   cpu.reg.B--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &=  ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.B & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.B)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x15: 
				   cpu.reg.C--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.C & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.C)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;
		case 0x25: 
				   cpu.reg.H--;
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((cpu.reg.H & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(cpu.reg.H)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 4;
				   size = 1;
				   break;

		// Increment memory value
		case 0x35: 
				   memVal = rdMem (cpu.reg.HL);
				   memVal--;
				   wrMem (cpu.reg.HL, memVal);
				   cpu.reg.F |= NEG_FLAG;
				   cpu.reg.F &= ~(ZERO_FLAG | HALFCARRY_FLAG); 
				   if ((memVal & 0x0F) == 0xF) cpu.reg.F |= HALFCARRY_FLAG;
				   if (!(memVal)) cpu.reg.F|= ZERO_FLAG;
				   cycles = 12;
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

		case 0x17: //RLA
				   memVal = cpu.reg.F;
				   cpu.reg.F &= ~ALL_FLAGS;
				   cpu.reg.F |= (cpu.reg.A & 0x80) >> 3;
				   cpu.reg.A = (cpu.reg.A << 1) | ((memVal & CARRY_FLAG) >> 4);
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
	}

	// Update Program Counter
	cpu.reg.PC += size;

	// Timing
	uSecs.QuadPart = 0;
	while (uSecs.QuadPart < cycles * 1000000 / CPU_FREQ) 
	{
		QueryPerformanceCounter (&endTime);
		uSecs.QuadPart = endTime.QuadPart - startTime.QuadPart;
		uSecs.QuadPart *= 1000000;
		uSecs.QuadPart /= frequency.QuadPart;
	}
}

