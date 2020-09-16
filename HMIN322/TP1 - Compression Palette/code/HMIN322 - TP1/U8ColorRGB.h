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
	inline int distSq(const U8ColorRGB &target) const { return (r - target.r) * (r - target.r) + (g - target.g) * (g - target.g) + (b - target.b) * (b - target.b); }

	bool operator==(const U8ColorRGB &other) const { return r == other.r && g == other.g && b == other.b; }
};