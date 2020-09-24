/* Compilation Windows:
	g++ -o visu1.exe visu1.cpp -O2 -lgdi32

	Execution:
	visu1.exe .\data\knix.hdr

	Result:

	Voxel size=0.332000 0.332000 4.499932
	min=0   max=5145
	min=0   max=5145
	min=0   max=5145
	I(256,256,12)=986
*/

#include "CImg.h"
using namespace cimg_library;

/* Main program */
int main(int argc, char** argv)
{
	/* Create and load the 3D image */
	CImg<unsigned short> img;
	float voxelsize[3];
	/* Load in Analyze format */
	img.load_analyze(argv[1], voxelsize);

	/* Get the image dimensions */
	int dim[] = { img.width(),img.height(),img.depth() };

	printf("Reading %s. Dimensions=%d %d %d\n", argv[1], dim[0], dim[1], dim[2]);
	printf("Voxel size=%f %f %f\n", voxelsize[0], voxelsize[1], voxelsize[2]);

	/* 1. Direct values of min,max */
	printf("min=%d   max=%d\n", img.min(), img.max());

	/* 2. Computation based on a classic loop */
	int max = -1;
	int min = 100000;
	int voxel;
	for (int i = 0; i < dim[0]; i = i + 1)
	{
		for (int j = 0; j < dim[1]; j = j + 1)
		{
			for (int k = 0; k < dim[2]; k = k + 1)
			{
				voxel = img(i, j, k, 0);

				if (voxel < min)
				{
					min = voxel;
				}
				if (voxel > max)
				{
					max = voxel;
				}
			}
		}
	}
	printf("  min=%d   max=%d\n", min, max);

	/* 3. Computation based on image loop macro */
	/* See http://cimg.sourceforge.net/reference/group__cimg__loops.html */
	cimg_for(img, ptr, unsigned short)
	{
		if (*ptr < min)
		{
			min = *ptr;
		}
		if (*ptr > max)
		{
			max = *ptr;
		}
	}
	printf("    min=%d   max=%d\n", min, max);

	/* Value of a given voxel */
	printf("I(256,256,12)=%d\n", img(256, 256, 12));

}