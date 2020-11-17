#include <Windows.h>
#include "cpu.h"

#define CPU_FREQ 4194304

LARGE_INTEGER frequency;

int main ()
{
	cpu.ram[0x0] = 0x04;
	cpu.ram[0x1] = 0x15;
	executeNextOp ();
	executeNextOp ();

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
	memset (cpu.ram, 0, 0x2000);

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
		/* CONTROL */
		case 0x00: cycles = 4; size = 1; break;	// NOP

		/* BRANCHES */

		/* 8-bit LOAD/STORE/MOVE */

		// LD immediate to memory
		case 0x02: wrMem(cpu.reg.BC, cpu.reg.A); cycles = 12; size = 3; break;
		case 0x12: wrMem(cpu.reg.DE, cpu.reg.A); cycles = 12; size = 3; break;

		/* 16-bit LOAD/STORE/MOVE */
		
		// LD immediate 16 bits to reg
		case 0x01: cpu.reg.BC = immWord; cycles = 12; size = 3; break;
		case 0x11: cpu.reg.DE = immWord; cycles = 12; size = 3; break;
		case 0x21: cpu.reg.HL = immWord; cycles = 12; size = 3; break;
		case 0x31: cpu.reg.SP = immWord; cycles = 12; size = 3; break;
	
		/* 8-bit ARITH/LOGIC */

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

		/* 16-bit ARITH/LOGIC */

		// Increment 16-bit registers
		case 0x03: cpu.reg.BC++; cycles = 8; size = 1; break;
		case 0x13: cpu.reg.DE++; cycles = 8; size = 1; break;
		case 0x23: cpu.reg.HL++; cycles = 8; size = 1; break;
		case 0x33: cpu.reg.SP++; cycles = 8; size = 1; break;

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

