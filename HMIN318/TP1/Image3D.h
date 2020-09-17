#pragma once

#include <vector>
#include <functional>
#include <direct.h> // _getcwd

namespace MedicalImage {
	enum Axis {
		X = 0,
		Y = 1,
		Z = 2
	};

	enum VisualisationMode {
		MIP = 1,
		AIP = 2,
		MinIP = 3
	};

	class Image3D
	{
	private:
		std::vector<unsigned short> _data;
		int _dimensions[3];
		float _voxelSize[3];
		unsigned short _min, _max;

		inline void endian_swap(unsigned short &x)
		{
			x = (x >> 8) | ((x & 0x00FF) << 8);
		}

		// i and j are the axis following axis in the direct order
		// Axis |   i |   j |
		//    X |   Y |   Z |
		//    Y |   Z |   X |
		//    Z |   X |   Y |
		void ForEach(const Axis axis, int i, int j, std::function<void(unsigned short &)> func);

	public:
		Image3D(const char* name);
		Image3D(const int dimensions[3], const float voxelSize[3]);

		unsigned short min() { return _min; }
		unsigned short max() { return _max; }
		int width() { return _dimensions[0]; }
		int height() { return _dimensions[1]; }
		int depth() { return _dimensions[2]; }

		unsigned short &operator() (int x, int y, int z)
		{
			return _data[z * _dimensions[0] * _dimensions[1] + y * _dimensions[0] + x];
		}

		Image3D ComputeVisualisation(const Image3D &source, const Axis axis, const VisualisationMode mode);

		void Save(const char* name, bool littleEndian);
	};
}