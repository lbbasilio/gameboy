#include <stdio.h>
#include <time.h>
#include "cpu.h"

int main ()
{
	reg.AF = 0x9040;

	printf("%d\n", reg.A);
	printf("%d\n", reg.F);
	
	return 0;
}

void init ()
{
	cpuRam = malloc(0x10000);
	reg.AF = 0x0;
	reg.BC = 0x0;
	reg.DE = 0x0;
	reg.HL = 0x0;
	reg.SP = 0xFFFE;
	reg.PC = 0x100;
}

void executeNextOp ()
{
	clock_t start, diff;
	start = clock();

	unsigned int cycles;
	unsigned char opcode;
	unsigned char target;
	unsigned char immval;
	switch (opcode)	
	{
		// LD immediate 8 bits to reg
		case 0x06: reg.B = immval; cycles = 8; break;
		case 0x0E: reg.C = immval; cycles = 8; break;
		case 0x16: reg.D = immval; cycles = 8; break;
		case 0x1E: reg.E = immval; cycles = 8; break;
		case 0x26: reg.H = immval; cycles = 8; break;
		case 0x2E: reg.L = immval; cycles = 8; break;

		// LD reg to reg
		case 0x7F: reg.A = reg.A; cycles = 4; break;
		case 0x78: reg.A = reg.B; cycles = 4; break;
		case 0x79: reg.A = reg.C; cycles = 4; break;
		case 0x7A: reg.A = reg.D; cycles = 4; break;
		case 0x7B: reg.A = reg.E; cycles = 4; break;
		case 0x7C: reg.A = reg.H; cycles = 4; break;
		case 0x7D: reg.A = reg.L; cycles = 4; break;
		case 0x7E: reg.A = reg.HL; cycles = 8; break;

		case 0x40: reg.B = reg.B; cycles = 4; break;
		case 0x41: reg.B = reg.C; cycles = 4; break;
		case 0x42: reg.B = reg.D; cycles = 4; break;
		case 0x43: reg.B = reg.E; cycles = 4; break;
		case 0x44: reg.B = reg.H; cycles = 4; break;
		case 0x45: reg.B = reg.L; cycles = 4; break;
		case 0x46: reg.B = reg.HL; cycles = 8; break;
	
		case 0x48: reg.C = reg.B; cycles = 4; break;
		case 0x49: reg.C = reg.C; cycles = 4; break;
		case 0x4A: reg.C = reg.D; cycles = 4; break;
		case 0x4B: reg.C = reg.E; cycles = 4; break;
		case 0x4C: reg.C = reg.H; cycles = 4; break;
		case 0x4D: reg.C = reg.L; cycles = 4; break;
		case 0x4E: reg.C = reg.HL; cycles = 8; break;

		case 0x50: reg.D = reg.B; cycles = 4; break;
		case 0x51: reg.D = reg.C; cycles = 4; break;
		case 0x52: reg.D = reg.D; cycles = 4; break;
		case 0x53: reg.D = reg.E; cycles = 4; break;
		case 0x54: reg.D = reg.H; cycles = 4; break;
		case 0x55: reg.D = reg.L; cycles = 4; break;
		case 0x56: reg.D = reg.HL; cycles = 8; break;
	
		case 0x58: reg.E = reg.B; cycles = 4; break;
		case 0x59: reg.E = reg.C; cycles = 4; break;
		case 0x5A: reg.E = reg.D; cycles = 4; break;
		case 0x5B: reg.E = reg.E; cycles = 4; break;
		case 0x5C: reg.E = reg.H; cycles = 4; break;
		case 0x5D: reg.E = reg.L; cycles = 4; break;
		case 0x5E: reg.E = reg.HL; cycles = 8; break;

		case 0x60: reg.H = reg.B; cycles = 4; break;
		case 0x61: reg.H = reg.C; cycles = 4; break;
		case 0x62: reg.H = reg.D; cycles = 4; break;
		case 0x63: reg.H = reg.E; cycles = 4; break;
		case 0x64: reg.H = reg.H; cycles = 4; break;
		case 0x65: reg.H = reg.L; cycles = 4; break;
		case 0x66: reg.H = reg.HL; cycles = 8; break;
	
		case 0x68: reg.L = reg.B; cycles = 4; break;
		case 0x69: reg.L = reg.C; cycles = 4; break;
		case 0x6A: reg.L = reg.D; cycles = 4; break;
		case 0x6B: reg.L = reg.E; cycles = 4; break;
		case 0x6C: reg.L = reg.H; cycles = 4; break;
		case 0x6D: reg.L = reg.L; cycles = 4; break;
		case 0x6E: reg.L = reg.HL; cycles = 8; break;

	}

	diff = start - clock();
}

