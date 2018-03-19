#include "Pos.h"

struct Pos enPos_create(float X, float Y)
{
	struct Pos p;
	p.X = X;
	p.Y = Y;

	return p;
}

void enPos_setXY(struct Pos* pos, float X, float Y)
{
	pos->X = X;
	pos->Y = Y;
}
