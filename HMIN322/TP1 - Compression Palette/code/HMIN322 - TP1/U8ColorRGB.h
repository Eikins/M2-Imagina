#pragma once

#include <vector>

struct U8ColorRGB
{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;


	U8ColorRGB() { r = 0; g = 0; b = 0; }
	U8ColorRGB(int red, int green, int blue) { r = red; g = green; b = blue; }
	inline int magSq() { return r * r + g * g + b * b; }
	inline int distSq(U8ColorRGB target) { return (r - target.r) * (r - target.r) + (g - target.g) * (g - target.g) + (b - target.b) * (b - target.b); }

	bool operator==(U8ColorRGB other) { return r == other.r && g == other.g && b == other.b; }
};