#include <iostream>
#include <cmath>
#include <vector>

#include "CImg.h"

using namespace cimg_library;

unsigned short GetOtsuThreshold(const CImg<unsigned short> &img)
{
    // Calculate the threshold using the method:
    // A threshold selection method from gray-level histograms,
    // N. Otsu, 1975, Automatica

    double mean   = 0.0;
    double expect = 0.0; // mu
    double cumul  = 0.0; // omega
    double maxVar = 0.0; // sigma_B

    // C6262: Use the heap, too much data for the stack
    std::vector<double> hist(USHRT_MAX, 0.0F);

    unsigned int voxelCount = img.size();
    unsigned short threshold = 0;

    // ==== Compute hist ====
    cimg_for(img, ptr, unsigned short)
    {
        hist[*ptr]++;
    }

    for (int i = 0; i < USHRT_MAX; i++) {
        hist[i] /= voxelCount;
        mean += i * hist[i];
    }

    // ==== Threshold ====
    for (int i = 0; i < USHRT_MAX; i++) {
        cumul += hist[i];
        expect += i * hist[i];

        if (cumul >= 1.0) break;

        double var = std::pow(mean * cumul - expect, 2) / (cumul * (1 - cumul));

        if (var >= maxVar) {
            threshold = i;
            maxVar = var;
        }
    }

    return threshold;
}


int main(int argc, char** argv)
{
	if (argc < 4)
	{
		std::cerr << "Usage: <erodeCount> <dilateCount>" << std::endl;
		return EXIT_FAILURE;
	}

	// Load Img
	CImg<unsigned short> img;
	float voxelsize[3];
	img.load_analyze(argv[1], voxelsize);
	int dim[] = { img.width(), img.height(), img.depth() };

    unsigned short threshold = GetOtsuThreshold(img);
	unsigned int erodeCount = atoi(argv[2]);
	unsigned int dilateCount = atoi(argv[3]);

	CImg<unsigned short> binarizedImg = img.get_threshold(threshold);

	for (int i = 0; i < erodeCount; i++)
	{
		binarizedImg.erode(2);
	}

	for (int i = 0; i < dilateCount; i++)
	{
		binarizedImg.dilate(2);
	}

	binarizedImg.display("Binarized");

	return EXIT_SUCCESS;
}