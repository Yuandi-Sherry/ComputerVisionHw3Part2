//
//  canny.cpp
//  Canny Edge Detector
//
//  Created by Hasan Akgün on 21/03/14.
//  Copyright (c) 2014 Hasan Akgün. All rights reserved.
//

#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <queue>
#include "canny.h"
#define cimg_OS 2
#define _USE_MATH_DEFINES
#define M_PI 3.14159265358979323846

using namespace cimg_library;
using namespace std;


canny::canny(CImg<unsigned char> origin, double percentage, int smallLength)
{
	img = origin;
	this->smallLength = smallLength;
	this->percentage = percentage;
	if (!img) {
		cout << "Could not open or find the image" << std::endl;
	}
	else {
		vector<vector<double>> filter = createFilter(3, 3, 1);
		// 原图尺寸 480 * 480
		grayscaled = toGrayScale();//Grayscale the image
		// 478 * 478
		gFiltered = useFilter(grayscaled, filter); //Gaussian Filter
		// 476 * 476
		sFiltered = sobel(); //Sobel Filter
		// 474 * 474
		non = nonMaxSupp(sFiltered); //Non-Maxima Suppression
		//non.display();
		adaptiveThreshold();
		// 474 * 474
		thres = threshold(non, cannyLow, cannyHigh); //Double Threshold and Finalize
		//thres.display();
		link = linkFinal(non, cannyLow, cannyHigh);
		//link = nonMaxSupp(link);
		//link = nonMaxSupp();
		//link.display();
		link.save("edge.bmp");
	}
}

CImg<unsigned char> canny::toGrayScale() {
	CImg<unsigned char> grayscaled(img.width(), img.height(), img.depth());
	grayscaled = img;
	cimg_forXY(grayscaled, i, j) {
		int r = grayscaled(i, j, 0, 0);
		int g = grayscaled(i, j, 0, 1);
		int b = grayscaled(i, j, 0, 2);
		int gray = (int)(r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayscaled(i, j, 0, 0) = gray;
		grayscaled(i, j, 0, 1) = gray;
		grayscaled(i, j, 0, 2) = gray;
	}
	return grayscaled;
}

vector<vector<double>> canny::createFilter(int row, int column, double sigmaIn) {
	vector<vector<double>> temp;
	vector<vector<double>> filter;
	for (int i = 0; i < row; i++) {
		vector<double> col;
		for (int j = 0; j < column; j++) {
			col.push_back(-1);
		}
		filter.push_back(col);
	}
	float coordSum = 0;
	float constant = 2.0 * sigmaIn * sigmaIn;
	// Sum is for normalization
	float sum = 0.0;
	// compute Gaussian
	for (int x = -row / 2; x <= row / 2; x++) {
		for (int y = -column / 2; y <= column / 2; y++) {
			coordSum = (x*x + y * y);
			filter[x + row / 2][y + column / 2] = (exp(-(coordSum) / constant)) / (M_PI * constant);
			sum += filter[x + row / 2][y + column / 2];
		}
	}
	// Normalize the Filter
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			filter[i][j] /= sum;
		}
	}
	return filter;
}

CImg<unsigned char> canny::useFilter(CImg<unsigned char> img_in, vector<vector<double>> filterIn) {
	int size = (int)filterIn.size() / 2;
	CImg<unsigned char> filteredImg = CImg<unsigned char>(img_in.width() - 2 * size, img_in.height() - 2 * size);
	for (int i = size; i < img_in.width() - size; i++) {
		for (int j = size; j < img_in.height() - size; j++) {
			double sum = 0;
			for (int x = 0; x < filterIn.size(); x++) {
				for (int y = 0; y < filterIn.size(); y++) {
					sum += filterIn[x][y] * (double)(img_in(i + x - size, j + y - size));
				}
			}
			filteredImg(i - size, j - size) = sum;
		}
	}
	return filteredImg;
}

