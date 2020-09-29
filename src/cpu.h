#ifndef GBCPU_H
#define GBCPU_H

static struct
{
	union
	{
		unsigned short AF;
		struct { unsigned char F, A; }
	};

	union
	{
		unsigned short BC;
		struct { unsigned char C, B; }
	};

	union
	{
		unsigned short DE;
		struct { unsigned char E, D; }
	};

	union
	{
		unsigned short HL;
		struct { unsigned char L, H; }
	};

	unsigned short SP;
	unsigned short PC;

} reg;

static unsigned char* cpuRam;
void init ();
void executeNextOp ();

#endif
