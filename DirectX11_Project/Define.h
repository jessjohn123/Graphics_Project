#pragma once
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "XTime.h"

using namespace DirectX;

#define RASTER_WIDTH 500
#define RASTER_HEIGHT 500
#define TOTAL_PIXELS ((RASTER_WIDTH) * (RASTER_HEIGHT))
#define RED_COLOR {1,0.001f,0.001f,1}
#define BLUE_COLOR {0.001f,0.001f,1,1}
#define GREEN_COLOR {0.001f,1,0.001f,1}
#define YELLOW_COLOR {1,1,0.001f,1}
#define WHITE_COLOR {1,1,1,1}
#define LIGHTBLUE_COLOR 0x0000FFFF
#define MAGENTA_COLOR 0xFF00FF

#define FIELDOFVIEW 65
#define ASPECTRATIO ((RASTER_WIDTH) / (RASTER_HEIGHT))
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

unsigned int Raster[TOTAL_PIXELS];
float zBuffer[TOTAL_PIXELS];

struct XMMatrix4
{
	XMFLOAT4 vect[4];
};

struct XMMatrix3
{
	XMFLOAT3 vect[3];
};
struct MY_VERTEX
{
	XMFLOAT4 pos;
	unsigned int color;
	XMFLOAT2 u, v;
};
#endif // !_DEFINE_H_

