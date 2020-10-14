// TP2 - Changement d'espace couleur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>
#include <array>

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

double PSNRRGB(ImageBase &src, ImageBase &cmp)
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
			eqm[0] += static_cast<double>(std::abs(sr - cr)) * static_cast<double>(std::abs(sr - cr));
			eqm[1] += static_cast<double>(std::abs(sg - cg)) * static_cast<double>(std::abs(sg - cg));
			eqm[2] += static_cast<double>(std::abs(sb - cb)) * static_cast<double>(std::abs(sb - cb));
		}
	}
	eqm[0] *= mn_inv;
	eqm[1] *= mn_inv;
	eqm[2] *= mn_inv;

	return 10.0 * std::log10(255.0 * 255.0 / ((eqm[0] + eqm[1] + eqm[2]) / 3.0));
}

double PSNR(vector<double> &src, vector<double> &cmp)
{
	unsigned int length = src.size();
	double eqm = 0.0;
	for (int i = 0; i <  length; i++)
	{
		double diff = src[i] - cmp[i];
		eqm += diff * diff;
	}
	eqm *= 1.0 / length;
	return 10.0 * std::log10(1.0 / eqm);
}

double Convolve(const vector<double> &input, const vector<double> &filter, const int &n)
{
	double sum = 0.0;
	for (unsigned i = 0; i < filter.size(); i++)
	{
		int k = n - i + 1;
		if (k >= 0 && k < input.size())
		{
			sum += input[k] * filter[i];
		}
		else if (k < 0)
		{
			sum += input[0] * filter[i];
		}
		else
		{
			sum += input[input.size() - 1] * filter[i];
		}
	}
	return sum;
}

void Transpose(const vector<double>& input, unsigned int width, unsigned int height, vector<double>& output)
{
	output.clear();
	output.resize(width * height);
	for (unsigned i = 0; i < width * height; i++) {
		unsigned int row = i / width;
		unsigned int col = i % width;
		output[height * col + row] = input[i];
	}
}

void Convolve(const vector<double> &input, const vector<double> &filter, vector<double> &output)
{
	int length = input.size();
	output.resize(length);
	for (unsigned int i = 0; i < length; i++)
	{
		// Convolve with i * 2
		output[i] = Convolve(input, filter, i);
	}
}

void ConvolveAndAdd(const vector<double>& input, const vector<double>& filter, vector<double>& output)
{
	int length = input.size();
	for (unsigned int i = 0; i < length; i++)
	{
		// Convolve with i * 2
		output[i] += Convolve(input, filter, i);
	}
}

void Upsample(const vector<double> &input, vector<double>& output)
{
	unsigned int length = input.size() << 1;
	output.resize(length);
	for (unsigned int i = 0; i < length; i++)
	{
		output[i] = input[i >> 1];
	}
}

void ConvolveWithDownsampling(const vector<double>& input, const vector<double>& filter, vector<double>& output)
{
	// Length / 2
	int length = input.size() >> 1;
	output.resize(length);
	for (unsigned int i = 0; i < length; i++)
	{
		// Convolve with i * 2
		output[i] = Convolve(input, filter, i << 1);
	}
}

void ImageDataToNormalizedVector(ImageBase &src, vector<double> &output)
{
	unsigned char* data = src.getData();
	unsigned int size = src.getTotalSize();

	output.clear();
	output.reserve(size);

	for (unsigned int i = 0; i < size; i++)
	{
		output.push_back(static_cast<double>(data[i]) / 255.0 - 0.5);
	}
}

void NormalizedVectorToImage(ImageBase& dest, vector<double> &data)
{
	unsigned char* imgData = dest.getData();
	for (unsigned i = 0; i < data.size(); i++)
	{
		imgData[i] = static_cast<unsigned char>((data[i] + 0.5) * 255.0);
	}
}

