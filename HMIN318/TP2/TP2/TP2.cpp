
#include "CImg.h"
using namespace cimg_library;

int main(int argc, char** argv)
{
	// Load Img
	CImg<unsigned short> img;
	float voxelsize[3];
	img.load_analyze(argv[1], voxelsize);
	int dim[] = { img.width(),img.height(),img.depth() };

	/* 3. Computation based on image loop macro */
	/* See http://cimg.sourceforge.net/reference/group__cimg__loops.html */
	cimg_for(img, ptr, unsigned short)
	{

	}

}