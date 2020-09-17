// ImagerieMedicale TP1.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "pch.h"
#include <iostream>
#include <cstdio>
#include <fstream>


#include "Image3D.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: <image>" << std::endl;
		return 1;
	}
  
	MedicalImage::Image3D image(argv[1]);

	std::cout << "Min: " << image.min() << ", Max: " << image.max() << std::endl;

	std::cout << "Intensity at 131, 157, 48 : " << image(131, 157, 48) << std::endl;

	std::string savedFilenameXMIP(argv[1]);
	savedFilenameXMIP.insert(0, "out\\X-MIP-");
	std::string savedFilenameYMIP(argv[1]);
	savedFilenameYMIP.insert(0, "out\\Y-MIP-");
	std::string savedFilenameZMIP(argv[1]);
	savedFilenameZMIP.insert(0, "out\\Z-MIP-");

	std::string savedFilenameXAIP(argv[1]);
	savedFilenameXAIP.insert(0, "out\\X-AIP-");
	std::string savedFilenameYAIP(argv[1]);
	savedFilenameYAIP.insert(0, "out\\Y-AIP-");
	std::string savedFilenameZAIP(argv[1]);
	savedFilenameZAIP.insert(0, "out\\Z-AIP-");

	std::string savedFilenameXMinIP(argv[1]);
	savedFilenameXMinIP.insert(0, "out\\X-MinIP-");
	std::string savedFilenameYMinIP(argv[1]);
	savedFilenameYMinIP.insert(0, "out\\Y-MinIP-");
	std::string savedFilenameZMinIP(argv[1]);
	savedFilenameZMinIP.insert(0, "out\\Z-MinIP-");

	image.ComputeVisualisation(MedicalImage::X, MedicalImage::MIP).Save(savedFilenameXMIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::Y, MedicalImage::MIP).Save(savedFilenameYMIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::Z, MedicalImage::MIP).Save(savedFilenameZMIP.c_str(), true);

	image.ComputeVisualisation(MedicalImage::X, MedicalImage::AIP).Save(savedFilenameXAIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::Y, MedicalImage::AIP).Save(savedFilenameYAIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::Z, MedicalImage::AIP).Save(savedFilenameZAIP.c_str(), true);

	image.ComputeVisualisation(MedicalImage::Z, MedicalImage::MinIP).Save(savedFilenameXMinIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::X, MedicalImage::MinIP).Save(savedFilenameYMinIP.c_str(), true);
	image.ComputeVisualisation(MedicalImage::Y, MedicalImage::MinIP).Save(savedFilenameZMinIP.c_str(), true);
}