// TP2 - Changement d'espace couleur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <algorithm>

#include "ImageBase.h"

using namespace std;

// https://en.wikipedia.org/wiki/YCbCr
void RGB2YCbCr(unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &y, unsigned char &cb, unsigned char &cr)
{
	y  = (unsigned char) (16 +  (+65.738 * r + 129.057 * g + 25.064 * b) / 256);
	cb = (unsigned char) (128 + (-37.945 * r - 74.494 * g + 112.439 * b) / 256);
	cr = (unsigned char) (128 + (112.439 * r - 94.154 * g - 18.285 * b) / 256);
}

void YCbCr2RGB(unsigned char &y, unsigned char &cb, unsigned char &cr, unsigned char &r, unsigned char &g, unsigned char &b)
{
	r = (unsigned char) std::min(255.0, std::max((-222.921 + (298.082 * y + 408.583 * cr) / 256), 0.0));
	g = (unsigned char) std::min(255.0, std::max((+135.576 + (298.082 * y - 100.291 * cb - 208.120 * cr) / 256), 0.0));
	b = (unsigned char) std::min(255.0, std::max((-276.836 + (298.082 * y + 516.412 * cb) / 256), 0.0));
}

inline void sample(ImageBase &source, int &x, int &y, unsigned char &r, unsigned char &g, unsigned char &b)
{
	r = source[y * 3][x * 3 + 0];
	g = source[y * 3][x * 3 + 1];
	b = source[y * 3][x * 3 + 2];
}

inline void sampleReducedRB(ImageBase &source, int x, int y, unsigned char &r, unsigned char &g, unsigned char &b)
{
	g = source[y * 3][x * 3 + 1];
	// Get even column and rows
	if ((x & 1) == 1)
	{
		x--;
	}
	if ((y & 1) == 1)
	{
		y--;
	}
	r = source[y * 3][x * 3 + 0];
	b = source[y * 3][x * 3 + 2];
}

inline void sampleReducedGB(ImageBase &source, int x, int y, unsigned char &r, unsigned char &g, unsigned char &b)
{
	r = source[y * 3][x * 3 + 0];
	// Get even column and rows
	if ((x & 1) == 1)
	{
		x--;
	}
	if ((y & 1) == 1)
	{
		y--;
	}
	g = source[y * 3][x * 3 + 1];
	b = source[y * 3][x * 3 + 2];
}

inline void set(ImageBase &source, int &x, int &y, unsigned char &r, unsigned char &g, unsigned char &b)
{
	source[y * 3][x * 3 + 0] = r;
	source[y * 3][x * 3 + 1] = g;
	source[y * 3][x * 3 + 2] = b;
}

//inline void set(ImageBase &source, int x, int y, unsigned char r, unsigned char g, unsigned char b)
//{
//	source[y * 3][x * 3 + 0] = r;
//	source[y * 3][x * 3 + 1] = g;
//	source[y * 3][x * 3 + 2] = b;
//}

double psnr(ImageBase &src, ImageBase &cmp)
{
	double eqm[3] = { 0.0, 0.0, 0.0 };
	double mn_inv = 1.0 / (src.getHeight() * src.getWidth());

	for (int y = 0; y < src.getHeight(); ++y)
	{
		for (int x = 0; x < src.getWidth(); ++x)
		{
			unsigned char sr, sg, sb; // source
			unsigned char cr, cg, cb;
			sample(src, x, y, sr, sg, sb);
			sample(cmp, x, y, cr, cg, cb);
			eqm[0] += std::abs(sr - cr) * std::abs(sr - cr);
			eqm[1] += std::abs(sg - cg) * std::abs(sg - cg);
			eqm[2] += std::abs(sb - cb) * std::abs(sb - cb);
		}
	}
	eqm[0] *= mn_inv;
	eqm[1] *= mn_inv;
	eqm[2] *= mn_inv;

	return 10.0 * std::log10(255 * 255 / ((eqm[0] + eqm[1] + eqm[2]) / 3.0));
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cout << "Usage: input output_ppm output_pgm output_palette" << endl;
		return 1;
	}

	ImageBase inputImage;
	inputImage.load(argv[1]);

	ImageBase inputImageYCbCr(inputImage.getWidth(), inputImage.getHeight(), true);
	ImageBase outputImage(inputImage.getWidth(), inputImage.getHeight(), true);

	//// RGB Compression
	//for (int y = 0; y < inputImage.getHeight(); y++)
	//{
	//	for (int x = 0; x < inputImage.getWidth(); x++)
	//	{
	//		unsigned char r, g, b;
	//		sampleReducedRB(inputImage, x, y, r, g, b);
	//		set(outputImage, x, y, r, g, b);
	//	}
	//}

	// YCrCb Compression
	// To YCbCr
	for (int y = 0; y < inputImage.getHeight(); y++)
	{
		for (int x = 0; x < inputImage.getWidth(); x++)
		{
			unsigned char r, g, b;
			unsigned char Y, Cb, Cr;
			sample(inputImage, x, y, r, g, b);
			RGB2YCbCr(r, g, b, Y, Cb, Cr);
			set(inputImageYCbCr, x, y, Y, Cb, Cr);
		}
	}

	for (int y = 0; y < inputImage.getHeight(); y++)
	{
		for (int x = 0; x < inputImage.getWidth(); x++)
		{
			unsigned char r, g, b;
			sampleReducedGB(inputImageYCbCr, x, y, r, g, b);
			set(outputImage, x, y, r, g, b);
		}
	}

	// To RGB
	for (int y = 0; y < inputImage.getHeight(); y++)
	{
		for (int x = 0; x < inputImage.getWidth(); x++)
		{
			unsigned char Y, Cb, Cr;
			unsigned char r, g, b;
			sample(outputImage, x, y, Y, Cb, Cr);
			YCbCr2RGB(Y, Cb, Cr, r, g, b);
			set(outputImage, x, y, r, g, b);
		}
	}

	

	outputImage.save(argv[2]);
	inputImageYCbCr.save(argv[3]);

	std::cout << psnr(inputImage, outputImage) << std::endl;

}

