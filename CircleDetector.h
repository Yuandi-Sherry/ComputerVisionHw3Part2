#include <iostream>
#include "canny.h"
#include <time.h>     
using namespace std;
using namespace cimg_library;
#define M_PI 3.14159265358979323846
int thresDev[7] = {0, 4,  5.5, 10, 10,30,1.5 };
int stepDev[7] = {0, 100, 25, 25, 25 , 100,100};
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
	int step;
	double dia;
	CImg<unsigned char> img;
	CImg<unsigned char> result;
	CImg<unsigned char> edge;
	vector<point> potentialCircle;
	vector<point> selectedCircle;
	clock_t  start, end;
public:
	CircleDetector(CImg<unsigned char> img, CImg<unsigned char> originalImg, int id) {
		
		cimg_forXY(edge, i, j) {
			if (img(i, j) == 255) {
				edge(i, j, 0, 0) = 255;
				edge(i, j, 0, 1) = 255;
				edge(i, j, 0, 2) = 255;
			}
			else {
				edge(i, j, 0, 0) = 0;
				edge(i, j, 0, 1) = 0;
				edge(i, j, 0, 2) = 0;
			}
		}
		this->result = originalImg;
		this->img = img;
		this->id = id;
		this->width = img.width();
		this->height = img.height();
		edge = CImg<unsigned char>(width, height, 1, 3);
		maxR = min(width, height) / 2;
		dia = sqrt(pow(height, 2) + pow(width, 2)) / 2;
		step = maxR / 100;
		accumulation = CImg<unsigned char>(width, height, ceil(maxR / step) + 1, 1);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				for (int k = 0; k < maxR; k = k + step) {
					accumulation(i, j, k / step, 0) = 0;
				}
			}
		}
		start = clock();
		vote();
		filterThershold(maxR /1.3);
		filter();
		drawCircles();
		end = clock();
	}

	void vote() {
		string t = to_string(id) + "_Iedge.bmp";
		const char * temp = t.c_str();
		img.save(temp);
		// ����Բ���ϵ�
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				if (img(i, j) == 255) {
					for (int r = 0; r < maxR; r = r+ step) {
						// �������ܳ���Բ�ĵ������
						for (int theta = 0; theta < 360; theta++) {
							int x = i + r * cos(theta*M_PI / 180);
							int y = j + r * sin(theta*M_PI / 180);
							if (x >= 0 && x < width && y >= 0 && y < height)
								accumulation(x, y, r/ step, 0)++;
						}
					}
				}
			}
		}
		//accumulation.display();
	}

	void filterThershold(int threshold) {
		cout << "filterThershold" << endl;
		int maxRadius = 0;
		bool jumpOut = false;
		for (int r = floor(maxR/step)*5; r > 0;  r = r + step) {
			for (int i = r; i < width - r; i++) {
				for (int j = r; j < height - r; j++) {
					if(accumulation(i,j,r/ step,0) > threshold) {
						maxRadius = r;
						jumpOut = true;
						break;
					}
				}
				if (jumpOut)
					break;
			}
			if (jumpOut)
				break;
 		}

		for (int r = 0; r < maxR; r = r + step) {
			for (int i = r; i < width - r; i++) {
				for (int j = r; j < height - r; j++) {
					if (accumulation(i, j, r / step, 0) > threshold) {
						boolean push = true;
						for (int k = 0; k < potentialCircle.size(); k++) {
							if (r < maxRadius * 0.5) { // ������Ҫ�󣬲��ñ�������
								push = false;
								break;
							}
							if (r*0.5 > potentialCircle[k].r) { // ��Բ�δ�С������СԲֱ�ӱ��滻
								push = false;
								potentialCircle[k] = point(i, j, r);
							} else if(sqrt(pow(i - potentialCircle[k].x, 2) + pow(j - potentialCircle[k].y, 2)) < 0.9 * (potentialCircle[k].r + r)) { // ��Բ�������ཻ
								push = false;
								if (accumulation(i, j, r/ step, 0) > accumulation(potentialCircle[k].x, potentialCircle[k].y, potentialCircle[k].r/ step, 0)) { // ����Ʊ�����
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
		}

	}

	void filter() {
		int count = 0;
		for (int i = 0; i < potentialCircle.size(); ) {
			bool eraseI = false;
			for (int j = i + 1; j < potentialCircle.size(); ) {
				//�ж���Բ�ཻ
				if (sqrt(pow(potentialCircle[i].x - potentialCircle[j].x, 2) + pow(potentialCircle[i].y - potentialCircle[j].y, 2)) < 0.9 * (double)(potentialCircle[i].r + potentialCircle[j].r)) { // ��Բ�������ཻ
					count++;
					// ��Բi��Ʊ����Բj��
					if (accumulation(potentialCircle[i].x, potentialCircle[i].y, potentialCircle[i].r / step, 0) > accumulation(potentialCircle[j].x, potentialCircle[j].y, potentialCircle[j].r / step, 0)) { // ����Ʊ�����
						potentialCircle.erase(potentialCircle.begin() + j);
						continue;
					}
					else {
						potentialCircle.erase(potentialCircle.begin() + i);
						eraseI = true;
						break;
					}
				}
				j++;
			}
			if (!eraseI) {
				i++;
			}
		}
	}

	void drawCircles() {
		const double red[] = { 255, 0, 0 };
		const double green[] = { 0, 255, 0 };
		cout << "drawCircles" << potentialCircle.size() << endl;
		for (int i = 0; i < potentialCircle.size(); i++) {
			for (int theta = 0; theta < 360; theta++) {
				int x = potentialCircle[i].x + potentialCircle[i].r * cos(theta*M_PI / 180);
				int y = potentialCircle[i].y + potentialCircle[i].r * sin(theta*M_PI / 180);
				if (x >= -3 && x < width && y >= -3 && y < height) {
					result.draw_circle(x + 3, y + 3, 2, green);
					edge.draw_circle(x + 3, y + 3, 2, green);
				}
				
			}
		}
		string name_origin = to_string(id) + "_origin_I2.bmp";
		const char* input_origin = name_origin.c_str();
		result.save(input_origin);
		string name_edge = to_string(id) + "_edge_I2.bmp";
		const char* input_edge = name_edge.c_str();
		edge.save(input_edge);


		cout << "finish drawCircles" << endl;
		cout << "end" << end << endl;
		cout << "start" << start << endl;
		cout << "CLK_TCK" << CLK_TCK << endl;
		cout << "Hough Transform��ʱ��" << (double)(end - start) / CLK_TCK << endl;

		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				if (edge(i, j, 0, 2) == 0
					&& edge(i, j, 0, 1) == 255
					&& edge(i, j, 0, 0) == 0) {
					if (img(i, j) == 255) {
						edge(i, j, 0, 0) = 255;
						edge(i, j, 0, 1) = 0;
						edge(i, j, 0, 2) = 0;
					}
				}
			}
		}

		string t1 = to_string(id) + "-I3.bmp";
		const char * temp1 = t1.c_str();
		edge.save(temp1);
		
	}

};