CImg<unsigned char> canny::sobel() {
	// construct 2 matrix
	//Sobel X Filter
	double x1[] = { -1.0, 0, 1.0 };
	double x2[] = { -2.0, 0, 2.0 };
	double x3[] = { -1.0, 0, 1.0 };
	vector<vector<double>> xFilter(3);
	xFilter[0].assign(x1, x1 + 3);
	xFilter[1].assign(x2, x2 + 3);
	xFilter[2].assign(x3, x3 + 3);

	//Sobel Y Filter
	double y1[] = { 1.0, 2.0, 1.0 };
	double y2[] = { 0, 0, 0 };
	double y3[] = { -1.0, -2.0, -1.0 };
	vector<vector<double>> yFilter(3);
	yFilter[0].assign(y1, y1 + 3);
	yFilter[1].assign(y2, y2 + 3);
	yFilter[2].assign(y3, y3 + 3);

	//Limit Size
	// 定位矩阵在整个图像上移动的坐标范围
	int size = (int)xFilter.size() / 2; // size即为需要留出的修改坐标和图像边缘的最小距离，否则会导致数组vector越界
	CImg<unsigned char> filteredImg = CImg<unsigned char>(gFiltered.width() - 2 * size, gFiltered.height() - 2 * size);
	//gFiltered
	angles = CImg<unsigned char>(gFiltered.width() - 2 * size, gFiltered.height() - 2 * size); //AngleMap
	for (int i = size; i < gFiltered.width() - size; i++) {
		for (int j = size; j < gFiltered.height() - size; j++) {
			double sumx = 0;
			double sumy = 0;
			// 对图像上每个像素使用Sobel算子，即对应3*3矩阵上图像像素的加权求和
			for (int x = 0; x < xFilter.size(); x++) {
				for (int y = 0; y < xFilter.size(); y++) {
					sumx += xFilter[x][y] * (double)(gFiltered(i + x - size, j + y - size)); //Sobel_X Filter Value
					sumy += yFilter[x][y] * (double)(gFiltered(i + x - size, j + y - size)); //Sobel_Y Filter Value
				}
			}
			double sumxsq = sumx * sumx;
			double sumysq = sumy * sumy;
			double sq2 = sqrt(sumxsq + sumysq); // 记录梯度模
			if (sq2 > 255) {//Unsigned Char Fix
				sq2 = 255;
			}
			filteredImg(i - size, j - size) = sq2;
			// 记录梯度方向
			if (sumx == 0) { //Arctan Fix
				angles(i - size, j - size) = 90;
			}
			else {
				angles(i - size, j - size) = atan(sumy / sumx);
			}
		}
	}
	return filteredImg;
}


CImg<unsigned char> canny::nonMaxSupp(CImg<unsigned char> sFiltered) {
	CImg<unsigned char> nonMaxSupped = CImg<unsigned char>(sFiltered.width() - 2, sFiltered.height() - 2);
	// 与四个边缘都留出宽为1的像素边框
	for (int i = 1; i < sFiltered.width() - 1; i++) {
		for (int j = 1; j < sFiltered.height() - 1; j++) {
			float Tangent = angles(i, j);
			// 先将非最大化抑制后的图像从(0,0)开始的当前像素设定为右下像素在sFilter中的模长
			nonMaxSupped(i - 1, j - 1) = sFiltered(i, j);
			// 判定沿上下左右斜四方向
			//Horizontal Edge
			// 如果该像素的模长比左右都大或相同，则为边缘，根据德摩根定律可以得到下列if的判定条件
			if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5))) {
				if ((sFiltered(i, j) < sFiltered(i, j + 1)) || (sFiltered(i, j) < sFiltered(i, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
			//Vertical Edge
			if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5))) {
				if ((sFiltered(i, j) < sFiltered(i + 1, j)) || (sFiltered(i, j) < sFiltered(i - 1, j)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//-45 Degree Edge
			if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5))) {
				if ((sFiltered(i, j) < sFiltered(i - 1, j + 1)) || (sFiltered(i, j) < sFiltered(i + 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}

			//45 Degree Edge
			if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((22.5 < Tangent) && (Tangent <= 67.5))) {
				if ((sFiltered(i, j) < sFiltered(i + 1, j + 1)) || (sFiltered(i, j) < sFiltered(i - 1, j - 1)))
					nonMaxSupped(i - 1, j - 1) = 0;
			}
		}
	}
	return nonMaxSupped;
}

