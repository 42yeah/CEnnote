#pragma once

// Yeah, I know it's a little bit defensive 

struct Pos
{
	float X;
	float Y;
};

struct Pos enPos_create(float X, float Y);

void enPos_setXY(struct Pos* pos, float X, float Y);
