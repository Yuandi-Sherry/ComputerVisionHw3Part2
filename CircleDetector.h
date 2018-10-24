#include <iostream>
#include "canny.h"
using namespace std;
using namespace cimg_library;
#define M_PI 3.14159265358979323846
struct point {
	int x;
	int y;
	int r;
	point(int _x, int _y, int _r) {
		x = _x;
		y = _y;
		r = _r;
	}
};
class CircleDetector {
private:
	CImg<unsigned char> accumulation;
	int id;
	int width;
	int height;
	int maxR;
	CImg<unsigned char> img;
	CImg<unsigned char> result;
	vector<point> potentialCircle;
	vector<point> selectedCircle;

	

public:
	CircleDetector(CImg<unsigned char> img, CImg<unsigned char> originalImg, int id) {
		this->result = originalImg;
		this->img = img;
		this->id = id;
		this->width = img.width();
		this->height = img.height();
		maxR = min(width , height)/2;
		cout << "maxR " << maxR << endl; 
		accumulation = CImg<unsigned char>(width, height, ceil(maxR/5) + 1,1);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				for (int k = 0; k < maxR; k = k+5) {
					//cout << k << endl;
					accumulation(i,j,k/5,0) = 0;
				}
			}
		}
		vote();
		filterThershold(maxR/4);
		//findLocalMaximums();
		drawCircles();
	}

	void vote() {
		string t = to_string(id) + "edge_test1.bmp";
		const char * temp = t.c_str();
		img.save(temp);
		//result.display();
		//img.display();
		cout << "vote" << endl;
		// 遍历圆周上点
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				if (img(i, j) == 255) {
					for (int r = 0; r < maxR; r = r+5) {
						// 遍历可能出现圆心点的区域
						for (int theta = 0; theta < 360; theta++) {
							int x = i + r * cos(theta*M_PI / 180);
							int y = j + r * sin(theta*M_PI / 180);
							if (x >= 0 && x < width && y >= 0 && y < height)
								accumulation(x, y, r/5, 0)++;
						}
					}
				}
			}
		}
		//accumulation.display();
	}

	void filterThershold(int threshold) {
		cout << "filterThershold" << endl;
		for (int r = 0; r < maxR;  r = r + 5) {
			for (int i = r; i < width - r; i++) {
				for (int j = r; j < height - r; j++) {
					if(accumulation(i,j,r/5,0) > threshold) {
						boolean push = true;
						for (int k = 0; k < potentialCircle.size(); k++) {
							if (r*0.8 > potentialCircle[k].r) { // 两圆形大小差距过大，小圆直接被替换
								push = false;
								potentialCircle[k] = point(i, j, r);
							} else if(sqrt(pow(i - potentialCircle[k].x, 2) + pow(j - potentialCircle[k].y, 2)) < 0.9*(potentialCircle[k].r + r)) { // 两圆形明显相交
								push = false;
								if (accumulation(i, j, r/5, 0) > accumulation(potentialCircle[k].x, potentialCircle[k].y, potentialCircle[k].r/5, 0)) { // 保留票数多的
									potentialCircle[k] = point(i, j, r);
								}
							}
						}
						if (push) {
							potentialCircle.push_back(point(i, j, r));
						}
					}
				}
			}
			cout << r << endl;
 		}
		cout << potentialCircle.size() << endl;
	}

	void findLocalMaximums() {
		cout << "findLocalMaximums" << endl;

		for (int i = 0; i < potentialCircle.size(); ) {
			bool iChange = false;
			for (int j = i + 1; j < potentialCircle.size(); ) {
				// 判断半径
				
					if (potentialCircle[i].r < 0.5 * potentialCircle[j].r) {
						potentialCircle.erase(potentialCircle.begin() + i);
						iChange = true;
						break;
					}
					else if (potentialCircle[j].r < 0.5 * potentialCircle[i].r) {
						potentialCircle.erase(potentialCircle.begin() + j);
						continue;
					}
				
				j++;
			}
			if (!iChange) {
				i++;
			}

		}

		for (int i = 0; i < potentialCircle.size(); ) {
			bool iChange = false;
			for (int j = i + 1; j < potentialCircle.size(); ) {
				// 判断半径
				if (pow(potentialCircle[i].x - potentialCircle[j].x, 2) + pow(potentialCircle[i].y - potentialCircle[j].y, 2) < pow(potentialCircle[i].r + potentialCircle[j].r, 2)*0.9) {
					if (potentialCircle[i].r < 0.5 * potentialCircle[j].r) {
						potentialCircle.erase(potentialCircle.begin() + i);
						iChange = true;
						break;
					}
					else if (potentialCircle[j].r < 0.5 * potentialCircle[i].r) {
						potentialCircle.erase(potentialCircle.begin() + j);
						continue;
					}
				}
				j++;
			}
			if (!iChange) {
				i++;
			}
			
		}

		for (int i = 0; i < potentialCircle.size(); ) {
			bool iChange = false;
			for (int j = i + 1; j < potentialCircle.size(); ) {
				if (pow(potentialCircle[i].x - potentialCircle[j].x, 2) + pow(potentialCircle[i].y - potentialCircle[j].y, 2) < pow(potentialCircle[i].r + potentialCircle[j].r, 2) ) {
					if (accumulation(potentialCircle[i].x, potentialCircle[i].y, potentialCircle[i].r) < accumulation(potentialCircle[j].x, potentialCircle[j].y, potentialCircle[j].r)) {
						potentialCircle.erase(potentialCircle.begin() + i);
						iChange = true;
						break;
					}
					else {
						potentialCircle.erase(potentialCircle.begin() + j);
						continue;
					}
				}
				j++;
			}
			if (!iChange) {
				i++;
			}

		}
		cout << potentialCircle.size() << endl;
	}

	void drawCircles() {
		cout << "drawCircles" << endl;
		for (int i = 0; i < potentialCircle.size(); i++) {
			for (int theta = 0; theta < 360; theta++) {
				int x = potentialCircle[i].x + potentialCircle[i].r * cos(theta*M_PI / 180);
				int y = potentialCircle[i].y + potentialCircle[i].r * sin(theta*M_PI / 180); 
				if (x >= -3 && x < width && y >= -3 && y < height) {
					result(x + 3, y +3, 0, 0) = 255;
					result(x+3, y+3, 0, 1) = 0;
					result(x+3, y+3, 0, 2) = 0;
				}
				
			}
		}
		
		string name = to_string(id) + "final_test1.bmp";
		const char* input = name.c_str();
		result.display();
		result.save(input);
		cout << "finish drawCircles" << endl;
	}

};