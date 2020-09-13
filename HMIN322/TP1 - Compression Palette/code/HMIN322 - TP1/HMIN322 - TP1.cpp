// HMIN322 - TP1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <string>

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


//void compute2Means(ImageBase &src, ImageBase &dst, U8ColorRGB &m0, U8ColorRGB &m1)
//{
//
//	float newM0[]{ 0, 0, 0 };
//	float newM1[]{ 0, 0, 0 };
//
//	unsigned int m0Count = 0;
//	unsigned int m1Count = 0;
//
//	for (int y = 0; y < src.getHeight(); ++y)
//	{
//		for (int x = 0; x < src.getWidth(); ++x)
//		{
//			U8ColorRGB pixel = src(x, y);
//			U8ColorRGB mask = dst(x, y);
//
//			if (mask == m0)
//			{
//				newM0[0] += pixel.r;
//				newM0[1] += pixel.g;
//				newM0[1] += pixel.b;
//				m0Count++;
//			}
//			else
//			{
//				newM1[0] += pixel.r;
//				newM1[1] += pixel.g;
//				newM1[1] += pixel.b;
//				m1Count++;
//			}
//		}
//	}
//
//	m0.r = (unsigned char)(newM0[0] / m0Count);
//	m0.g = (unsigned char)(newM0[1] / m0Count);
//	m0.b = (unsigned char)(newM0[2] / m0Count);
//
//	m1.r = (unsigned char)(newM1[0] / m1Count);
//	m1.g = (unsigned char)(newM1[1] / m1Count);
//	m1.b = (unsigned char)(newM1[2] / m1Count);
//}
//
//void assign2Means(ImageBase &src, ImageBase &dst, U8ColorRGB &m0, U8ColorRGB &m1, int mag0, int mag1)
//{
//	for (int y = 0; y < src.getHeight(); ++y)
//	{
//		for (int x = 0; x < src.getWidth(); ++x)
//		{
//			U8ColorRGB color = src(x, y);
//			int magnitude = color.mag();
//
//			if (abs(magnitude - mag0) < abs(magnitude - mag1))
//			{
//				dst.set(x, y, m0);
//			}
//			else
//			{
//				dst.set(x, y, m1);
//			}
//		}
//	}
//}

void recomputePalette(ImageBase &src, std::vector<unsigned char> indexedImage, std::vector<U8ColorRGB> palette)
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

void applyPalette(ImageBase &src, std::vector<unsigned char> indexedImage, std::vector<U8ColorRGB> palette)
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

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "Usage: input output" << endl;
		return 1;
	}

	string inputFilename(argv[1]);
	string outputFilename(argv[2]);

	ImageBase inputImage;
	inputImage.load(argv[1]);

	std::vector<U8ColorRGB> colorPalette(2);
	computeInitial2Means(inputImage, colorPalette[0], colorPalette[1]);

	std::vector<unsigned char> compressedImageData(inputImage.getWidth() * inputImage.getHeight());

	for (int i = 0; i < 5; i++)
	{
		applyPalette(inputImage, compressedImageData, colorPalette);
		recomputePalette(inputImage, compressedImageData, colorPalette);
	}
	
	ImageBase outputImage(inputImage.getWidth(), inputImage.getHeight(), inputImage.getColor());
	for (int y = 0; y < outputImage.getHeight(); ++y)
	{
		for (int x = 0; x < outputImage.getWidth(); ++x)
		{
			unsigned char colorIndex = compressedImageData[y * outputImage.getWidth() + x];
			outputImage(x, y) = colorPalette[colorIndex];
		}
	}
	outputImage.save(argv[2]);

	return 0;
}