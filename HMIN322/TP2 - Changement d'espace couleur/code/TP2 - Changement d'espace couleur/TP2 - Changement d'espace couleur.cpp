// TP2 - Changement d'espace couleur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>

#include "ImageBase.h"

using namespace std;

// https://en.wikipedia.org/wiki/YCbCr
void RGBtoYCbCr(unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &y, unsigned char &cb, unsigned char &cr)
{
	y = (unsigned char) (0.299 * r + 0.587 * g + 0.114 * b);
	cb = (unsigned char) (128 + (b - y) / (2 - 2 * 0.114 * b));
	cr = (unsigned char) (128 + (r - y) / (2 - 2 * 0.299 * r));
}

void YCbCrtoRGB(unsigned char &y, unsigned char &cb, unsigned char &cr, unsigned char &r, unsigned char &g, unsigned char &b)
{
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: input output_ppm output_pgm output_palette" << endl;
		return 1;
	}

	ImageBase inputImage;
	inputImage.load(argv[1]);



}