CImg<unsigned char> canny::threshold(CImg<unsigned char> imgin, int low, int high)
{
	if (low > 255)
		low = 255;
	if (high > 255)
		high = 255;

	CImg<unsigned char> EdgeMat = CImg<unsigned char>(imgin.width(), imgin.height());
	//cout << "thres" << endl;
	for (int i = 0; i < imgin.width(); i++) {
		for (int j = 0; j < imgin.height(); j++) {
			EdgeMat(i, j) = imgin(i, j);
			if (EdgeMat(i, j) > high)
				EdgeMat(i, j) = 255;
			else if (EdgeMat(i, j) < low)
				EdgeMat(i, j) = 0;
			else {
				bool anyHigh = false;
				bool anyBetween = false;
				// 检测周围3*3 - 9个像素点
				for (int x = i - 1; x < i + 2; x++) {
					for (int y = j - 1; y < j + 2; y++) {
						if (x < 0 || y < 0 || x >= EdgeMat.width() || y >= EdgeMat.height()) //Out of bounds
							continue;
						else {
							//cout << "强边缘" << endl;
							if (EdgeMat(x, y) > high) { // 将> 修改为 >=
								//cout << "强边缘" << endl;
								EdgeMat(i, j) = 255;
								anyHigh = true;
								break;
							}
							// 弱边缘
							else if (EdgeMat(x, y) <= high && EdgeMat(x, y) >= low) {
								//cout << "弱边缘" << endl;
								anyBetween = true;
							}

						}
					}
					// 如果出现一个强边缘则直接停止周围检测
					if (anyHigh) {
						break;
					}

				}
				// 弱边缘 - 周围存在弱边缘且没有强边缘
				if (!anyHigh && anyBetween) {
					//cout << "anyBetween" << endl;
					// 检测周围20个像素点，与上边步骤相同
					for (int x = i - 2; x < i + 3; x++) {
						for (int y = j - 1; y < j + 3; y++) {
							if (x < 0 || y < 0 || x > EdgeMat.width() || y > EdgeMat.height()) //Out of bounds
								continue;
							else {
								if (EdgeMat(x, y) > high) {
									EdgeMat(i, j) = 255;
									anyHigh = true;
									break;
								}
							}
						}
						if (anyHigh)
							break;
					}
				}
				// 否则抑制
				if (!anyHigh)
					EdgeMat(i, j) = 0;
			}
		}
	}
	return EdgeMat;
}

