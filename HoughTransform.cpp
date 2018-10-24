#include "HoughTransform.h"
#include "CImg.h"
#include "canny.h"
#include <iostream>
#include <cmath>
#include <String>

#define M_PI 3.14159265358979323846
using namespace cimg_library;
using namespace std;
HoughTransform::HoughTransform(CImg<unsigned char> inputImg, CImg<unsigned char> origin, int id, int thres) {
	this->id = id;
	img = inputImg;
	result = origin;
	width = img.width();
	height = img.height();
	double temp = width * width + height * height;
	dia = ceil(sqrt(temp));
	deltaTheta = 45;
	deltaRho = dia / 30;
	threshold = thres;

	accumulation = CImg<unsigned char>(360, dia);

	string t = to_string(id) + "edge_origin.bmp";
	const char * tempName = t.c_str();
	img.save(tempName);
	initTriangle();
	fillAccumulation();
	findLocalMaximums(threshold);
	generateLines();
	drawPoints();
}


void HoughTransform::initTriangle() {
	cout << "initTriangle" << endl;
	sinTheta = new double[360];
	cosTheta = new double[360];
	for (int i = 0; i < 360; i++) {
		sinTheta[i] = sin(i*M_PI / 180);
		cosTheta[i] = cos(i*M_PI / 180);
	}

	for (int i = 0; i < accumulation.width(); i++) {
		for (int j = 0; j < accumulation.height(); j++) {
			accumulation(i, j) = 0;
		}
	}
}

void HoughTransform::fillAccumulation() {
	cout << "fillAccumulation" << endl;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//cout << img(i, j) << endl;
			if (img(i, j) == 255) {
				for (int theta = 0; theta < 360; theta++) {
					int ro = abs(round(i*cosTheta[theta] + j * sinTheta[theta]));
					accumulation(theta, ro) = accumulation(theta, ro) + 1;

				}
			}
		}
	}
	//accumulation.display();
}

void HoughTransform::findLocalMaximums(int threshold) {
	//cout << "findLocalMaximums" << endl;
	for (int r = 0; r < dia; r++) {
		for (int theta = 0; theta < 360; theta++) {
			if (accumulation(theta, r) > threshold) {
				sortBuffer.push_back(make_pair(theta, r));
			}
		}
	}

	for (int i = 0; i < sortBuffer.size(); ) {
		boolean eraseI = false;
		for (int j = i + 1; j < sortBuffer.size(); ) {
			if ((abs(sortBuffer[i].first - sortBuffer[j].first) < deltaTheta
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 360) < deltaTheta
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 180) < deltaTheta)
				&& ((sortBuffer[i].first >= 0 && sortBuffer[i].first <= 90 || sortBuffer[i].first >= 180 && sortBuffer[i].first <= 270)
					&& (sortBuffer[j].first >= 0 && sortBuffer[j].first <= 90 || sortBuffer[j].first >= 180 && sortBuffer[j].first <= 270)
					|| (sortBuffer[i].first >= 90 && sortBuffer[i].first <= 180 || sortBuffer[i].first >= 270 && sortBuffer[i].first <= 360)
					&& (sortBuffer[j].first >= 90 && sortBuffer[j].first <= 180 || sortBuffer[j].first >= 270 && sortBuffer[j].first <= 360))) {
				if (sortBuffer[i].first == 0 || sortBuffer[i].first == 180 || sortBuffer[j].first == 0 || sortBuffer[j].first == 180) { // 其中有一条垂直线
					double dis = sortBuffer[i].second / cosTheta[sortBuffer[i].first] - sortBuffer[j].second / cosTheta[sortBuffer[j].first];
					cout << "************8dis *****************  " << dis << endl;
					
					if (abs(dis) >= deltaRho * 5) {
						j++;
						continue;
					}
				}
				else if (sortBuffer[i].first == 90 || sortBuffer[i].first == 270 || sortBuffer[j].first == 90 || sortBuffer[j].first == 270) {
					double dis = (sortBuffer[i].second / sinTheta[sortBuffer[i].first]  - sortBuffer[j].second / sinTheta[sortBuffer[j].first]);
					cout << "-----------------8dis **-----------*  " << deltaRho << endl;
					if (abs(dis) >= deltaRho * 5) {
						j++;
						continue;
					}
				}
				else if (abs(sortBuffer[i].second - sortBuffer[j].second) > deltaRho) {
					j++;
					continue;
				}
				if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
					accumulation(sortBuffer[j].first, sortBuffer[j].second) ) {
					sortBuffer.erase(sortBuffer.begin() + i);
					eraseI = true;
				}
				else if (accumulation(sortBuffer[j].first, sortBuffer[j].second) <=
					accumulation(sortBuffer[i].first, sortBuffer[i].second) ) {
					sortBuffer.erase(sortBuffer.begin() + j);
					continue;
				}
				

				
				
			}
			/*else if ((sortBuffer[i].first == 0 || sortBuffer[i].first == 180) && (sortBuffer[j].first == 0 || sortBuffer[j].first == 180)
				|| (sortBuffer[i].first == 90 || sortBuffer[i].first == 270) && (sortBuffer[j].first == 90 || sortBuffer[i].first == 270)) {
				if (sortBuffer[i].first == 90 || sortBuffer[i].first == 270) {
					double dis = sortBuffer[i].second / sinTheta[sortBuffer[i].first] * cosTheta[sortBuffer[i].first];
					if (abs(dis) < deltaRho) {
						if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
							accumulation(sortBuffer[j].first, sortBuffer[j].second)) {
							sortBuffer.erase(sortBuffer.begin() + i);
							eraseI = true;
						}
						else if (accumulation(sortBuffer[j].first, sortBuffer[j].second) <=
							accumulation(sortBuffer[i].first, sortBuffer[i].second)) {
							sortBuffer.erase(sortBuffer.begin() + j);
							continue;
						}
					}
				}
				/*else {
					double dis = sortBuffer[i].second / cosTheta[sortBuffer[i].first];
					if (abs(dis) < deltaRho) {
						if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
							accumulation(sortBuffer[j].first, sortBuffer[j].second)) {
							sortBuffer.erase(sortBuffer.begin() + i);
							eraseI = true;
						}
						else if (accumulation(sortBuffer[j].first, sortBuffer[j].second) <=
							accumulation(sortBuffer[i].first, sortBuffer[i].second)) {
							sortBuffer.erase(sortBuffer.begin() + j);
							continue;
						}
					}
				}
				
			}*/
			j++;
		}
		if (!eraseI) {
			i++;
		}
		else {
			i--;
		}
	}

	
}

