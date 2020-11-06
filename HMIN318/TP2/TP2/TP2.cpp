#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <sstream>

#include "CImg.h"

using namespace cimg_library;

struct Vec3
{
public:
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
};

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

void ComputeCellBarycenters(
    CImg<unsigned short>& image,
    const std::array<float, 3> &voxelSize,
    std::vector<Vec3> &barycenters
)
{
    // Apply a median filter to remove noise
    image.blur_median(5);
    // Automatically threshold with Otsu method
    // (Implemented in TP 2)
    image.threshold(GetOtsuThreshold(image));

    // Apply morphological opening to remove bridges
    image.erode(3);
    image.dilate(3);

    // Get connected components
    auto labels = image.get_label();

    barycenters.resize(labels.max() + 1, Vec3());
    std::vector<unsigned int> barycenterCounts(barycenters.size(), 0);

    image.display("Labels");

    // Compute barycenters
    cimg_forXYZ(labels, x, y, z)
    {
        auto label = labels(x, y, z);
        barycenterCounts[label]++;
        barycenters[label].x += x * voxelSize[0];
        barycenters[label].y += y * voxelSize[1];
        barycenters[label].z += z * voxelSize[2];
    }

    for (int i = 0; i < barycenters.size(); i++)
    {
        unsigned int count = barycenterCounts[i];
        if (count > 0)
        {
            barycenters[i].x /= count;
            barycenters[i].y /= count;
            barycenters[i].z /= count;
        }
    }
}


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "Usage: <filename> <image count>" << std::endl;
		return EXIT_FAILURE;
	}

    unsigned int imageCount = atoi(argv[2]);

	// Load Images
	std::vector<CImg<unsigned short>> images(imageCount);

	std::array<float, 3> voxelsize;

    for (unsigned int i = 0; i < imageCount; i++)
    {
        std::stringstream name;
        name << argv[1] << "-" << i << ".img";
        images[i].load_analyze(name.str().c_str(), voxelsize.data());
    }

	int dim[] = { images[0].width(), images[0].height(), images[0].depth() };

    std::vector<Vec3> barycenters;
    ComputeCellBarycenters(images[0], voxelsize, barycenters);

    //for (auto vec : barycenters)
    //{
    //    std::cout << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
    //}

    //for (unsigned int i = 0; i < imageCount; i++)
    //{

    //    images[i].display("Stack");
    //}

    //unsigned short threshold = GetOtsuThreshold(img);
	//unsigned int erodeCount = atoi(argv[2]);
	//unsigned int dilateCount = atoi(argv[3]);

	//CImg<unsigned short> binarizedImg = img.get_threshold(threshold);

	//for (int i = 0; i < erodeCount; i++)
	//{
	//	binarizedImg.erode(2);
	//}

	//for (int i = 0; i < dilateCount; i++)
	//{
	//	binarizedImg.dilate(2);
	//}

	return EXIT_SUCCESS;
}