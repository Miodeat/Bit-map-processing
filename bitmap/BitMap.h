#pragma once

#define pi 3.1415926

#include<map>
#include<stdlib.h>
#include<ctime>
#include<math.h>
using namespace std;

class BitMap
{
public:
	BitMap();
	BitMap(CString c_path);
	BitMap(BITMAPFILEHEADER c_header, BYTE* c_infoAndPal, BYTE* c_pixelsImage, 
		BITMAPINFOHEADER c_infoHeader, LONG weidth, LONG height, LONG lineBytesNum,
		bool hasRGBToGray);
	~BitMap();

	void setPath(CString c_path);
	CString getPath();
	BITMAPFILEHEADER getHeader();
	BITMAPINFOHEADER getInfoHeader();
	BYTE* getInfoAndPalette();
	BYTE* getImagePixels();
	LONG getImgWidth();
	LONG getImgHeitgh();

	void loadFile(CString c_path);
	void save(CString savePath = _T(""));
	void saveAs();

	void RGBToGray();
	void aveFilterSmoothing(int areaWidth);
	void medianFilterSmoothing();
	void LaplaceSharpening();
	void sobelSharpening();
	void opposite();
	void binarization(int boundary);
	void addPepperAndSalt();
	void LaplaceEdgeDetection();
	void sobelEdgeDetection();
	void robertEdgeDetection();
	void cannyEdgeDetection();
	void pixelate();
	void relievo();

	double calculateSobel(int area[], int i);
	int calculateGauseTemplate25(int area[]);
	int calculatePixelate(int area[]);
	int calculateGauseTemplate9(int area[]);
	int calculateMedian(int area[]);
	int calculateLaplace(int area[]);
	int calculateRobert(int area[]);
	double calculateGrayStandardDeviation();

	int getBitCount();
	
	map<int, int> getGrayHistogramData();
	void getRGBHistogram(map<int, int> * &destination);
	
	BitMap clone();

private:
	CString path;
	BITMAPFILEHEADER header;
	BYTE* infoAndPalette;
	BYTE* imagePixels;
	BITMAPINFOHEADER infoHeader;
	LONG imgWidth;
	LONG imgHeight;
	LONG lineBytesNum;
	int pixelSize;
	bool hasRGBToGray;
	double* gradAngleForCanny = nullptr;
	double* weightForCanny = nullptr;

	map<int, int> createZero_GrayDegreeMap();
	void qSort(int low, int high, int * target);
	int partition(int low, int high, int * target);

};

