#include "pch.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>

#if defined(_WIN32)
	#include <filesystem>
#endif

#include "Image3D.h"


void MedicalImage::Image3D::ForEach(const Axis axis, int i, int j, std::function<void(unsigned short &)> func)
{
	switch (axis)
	{
		case X:
			for (int x = 0; x < _dimensions[0]; x++)
			{
				func((*this)(x, i, j));
			}
			break;
		case Y:
			for (int y = 0; y < _dimensions[1]; y++)
			{
				func((*this)(j, y, i));
			}
			break;
		case Z:
			for (int z = 0; z < _dimensions[2]; z++)
			{
				func((*this)(i, j, z));
			}
			break;
	}
}

MedicalImage::Image3D::Image3D(const char * name)
{
	sscanf(name, "%*[^.].%dx%dx%d.%fx%fx%f.%*s", _dimensions, _dimensions+1, _dimensions+2, _voxelSize, _voxelSize+1, _voxelSize+2);

	// Open the file at the end and get position
	std::ifstream imageFile;
	imageFile.open(name, std::ios::in | std::ios::binary | std::ios::ate);

	if (!imageFile) {  // replace this bloc
		std::cout << name << " is not a valid file\n";  // print filename to find out any issues (truncated name, etc...) 
		std::cout << "Error code: " << strerror(errno) << std::endl; // Get some system info as to why
#if defined(_WIN32)
		char cwd[512];                           // print current working directory. 
		_getcwd(cwd, sizeof(cwd));                // in case your path is relative 
		std::cout << "Current directory is " << cwd << std::endl;
#endif
		std::cout << "Try again !\n";
	}


	std::ifstream::pos_type pos = imageFile.tellg();

	int length = pos;
	std::cout << name << std::endl;

	_data.resize(length / 2);
	imageFile.seekg(0, std::ios::beg);
	imageFile.read((char*) &_data[0], length);

	imageFile.close();

	_min = 0xFFFF;
	_max = 0x0000;

	for (int i = 0; i < _data.size(); i++)
	{
		unsigned short &voxel = _data[i];
		endian_swap(voxel);
		if (voxel < _min) _min = voxel;
		if (voxel > _max) _max = voxel;
	}
}

MedicalImage::Image3D::Image3D(const int dimensions[3], const float voxelSize[3])
{
	_dimensions[0] = dimensions[0];
	_dimensions[1] = dimensions[1];
	_dimensions[2] = dimensions[2];
	_voxelSize[0] = voxelSize[0];
	_voxelSize[1] = voxelSize[1];
	_voxelSize[2] = voxelSize[2];

	_data.resize(dimensions[0] * dimensions[1] * dimensions[2]);
}

MedicalImage::Image3D MedicalImage::Image3D::ComputeVisualisation(const Axis axis, const VisualisationMode mode)
{
	int dim1 = (axis + 1) % 3;
	int dim2 = (axis + 2) % 3;

	int dims[3]{ _dimensions[dim1], _dimensions[dim2], _dimensions[axis] };
	float voxelSize[3]{ 1.0f, 1.0f, 1.0f };
	Image3D result(dims, voxelSize);

	switch (mode)
	{
	case MIP:
		for (int i = 0; i < _dimensions[dim1]; i++)
		{
			for (int j = 0; j < _dimensions[dim2]; j++)
			{
				unsigned short max = 0x0000;
				ForEach(axis, i, j, [&](unsigned short intensity) {
					if (intensity > max) max = intensity;
				});
				result(i, j, 0) = max;
			}
		}
		break;
	case AIP:
		for (int i = 0; i < _dimensions[dim1]; i++)
		{
			for (int j = 0; j < _dimensions[dim2]; j++)
			{
				float sum = 0.0f;
				ForEach(axis, i, j, [&](unsigned short intensity) {
					sum += intensity;
				});
				result(i, j, 0) = sum / _dimensions[axis];
			}
		}
		break;
	case MinIP:
		for (int i = 0; i < _dimensions[dim1]; i++)
		{
			for (int j = 0; j < _dimensions[dim2]; j++)
			{
				unsigned short min = 0xFFFF;
				ForEach(axis, i, j, [&](unsigned short intensity) {
					if (intensity < min) min = intensity;
				});
				result(i, j, 0) = min;
			}
		}
		break;
	}

	return result;
}

void MedicalImage::Image3D::Save(const char * name, bool littleEndian)
{
	std::ofstream outFile(name, std::ios::binary);
	if (littleEndian == false)
	{
		outFile.write((char*)&_data[0], _data.size() * 2);
	}
	else
	{
		for (const auto &voxel : _data)
		{
			unsigned short endianCorrected = voxel;
			endian_swap(endianCorrected);
			outFile.write((char*) &endianCorrected, 2);
		}
	}

	outFile.close();
}