void HoughTransform::generateLines() {
	/*for (int i = 0; i < sortBuffer.size(); i++) {
		cout << "theta  " << sortBuffer[i].first << "  rho  " << sortBuffer[i].second << endl;
		double k = 0, b = 0;
		if (sinTheta[sortBuffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[sortBuffer[i].first] / sinTheta[sortBuffer[i].first];
			b = sortBuffer[i].second / sinTheta[sortBuffer[i].first];
			linesParams.push_back(make_pair(k, b));
		}
		cout << " k " << k << " b " << b;
	}*/
	int count1 = 0;
	for (int i = 0; i < sortBuffer.size(); ) {
		boolean eraseI = false;
		double k = 0, b = 0;
		if (sinTheta[sortBuffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[sortBuffer[i].first] / sinTheta[sortBuffer[i].first];
			b= sortBuffer[i].second / sinTheta[sortBuffer[i].first];
		}
		else {
			b = (double)cosTheta[sortBuffer[i].first] * sortBuffer[i].second;
		}
		if (k * 0 + b > height && k*width + b > height || k * 0 + b < 0 && k*width + b < 0 
			|| sinTheta[sortBuffer[i].first] == 0 && (b < 0 || b > height) ) {// 不经过图片
			sortBuffer.erase(sortBuffer.begin() + i);
			eraseI = true;
			count1++;
		}
		if (!eraseI) {
			i++;
		}
	}
	cout << "count1" << count1 << endl;
	int count2 = 0;
	/*for (int i = 0; i < linesParams.size(); ) {
		boolean eraseI = false;
		for (int j = i + 1; j < sortBuffer.size(); ) {
			double k1 = 0, b1 = 0, k2 = 0, b2 = 0;
			if (sinTheta[sortBuffer[i].first] != 0) {
				k1 = (-1) * (double)cosTheta[sortBuffer[i].first] / sinTheta[sortBuffer[i].first];
				b1 = sortBuffer[i].second / sinTheta[sortBuffer[i].first];
				//linesParams.push_back(make_pair(k, b));
			}
			else {
				b1 = (double)cosTheta[sortBuffer[i].first] * sortBuffer[i].second;
			}
			if (sinTheta[sortBuffer[j].first] != 0) {
				k2 = (-1) * (double)cosTheta[sortBuffer[j].first] / sinTheta[sortBuffer[j].first];
				b2 = sortBuffer[j].second / sinTheta[sortBuffer[j].first];
				//linesParams.push_back(make_pair(k, b));
			}
			else {
				b2 = (double)cosTheta[sortBuffer[j].first] * sortBuffer[j].second;
			}
			
			if ((abs(sortBuffer[i].first - sortBuffer[j].first) < deltaTheta
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 360) < deltaTheta
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 180) < deltaTheta)) {
				if (k1 - k2 < 0.000001) { // 直线平行
					double dis = abs(b1 - b2) * cosTheta[sortBuffer[i].first];
					if (dis < deltaRho) {
						count2++;
						if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
							accumulation(sortBuffer[j].first, sortBuffer[j].second)) {
							sortBuffer.erase(sortBuffer.begin() + i);
							
							eraseI = true;
						}
						else {
							sortBuffer.erase(sortBuffer.begin() + j);
							continue;
						}
					}
				}
				else {
					// 交点
					double x = (-1) * (b1 - b2) / (k1 - k2);
					int y = k1 * x + b1;
					cout << "x " << x << " y " << y << endl;
					if (x >= 0 && x < width && y >= 0 && y < height) {
						if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
							accumulation(sortBuffer[j].first, sortBuffer[j].second)) {
							sortBuffer.erase(sortBuffer.begin() + i);
							eraseI = true;
						}
						else {
							sortBuffer.erase(sortBuffer.begin() + j);
							continue;
						}
					}
				}
			}
				
			j++;
		}
		if (!eraseI) {
			i++;
		}
	}*/
	cout << "count2  " << count2 << endl;
	int count3 = 0;
	for (int i = 0; i < sortBuffer.size(); ) {
		boolean eraseI = false;
		for (int j = i + 1; j < sortBuffer.size(); ) {
			if ((abs(sortBuffer[i].first - sortBuffer[j].first) < deltaTheta/2
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 360) < deltaTheta/2
				|| abs(abs(sortBuffer[i].first - sortBuffer[j].first) - 180) < deltaTheta/2)
				&& ((sortBuffer[i].first >= 0 && sortBuffer[i].first <= 90 || sortBuffer[i].first >= 180 && sortBuffer[i].first <= 270)
					&& (sortBuffer[j].first >= 0 && sortBuffer[j].first <= 90 || sortBuffer[j].first >= 180 && sortBuffer[j].first <= 270)
					|| (sortBuffer[i].first >= 90 && sortBuffer[i].first <= 180 || sortBuffer[i].first >= 270 && sortBuffer[i].first <= 360)
					&& (sortBuffer[j].first >= 90 && sortBuffer[j].first <= 180 || sortBuffer[j].first >= 270 && sortBuffer[j].first <= 360))
				&& abs(sortBuffer[i].second - sortBuffer[j].second) < deltaRho/2) {
				if (accumulation(sortBuffer[i].first, sortBuffer[i].second) <
					accumulation(sortBuffer[j].first, sortBuffer[j].second)) {
					sortBuffer.erase(sortBuffer.begin() + i);
					eraseI = true;
				}
				else {
					sortBuffer.erase(sortBuffer.begin() + j);
					continue;
				}
			}
			j++;
		}
		if (!eraseI) {
			i++;
		}
		else {
			i--;
		}
	}
	cout << "********count3***********" << count3 << endl;
	//draw
	for (int i = 0; i < sortBuffer.size(); i++) {
		cout << "theta  " << sortBuffer[i].first << "  rho  " << sortBuffer[i].second;
		double k = 0, b = 0;
		if (sinTheta[sortBuffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[sortBuffer[i].first] / sinTheta[sortBuffer[i].first];
			b = sortBuffer[i].second / sinTheta[sortBuffer[i].first];
			linesParams.push_back(make_pair(k, b));
		}
		cout << " k " << k << " b " << b << endl;
	}
	cout << "size  " << linesParams.size()<< endl;
	
	const double blue[] = { 0, 0, 255 };
	const double red[] = { 255, 0, 0 };
	for (int i = 0; i < linesParams.size(); i++) {
		int x0 = 0, y0, x1, y1;
		double k = linesParams[i].first;
		double b = linesParams[i].second;
		if (k*x0 + b >= 0 && k*x0 + b < height) {
			y0 = k * x0 + b;
			x1 = width - 1;
			y1 = k * x1 + b;
		}
		else if(k != 0){
			y0 = 0;
			x0 = (y0 - b) / k;
			y1 = height - 1;
			x1 = (y1 - b) / k;
		}
		else {
			x0 = 0;
			x1 = width - 1;
			y0 = b;
			y1 = b;
		}
		if (k - 0.05 < 0.01) {
			result.draw_line(x0 + 3, y0 + 3, x1 + 3, y1 + 3, red);
		}
		else{
			cout << k << "   ____________    " << b << endl;
			result.draw_line(x0 + 3, y0 + 3, x1 + 3, y1 + 3, blue);
		
		}
		
	}

	string t = to_string(id) + "paperLines_origin.bmp";
	const char * temp = t.c_str();

	//result.display();
	result.save(temp);
}

void HoughTransform::drawPoints() {
	for (int i = 0; i < linesParams.size(); i++) {
		for (int j = i + 1; j < linesParams.size(); j++) {
			if (linesParams[i].first != linesParams[j].first) {
				double x = (-1) * (linesParams[i].second - linesParams[j].second) / (linesParams[i].first - linesParams[j].first);
				int y = linesParams[i].first * x + linesParams[i].second;
				if (x >= 0 && x < width && y >= 0 && y < height) {
					points.push_back(make_pair((int)x + 3, y + 3));
				}
			}
		}
		
	}
	const double pointColor[] = { 255, 0, 0 };
	cout << "draw points" << " " << points.size()<< endl;
	for (int i = 0; i < points.size(); i++) {
		result.draw_circle(points[i].first, points[i].second, 2, pointColor);
	}
	//result.display(); 
	string t = to_string(id) + "paperPoint_origin.bmp";
	const char * temp = t.c_str();
	result.save(temp);
}