void WaveletDecomposition(const vector<double> &input, array<vector<double>, 4> &output,
	unsigned int width, unsigned int height, const vector<double> &lowPassFilter, const vector<double> &highPassFilter)
{
	array<vector<double>, 2> doubleBuffer;

	// LL | HL
	// ---+---
	// LH | HH
	
	// output => { LL, HL, LH, HH }

	// LOW PASS
	// =====================================================================
	// Horizontal Convolution
	ConvolveWithDownsampling(input, lowPassFilter, doubleBuffer[1]);
	// Transpose for vertical Convolution
	Transpose(doubleBuffer[1], width / 2, height, doubleBuffer[0]);
	// Low-Low
	ConvolveWithDownsampling(doubleBuffer[0], lowPassFilter, doubleBuffer[1]);
	Transpose(doubleBuffer[1], width / 2, height / 2, output[0]);
	// Low-High
	ConvolveWithDownsampling(doubleBuffer[0], highPassFilter, doubleBuffer[1]);
	Transpose(doubleBuffer[1], width / 2, height / 2, output[2]);

	// HIGH PASS
	// =====================================================================
	// Horizontal Convolution
	ConvolveWithDownsampling(input, highPassFilter, doubleBuffer[1]);
	// Transpose for vertical Convolution
	Transpose(doubleBuffer[1], width / 2, height, doubleBuffer[0]);
	// High-Low
	ConvolveWithDownsampling(doubleBuffer[0], lowPassFilter, doubleBuffer[1]);
	Transpose(doubleBuffer[1], width / 2, height / 2, output[1]);
	// High-High
	ConvolveWithDownsampling(doubleBuffer[0], highPassFilter, doubleBuffer[1]);
	Transpose(doubleBuffer[1], width / 2, height / 2, output[3]);
}

