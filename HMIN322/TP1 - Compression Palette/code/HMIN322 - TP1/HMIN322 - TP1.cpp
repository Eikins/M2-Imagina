// HMIN322 - TP1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>

#include "ImageBase.h"

using namespace std;

void computeInitial2Means(ImageBase &image, U8ColorRGB &m0, U8ColorRGB &m1)
{
	int mag0 = 195076; // 3 * 255^2 + 1
	int mag1 = -1;

	for (int y = 0; y < image.getHeight(); ++y)
	{
		for (int x = 0; x < image.getWidth(); ++x)
		{
			U8ColorRGB pixel = image(x, y);

			int magnitude = pixel.magSq();
			if (magnitude < mag0)
			{
				mag0 = magnitude;
				m0 = pixel;
			}

			if (magnitude > mag1)
			{
				mag1 = magnitude;
				m1 = pixel;
			}
		}
	}
}

void recomputePalette(ImageBase &src, std::vector<unsigned char> &indexedImage, std::vector<U8ColorRGB> &palette)
{
	std::vector<float> means(palette.size() * 4);

	for (int y = 0; y < src.getHeight(); ++y)
	{
		for (int x = 0; x < src.getWidth(); ++x)
		{
			unsigned char colorIndex = indexedImage[y * src.getWidth() + x];
			U8ColorRGB pixel = src(x, y);
			means[colorIndex * 4 + 0] += pixel.r;
			means[colorIndex * 4 + 1] += pixel.g;
			means[colorIndex * 4 + 2] += pixel.b;
			means[colorIndex * 4 + 3] += 1; // count
		}
	}

	for (int i = 0; i < palette.size(); i++)
	{
		float count = means[4 * i + 3];
		palette[i].r = (unsigned char)(means[4 * i + 0] / count);
		palette[i].g = (unsigned char)(means[4 * i + 1] / count);
		palette[i].b = (unsigned char)(means[4 * i + 2] / count);
	}
}

void applyPalette(ImageBase &src, std::vector<unsigned char> &indexedImage, std::vector<U8ColorRGB> &palette)
{
	for (int y = 0; y < src.getHeight(); ++y)
	{
		for (int x = 0; x < src.getWidth(); ++x)
		{
			U8ColorRGB color = src(x, y);
			unsigned char nearestColorIndex;
			int nearestColorSqDist = 195076; // 3 * 255^2 + 1
			for (int i = 0; i < palette.size(); i++)
			{
				int distSq = color.distSq(palette[i]);
				if (distSq < nearestColorSqDist)
				{
					nearestColorIndex = i;
					nearestColorSqDist = distSq;
				}
			}
			indexedImage[y * src.getWidth() + x] = nearestColorIndex;
		}
	}
}

double psnr(ImageBase &src, ImageBase &cmp)
{
	double eqm[3] = { 0.0, 0.0, 0.0 };
	double mn_inv = 1.0 / (src.getHeight() * src.getWidth());

	for (int y = 0; y < src.getHeight(); ++y)
	{
		for (int x = 0; x < src.getWidth(); ++x)
		{
			U8ColorRGB srcColor = src(x, y);
			U8ColorRGB cmpColor = cmp(x, y);
			eqm[0] += std::abs(srcColor.r - cmpColor.r) * std::abs(srcColor.r - cmpColor.r);
			eqm[1] += std::abs(srcColor.g - cmpColor.g) * std::abs(srcColor.g - cmpColor.g);
			eqm[2] += std::abs(srcColor.b - cmpColor.b) * std::abs(srcColor.b - cmpColor.b);
		}
	}
	eqm[0] *= mn_inv;
	eqm[1] *= mn_inv;
	eqm[2] *= mn_inv;

	return 10.0 * std::log10(255 * 255 / ((eqm[0] + eqm[1] + eqm[2]) / 3.0));
}

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		cout << "Usage: input output_ppm output_pgm output_palette" << endl;
		return 1;
	}

	ImageBase inputImage;
	inputImage.load(argv[1]);

	//std::vector<U8ColorRGB> colorPalette(2);
	//computeInitial2Means(inputImage, colorPalette[0], colorPalette[1]);

	std::vector<U8ColorRGB> colorPalette(256);
	for (int i = 0; i < 256; i++)
	{
		colorPalette[i] = { std::rand() % 256, std::rand() % 256, std::rand() % 255 };
	}

	std::vector<unsigned char> compressedImageData(inputImage.getWidth() * inputImage.getHeight());

	std::vector<U8ColorRGB> lastPalette;
	int iter = 0;
	int max_iter = 1;

	do
	{
		lastPalette = colorPalette;
		applyPalette(inputImage, compressedImageData, colorPalette);
		recomputePalette(inputImage, compressedImageData, colorPalette);
		iter++;
		std::cout << iter << std::endl;
	} while (lastPalette != colorPalette && iter < max_iter);

	std::cout << iter << std::endl;
	
	ImageBase outputImage(inputImage.getWidth(), inputImage.getHeight(), inputImage.getColor());
	ImageBase outputPGM(inputImage.getWidth(), inputImage.getHeight(), inputImage.getColor());
	int paletteWidth = std::ceil(std::sqrt(colorPalette.size()));
	int paletteHeight = std::floor(std::sqrt(colorPalette.size()));
	ImageBase outputPalette(paletteWidth * 16, paletteHeight * 16, inputImage.getColor());


	for (int y = 0; y < outputImage.getHeight(); ++y)
	{
		for (int x = 0; x < outputImage.getWidth(); ++x)
		{
			unsigned char colorIndex = compressedImageData[y * outputImage.getWidth() + x];
			outputImage.set(x, y, colorPalette[colorIndex]);
		}
	}
	outputImage.save(argv[2]);

	for (int y = 0; y < outputImage.getHeight(); ++y)
	{
		for (int x = 0; x < outputImage.getWidth(); ++x)
		{
			unsigned char colorIndex = compressedImageData[y * outputImage.getWidth() + x];
			outputPGM.set(x, y, { colorIndex, colorIndex, colorIndex });
		}
	}
	outputPGM.save(argv[3]);


	for (int i = 0; i < colorPalette.size(); i++)
	{
		for (int y = 0; y < 16; ++y)
		{
			for (int x = 0; x < 16; ++x)
			{
				outputPalette.set((i % paletteWidth) * 16 + x, (i / paletteWidth) * 16 + y, colorPalette[i]);
			}
		}
	}

	outputPalette.save(argv[4]);

	std::cout << psnr(inputImage, outputImage) << std::endl;

	return 0;
}