CImg<unsigned char> canny::linkFinal(CImg<unsigned char> imgin, int low, int high) {
	//cout << "link" << endl;
	CImg<unsigned char> FinalImg = CImg<unsigned char>(imgin.width(), imgin.height());
	CImg<unsigned char> FlagMat = CImg<unsigned char>(imgin.width(), imgin.height());

	int count = 0;
	int minus = 0;
	for (int i = 0; i < imgin.width(); i++) {
		for (int j = 0; j < imgin.height(); j++) {
			FinalImg(i, j) = thres(i, j);
			FlagMat(i, j) = 0;
		}
	}
	bool connected = false;
	stack<pair<int, int> > pixelStack; // 用于DFS，存储弱边缘
	queue<pair<int, int> > pixelQueue; // 记录组成边缘的点
	/*for (int i = 1; i < imgin.width() - 1; i++) {
		for (int j = 1; j < imgin.height() - 1; j++) {
			if (non(i, j) < high && non(i, j) > low) { // 该像素是弱边缘
				//cout << "link2" << endl;
				pixelStack.push(make_pair(i, j));
				pixelQueue.push(make_pair(i, j)); // 有可能会变成强边缘
				FlagMat(i, j) = 1;
				while (!pixelStack.empty()) {
					pair<int, int> tempPixel = pixelStack.top();
					pixelStack.pop();
					// 8邻域，看是否存在强/弱边缘
					for (int x = tempPixel.first - 1; x <= tempPixel.first + 1; x++) {
						for (int y = tempPixel.second - 1; y <= tempPixel.second + 1; y++) {
							if (x < 0 || y < 0 || x >= FlagMat.width() || y >= FlagMat.height()) {
								continue;
							}

							if (non(x, y) < high && non(x, y) > low && FlagMat(x, y) == 0) { // 该像素是弱边缘，且未被标记
								pixelStack.push(make_pair(x, y));
								pixelQueue.push(make_pair(x, y)); // 有可能会变成强边缘
								FlagMat(x, y) = 1; // 标记
							}
							if (FinalImg(x, y) == 255) { // 如果周围有强边缘
								connected = true;
							}
						}
					}
				}
				if (connected) {
					while (!pixelQueue.empty()) {
						pair<int, int> tempPixel = pixelQueue.front();
						pixelQueue.pop();
						//FlagMat(tempPixel.first, tempPixel.second) = 0; // 为了使得经过原来弱边缘的其他边缘仍然能够相连，则恢复未标记状态
						FinalImg(tempPixel.first, tempPixel.second) = 255;
						count++;
					}
					connected = false;
				}
				else {
					while (!pixelQueue.empty()) {
						pair<int, int> tempPixel = pixelQueue.front();
						pixelQueue.pop();
						FlagMat(tempPixel.first, tempPixel.second) = 0;
					}
				}
			}
		}
	}*/

	// 去掉长度 < 20的边缘
	for (int i = 0; i < imgin.width(); i++) {
		for (int j = 0; j < imgin.height(); j++) {
			FlagMat(i, j) = 0;
		}
	}
	for (int i = 0; i < imgin.width(); i++) {
		for (int j = 0; j < imgin.height(); j++) {
			if (FinalImg(i, j) == 255) { // 该像素为强边缘点
				if (FlagMat(i, j) == 0) {
					FlagMat(i, j) = 1; // 标记边缘点
					pixelStack.push(make_pair(i, j));
					pixelQueue.push(make_pair(i, j));
					int length = 1;
					while (!pixelStack.empty()) {
						pair<int, int> tempPixel = pixelStack.top();
						pixelStack.pop();
						for (int x = tempPixel.first - 1; x <= tempPixel.first + 1; x++) {
							for (int y = tempPixel.second - 1; y <= tempPixel.second + 1; y++) {
								if (x < 0 || y < 0 || x >= FlagMat.width() || y >= FlagMat.height()) {

								}
								else if (FinalImg(x, y) == 255 && FlagMat(x, y) == 0) { // 周围有强边缘，则开始计数

									pixelStack.push(make_pair(x, y));
									pixelQueue.push(make_pair(x, y));
									FlagMat(x, y) = 1;
									length++;
								}
							}
						}
					}
					if (length < smallLength) {

						while (!pixelQueue.empty()) {
							pair<int, int> tempPixel = pixelQueue.front();
							pixelQueue.pop();
							minus++;
							FinalImg(tempPixel.first, tempPixel.second) = 0;
						}
					}
					else {

						while (!pixelQueue.empty()) {
							pair<int, int> tempPixel = pixelQueue.front();
							pixelQueue.pop();
						}
					}
				}
			}
		}
	}
	cout << "新描了" << count << "个点" << endl;
	cout << "去掉了" << minus << "个点" << endl;
	return FinalImg;
}

void canny::adaptiveThreshold() {
	CImg<unsigned char> adaptiveImg = CImg<unsigned char>(non.width(), non.height());
	// 计算梯度直方图
	int* ranges = new int[256];
	// init
	for (int i = 0; i < 256; i++) {
		ranges[i] = 0;
	}
	// 计数
	for (int i = 0; i < sFiltered.width(); i++) {
		for (int j = 0; j < sFiltered.height(); j++) {
			ranges[sFiltered(i, j)]++;
		}
	}
	double PercentOfPixelsNotEdges = percentage;
	int total = PercentOfPixelsNotEdges * non.width()*non.height(); // 边缘点的总数

	int low = 0, sum = 0;
	for (low = 0; low < 256; low++) {
		sum += ranges[low];
		if (sum > total) {
			break;
		}
	}
	cannyLow = (low + 1);
	cannyHigh = (int)(2.5 * cannyLow);
	cout << "cannyLow" << cannyLow << "cannyHigh" << cannyHigh << endl;
	delete[] ranges;
}

CImg<unsigned char> canny::getResult()
{
	CImg<unsigned char> FinalImg = CImg<unsigned char>(link.width(), link.height());
	for (int i = 0; i < link.width(); i++) {
		for (int j = 0; j < link.height(); j++) {
			FinalImg(i, j) = link(i, j);
		}
	}
	return FinalImg;
}