void WaveletRecomposition(const array<vector<double>, 4> &decomposition, vector<double> &output,
	unsigned int width, unsigned int height, const vector<double>& lowPassFilter, const vector<double>& highPassFilter)
{
	vector<double> recomposedLow;
	vector<double> recomposedHigh;

	array<vector<double>, 2> doubleBuffer;

	// LL | HL
	// ---+---
	// LH | HH

	// decomposition => { LL, HL, LH, HH }
	
	// LOW PASS RECOMPOSITION
	// =====================================================================
	// Vertical Upsample and convolution
	// LL -> L
	Transpose(decomposition[0], width / 2, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	Convolve(doubleBuffer[0], lowPassFilter, recomposedLow);

	// LH -> L
	Transpose(decomposition[2], width / 2, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	ConvolveAndAdd(doubleBuffer[0], highPassFilter, recomposedLow);

	// HIGH PASS RECOMPOSITION
	// =====================================================================
	// Vertical Upsample and convolution
	// HL -> H
	Transpose(decomposition[1], width / 2, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	Convolve(doubleBuffer[0], lowPassFilter, recomposedHigh);

	// HH -> H
	Transpose(decomposition[3], width / 2, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	ConvolveAndAdd(doubleBuffer[0], highPassFilter, recomposedHigh);

	// FINAL RECOMPOSITION
	// =====================================================================
	// Horizontal Upsample and convolution for now
	// Low 
	Transpose(recomposedLow, width, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	Convolve(doubleBuffer[0], lowPassFilter, output);

	// Low 
	Transpose(recomposedHigh, width, height / 2, doubleBuffer[1]);
	Upsample(doubleBuffer[1], doubleBuffer[0]);
	ConvolveAndAdd(doubleBuffer[0], highPassFilter, output);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "Usage: input output" << endl;
		return 1;
	}

	ImageBase inputImage;
	inputImage.load(argv[1]);

	unsigned int width = inputImage.getWidth();
	unsigned int height = inputImage.getHeight();

	array<ImageBase, 4> decompositionImages{
			ImageBase(width / 2, height / 2, false),
			ImageBase(width / 2, height / 2, false),
			ImageBase(width / 2, height / 2, false),
			ImageBase(width / 2, height / 2, false)
	};

	// Haar
	const vector<double> kHaarLowPass{ 0.5, 0.5 };
	const vector<double> kHaarHighPass{ -0.5, 0.5 };

	const vector<double> kInverseHaarLowPass{ 0.5, 0.5 };
	const vector<double> kInverseHaarHighPass{ 0.5, -0.5 };

	// Sym4
	const vector<double>kSym4LowPass{
		-0.04098405819 * 1.25,
		-0.016030789   * 1.25,
		0.26917758891  * 1.25,
		0.43476757085  * 1.25,
		0.16112065016  * 1.25,
		-0.05367097188 * 1.25,
		-0.00681788234 * 1.25,
		0.01743048862  * 1.25
	};

	const vector<double>kSym4HighPass{
		-0.01743048862  * 1.25,
		-0.00681788234  * 1.25,
		0.05367097188   * 1.25,
		0.16112065016   * 1.25,
		- 0.43476757085 * 1.25,
		0.26917758891   * 1.25,
		0.016030789     * 1.25,
		- 0.04098405819 * 1.25
	};

	const vector<double>kInverseSym4LowPass{
		0.01743048862  * 1.25,
		-0.00681788234 * 1.25,
		-0.05367097188 * 1.25,
		0.16112065016  * 1.25,
		0.43476757085  * 1.25,
		0.26917758891  * 1.25,
		-0.016030789   * 1.25,
		-0.04098405819 * 1.25
	};

	const vector<double>kInverseSym4HighPass{
		-0.04098405819 * 1.25,
		0.016030789    * 1.25,
		0.26917758891  * 1.25,
		-0.43476757085 * 1.25,
		0.16112065016  * 1.25,
		0.05367097188  * 1.25,
		-0.00681788234 * 1.25,
		-0.01743048862 * 1.25
	};

	vector<double> inputSignal;
	ImageDataToNormalizedVector(inputImage, inputSignal);

	array<vector<double>, 4> decomposition;
	WaveletDecomposition(inputSignal, decomposition, width, height, kSym4LowPass, kSym4HighPass);

	//decomposition[3].assign(decomposition[3].size(), 0.0);

	vector<double> recomposedSignal;
	WaveletRecomposition(decomposition, recomposedSignal, width, height, kInverseSym4LowPass, kInverseHaarHighPass);

	ImageBase recomposedImg(width, height, false);
	NormalizedVectorToImage(recomposedImg, recomposedSignal);
	recomposedImg.save(&string("recomposed.pgm")[0]);

	cout << "PSNR: " << PSNR(inputSignal, recomposedSignal) << endl;

	array<string, 4> filenames{
		string(argv[2]) + string("-LL.pgm"),
		string(argv[2]) + string("-HL.pgm"),
		string(argv[2]) + string("-LH.pgm"),
		string(argv[2]) + string("-HH.pgm")
	};

	for (int i = 0; i < 4; i++)
	{
		NormalizedVectorToImage(decompositionImages[i], decomposition[i]);
		decompositionImages[i].save(&filenames[i][0]);
	}


	// YCrCb Compression
	// To YCbCr
	//for (int y = 0; y < inputImage.getHeight(); y++)
	//{
	//	for (int x = 0; x < inputImage.getWidth(); x++)
	//	{
	//		unsigned char r, g, b;
	//		unsigned char Y, Cb, Cr;
	//		sample(inputImage, x, y, r, g, b);
	//		RGB2YCbCr(r, g, b, Y, Cb, Cr);
	//		set(inputImageYCbCr, x, y, Y, Cb, Cr);
	//	}
	//}


	//// To RGB
	//for (int y = 0; y < inputImage.getHeight(); y++)
	//{
	//	for (int x = 0; x < inputImage.getWidth(); x++)
	//	{
	//		unsigned char Y, Cb, Cr;
	//		unsigned char r, g, b;
	//		sample(outputImage, x, y, Y, Cb, Cr);
	//		YCbCr2RGB(Y, Cb, Cr, r, g, b);
	//		set(outputImage, x, y, r, g, b);
	//	}
	//}

	

	//outputImage.save(argv[2]);
	//inputImageYCbCr.save(argv[3]);

	//std::cout << psnr(inputImage, outputImage) << std::endl;

}

