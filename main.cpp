#include "canny.h"
//#include "HoughTransform.h"
#include "CImg.h"
#include <String>
#include "CircleDetector.h"
using namespace cimg_library;
using namespace std;
int main()
{
	double percentage[7] = {0, 0.9, 0.8, 0.8, 0.8, 0,8, };
	int thres[7] = {0, 100, 200, 200,300, 300, }

	for (int i = 6; i < 7; i++) {
		string name = to_string(i) + ".bmp";
		const char* input = name.c_str();
		CImg<unsigned char> originalImg(input);
		string edge_origin = to_string(i) + "edge_test1.bmp";
		input = edge_origin.c_str();
		CImg<unsigned char> edgeImg(input); 
		canny answer = canny(originalImg, 0.7, 60);
		CircleDetector houghTransform = CircleDetector(edgeImg, originalImg, i);
	}
	/*for (int i = 1; i <= 6; i++) {
		string name = to_string(i) + ".bmp";
		const char* input = name.c_str();
		CImg<unsigned char> originalImg(input);
		canny answer = canny(originalImg);
		CircleDetector circle = CircleDetector(answer.getResult(), originalImg, i);
	}*/
	//HoughTransform houghTransform = HoughTransform(answer.getResult(), originalImg, i);
	
	system("pause");
}
