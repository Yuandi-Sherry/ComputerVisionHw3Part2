//
//  canny.h
//  Canny Edge Detector
//
//  Created by Hasan Akgün on 21/03/14.
//  Copyright (c) 2014 Hasan Akgün. All rights reserved.
//

#ifndef _CANNY_
#define _CANNY_
#include "CImg.h"
#include <vector>
#define cimg_OS 2
#define M_PI 3.14159265358979323846


using namespace cimg_library;
using namespace std;

class canny {
private:
	CImg<unsigned char> img; //Original Image
	CImg<unsigned char> grayscaled; // Grayscale
	CImg<unsigned char> gFiltered; // Gradient
	CImg<unsigned char> sFiltered; //Sobel Filtered
	CImg<unsigned char> angles; //Angle Map
	CImg<unsigned char> non; // Non-maxima supp.
	CImg<unsigned char> thres; //Double threshold and final
	CImg<unsigned char> link;
	CImg<unsigned char> adaptiveThres;
	int cannyLow;
	int cannyHigh;
	double percentage;
	int smallLength;
public:
	canny(CImg<unsigned char> originalImg, double, int); //Constructor
	CImg<unsigned char> toGrayScale();
	vector<vector<double>> createFilter(int, int, double); //Creates a gaussian filter
	CImg<unsigned char> useFilter(CImg<unsigned char>, vector<vector<double>>); //Use some filter
	CImg<unsigned char> sobel(); //Sobel filtering
	CImg<unsigned char> nonMaxSupp(CImg<unsigned char>); //Non-maxima supp.
	CImg<unsigned char> threshold(CImg<unsigned char>, int, int); //Double threshold and finalize picture
	CImg<unsigned char> linkFinal(CImg<unsigned char> imgin, int low, int high);
	CImg<unsigned char> getResult();
	void adaptiveThreshold();
};

#endif
