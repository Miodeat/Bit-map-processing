#include "stdafx.h"
#include "BitMap.h"


BitMap::BitMap()
{
	this->path = "";
	
	this->infoAndPalette = nullptr;
	this->imagePixels = nullptr;
	this->imgHeight = 0;
	this->imgWidth = 0;
	this->lineBytesNum = 0;
	this->infoHeader = *(new BITMAPINFOHEADER());
	this->pixelSize = 0;
	this->hasRGBToGray = false;
}

BitMap::BitMap(CString c_path)
{
	this->path = "";
	
	this->infoAndPalette = nullptr;
	this->imagePixels = nullptr;
	this->imgHeight = 0;
	this->imgWidth = 0;
	this->lineBytesNum = 0;
	this->infoHeader = *(new BITMAPINFOHEADER());
	this->pixelSize = 0;
	this->hasRGBToGray = false;

	loadFile(c_path);
}


BitMap::BitMap(BITMAPFILEHEADER c_header, BYTE* c_infoAndPal,BYTE* c_pixelsImage, 
	BITMAPINFOHEADER c_infoHeader, LONG c_weidth, LONG c_height, LONG c_lineBytesNum, 
	bool c_hasRGBToGray)
{
	this->path = "";
	this->header = c_header;
	this->infoAndPalette = c_infoAndPal;
	this->imagePixels = c_pixelsImage;
	this->infoHeader = c_infoHeader;
	this->imgWidth = c_weidth;
	this->imgHeight = c_height;
	this->lineBytesNum = c_lineBytesNum;
	this->pixelSize = infoHeader.biBitCount / 8;
	this->hasRGBToGray = c_hasRGBToGray;

}


BitMap::~BitMap()
{
	
}

void BitMap::setPath(CString c_path)
{
	this->path = c_path;
}

CString BitMap::getPath()
{
	return path;
}

BITMAPFILEHEADER BitMap::getHeader()
{
	return header;
}

BITMAPINFOHEADER BitMap::getInfoHeader()
{
	return infoHeader;
}

BYTE * BitMap::getInfoAndPalette()
{
	return infoAndPalette;
}

BYTE * BitMap::getImagePixels()
{
	return imagePixels;
}

LONG BitMap::getImgWidth()
{
	return imgWidth;
}

LONG BitMap::getImgHeitgh()
{
	return imgHeight;
}

void BitMap::loadFile(CString c_path)
{
	this->path = c_path;
	CFile bmp(c_path, CFile::modeRead);

	//read BitMap file header
	bmp.Read(&header, sizeof(BITMAPFILEHEADER));

	//read information header and palette(if exits)
	infoAndPalette = new BYTE[header.bfOffBits - 14];
	bmp.Read(infoAndPalette, header.bfOffBits - 14);

	//read image
	memcpy(&infoHeader, infoAndPalette, sizeof(BITMAPINFOHEADER));
	imgWidth = infoHeader.biWidth;
	imgHeight = infoHeader.biHeight;
	LONG bitCount = infoHeader.biBitCount;
	pixelSize = infoHeader.biBitCount / 8;// get the bytes number that each pixel uses
	lineBytesNum = (imgWidth * bitCount + 31) / 32 *4;
	imagePixels = new BYTE[lineBytesNum * imgHeight];
	bmp.Read(imagePixels, lineBytesNum * imgHeight);

	bmp.Close();
}

void BitMap::save(CString savePath)
{
	if (savePath.GetLength() == 0) {
		savePath = this->path;
	}

	//save bit map
	CFile saveFile(savePath, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary);
	saveFile.Write(&header, sizeof(BITMAPFILEHEADER));//save file header
	saveFile.Write(infoAndPalette, header.bfOffBits - 14);//save infomation header and palette
	saveFile.Write(imagePixels, lineBytesNum * imgHeight);//save pixels data
	
	saveFile.Close();
}

void BitMap::saveAs()
{
	CFileDialog dlg(false, _T("bmp"), _T("untitled"), OFN_HIDEREADONLY,
		_T("Bit map file(.bmp)|*.bmp||"));
	if (dlg.DoModal() != IDOK) {
		return;
	}

	CString savePath = dlg.GetPathName();
	save(savePath);
}

void BitMap::RGBToGray()
{
	BYTE red, green, blue, gray;

	//judge if image uses RGB
	if (pixelSize != 3) {
		return;
	}

	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			blue = (BYTE)*(imagePixels + j * pixelSize + i * imgWidth * pixelSize);
			green =(BYTE) *(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 1);
			red = (BYTE)*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 2);
			gray = (BYTE)((float)(red * 0.299 + green * 0.587 + blue * 0.114));

			*(imagePixels + j * pixelSize + i * imgWidth * pixelSize) = gray;
			*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 1) = gray;
			*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 2) = gray;
		}
	}
	this->hasRGBToGray = true;
}

void BitMap::aveFilterSmoothing(int areaWidth) // smoothing of average value filter using 5*5 Gause template
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];

	double sd = calculateGrayStandardDeviation(); // get mean square error of image
	for (int i((areaWidth - 1)/2); i < imgHeight - ((areaWidth - 1) / 2); i++) {
		for (int j((areaWidth - 1) / 2); j < imgWidth - ((areaWidth - 1) / 2); j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int * area = new int[areaWidth * areaWidth];

			int count = 0;
			// get area which is close
			for (int k = i - ((areaWidth - 1) / 2); k < i + 1 + ((areaWidth - 1) / 2); k++) {
				for (int l = j - ((areaWidth - 1) / 2); l < j + 1 + ((areaWidth - 1) / 2); l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}
			switch (areaWidth) {
			case 3: {
				int afterSm = calculateGauseTemplate9(area); // get gray degree after smoothing by Gause template
				int minus = abs(int(gray - afterSm));
				int result = minus > (0.85 * sd) ? afterSm : gray; // set a threshold
				*(newImagePixels + j + i * imgWidth) = (BYTE)result;
				break;
			}
			case 5: {
				int afterSm = calculateGauseTemplate25(area); // get gray degree after smoothing by Gause template
				int minus = abs(int(gray - afterSm));
				int result = minus > (0.85 * sd) ? afterSm : gray; // set a threshold
				*(newImagePixels + j + i * imgWidth) = (BYTE)result;
				break;
			}
			default:
				return;
			}
		}
	}

	for (int i(2); i < imgHeight - 2; i++) {
		for (int j(2); j < imgWidth - 2; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::medianFilterSmoothing() // smoothing of median filter using 5*5 pixel close-area
{
	BYTE * newImagePixels = new BYTE[lineBytesNum * imgHeight];

	for (int i(2); i < imgHeight - 2; i++) {
		for (int j(2); j < imgWidth - 2; j++) {
			int area[25];

			int count = 0;
			// get area which is close( 5*5 )
			for (int k = i - 2; k < i + 3; k++) {
				for (int l = j - 2; l < j + 3; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			int median = calculateMedian(area);
			*(newImagePixels + j + i * imgWidth) = (BYTE)median;
		}
	}
	
	for (int i(2); i < imgHeight - 2; i++) {
		for (int j(2); j < imgWidth - 2; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::LaplaceSharpening()
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int area[9];
			int count = 0;
			
			for (int k = i - 1; k < i + 2; k++) {
				for (int l = j - 1; l < j + 2; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			int afterLaplace = calculateLaplace(area);
			//afterLaplace = abs(afterLaplace);
			afterLaplace = afterLaplace > 43 ? 255 : 0;
			int result = gray + afterLaplace;
			*(newImagePixels + j + i * imgWidth) = (BYTE)(result > 255 ? 255 : result);
		}
	}

	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::sobelSharpening()
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int area[9];
			int count = 0;

			for (int k = i - 1; k < i + 2; k++) {
				for (int l = j - 1; l < j + 2; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			double afterSobel = calculateSobel(area, i * imgWidth + j);
			afterSobel = afterSobel / 5;
			int result = gray + afterSobel;
			*(newImagePixels + j + i * imgWidth) = (BYTE)result;
		}
	}

	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::opposite()
{
	for (int i(0); i < imgHeight * pixelSize; i++) {
		for (int j(0); j < imgWidth * pixelSize; j++) {
			int degree = (int) * (imagePixels + j + i * imgWidth);
			int afterChange = 255 - degree;
			*(imagePixels + j + i * imgWidth) = (BYTE)afterChange;
		}
	}
}

void BitMap::binarization(int boundary)
{
	if (infoHeader.biBitCount != 8 && !hasRGBToGray) {
		return;
	}

	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			int degree = (int) * (imagePixels + j * pixelSize + i * imgWidth * pixelSize);
			degree = degree > boundary ? 255 : 0;
			if (infoHeader.biBitCount == 8) {
				*(imagePixels + j + i * imgWidth) = (BYTE)degree;
			}
			else {
				*(imagePixels + j * pixelSize + i * imgWidth * pixelSize) = (BYTE)degree;
				*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 1) = (BYTE)degree;
				*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 2) = (BYTE)degree;
			}
		}
	}
}

void BitMap::addPepperAndSalt()
{
	srand(time(0));
	for (int i(0); i < (imgWidth * imgHeight / 30); i++) {
		int col = rand() % imgWidth;
		int row = rand() % imgHeight;
		int kind = rand() % 2;
		*(imagePixels + col + row * imgWidth) = kind == 1 ? 255 : 0;
	}
}

void BitMap::LaplaceEdgeDetection()
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int area[9];
			int count = 0;

			for (int k = i - 1; k < i + 2; k++) {
				for (int l = j - 1; l < j + 2; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			int afterLaplace = calculateLaplace(area);
			int result = 0;
			//afterLaplace = abs(afterLaplace);
			if (afterLaplace > 0) {
				result = afterLaplace > 43 ? 255 : 0;
			}
			else {
				//result = afterLaplace < -37 ? 255 : 0;
			}
			*(newImagePixels + j + i * imgWidth) = (BYTE)result;

		}
	}

	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::sobelEdgeDetection()
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int area[9];
			int count = 0;

			for (int k = i - 1; k < i + 2; k++) {
				for (int l = j - 1; l < j + 2; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			double afterSobel = calculateSobel(area, j + i * imgWidth);
			//int result = afterSobel > 255 ? 255 : afterSobel;
			*(newImagePixels + j + i * imgWidth) = (BYTE)afterSobel;

		}
	}

	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::robertEdgeDetection()
{
	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(0); i < imgHeight - 1; i++) {
		for (int j(0); j < imgWidth - 1; j++) {
			int gray = (int) * (imagePixels + j + i * imgWidth);
			int area[4];
			int count = 0;

			for (int k = i; k < i + 2; k++) {
				for (int l = j; l < j + 2; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			double afterRobert = calculateRobert(area);
			int result = afterRobert > 255 ? 255 : afterRobert;
			*(newImagePixels + j + i * imgWidth) = (BYTE)result;

		}
	}

	for (int i(0); i < imgHeight - 1; i++) {
		for (int j(0); j < imgWidth - 1; j++) {
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::cannyEdgeDetection()
{
	this->gradAngleForCanny = new double[lineBytesNum * imgHeight]; // creade vector for directions of grads
	this->weightForCanny = new double[lineBytesNum * imgHeight];

	aveFilterSmoothing(5); // 5 * 5 Gauss smoothing
	sobelEdgeDetection();

	BYTE* newImagePixels = new BYTE[lineBytesNum * imgHeight];
	for (int i(1); i < imgHeight - 1; i++) {
		for (int j(1); j < imgWidth - 1; j++) {

			double gradAngle = this->gradAngleForCanny[i * imgWidth + j];
			double weight = this->weightForCanny[i * imgWidth + j];
			int grad = this->imagePixels[i * imgWidth + j];

			// inhibit grads which are not largest in near field
			if ((gradAngle > 90 && gradAngle <= 135) || (gradAngle > -90 && gradAngle <= -45)) {
				double grad1 = this->imagePixels[(i - 1) * imgWidth + j];
				double grad2 = this->imagePixels[(i + 1) * imgWidth + j];
				double grad3 = this->imagePixels[(i - 1) * imgWidth + j - 1];
				double grad4 = this->imagePixels[(i + 1) * imgWidth + j + 1];

				
				double temp1 = grad4 * weight + (1 - weight) * grad2;
				double temp2 = grad3 * weight + (1 - weight) * grad1;

				if (grad < temp1 || grad < temp2) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}
			}
			/*else if (gradAngle == -45) {
				double grad3 = this->imagePixels[(i - 1) * imgWidth + j - 1];
				double grad4 = this->imagePixels[(i + 1) * imgWidth + j + 1];

				if (!(grad > grad3 && grad > grad4)) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}
			}*/
			else if ((gradAngle > 135 && gradAngle <= 180) || (gradAngle > -45 && gradAngle <= 0)) {
				double grad1 = this->imagePixels[(i - 1) * imgWidth + j - 1];
				double grad2 = this->imagePixels[(i + 1) * imgWidth + j + 1];
				double grad3 = this->imagePixels[(i) * imgWidth + j + 1];
				double grad4 = this->imagePixels[(i) * imgWidth + j - 1];

				double temp1 = grad3 * weight + (1 - weight) * grad2;
				double temp2 = grad4 * weight + (1 - weight) * grad1;

				if (grad < temp1 || grad < temp2) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}
			}
			/*else if (gradAngle == 0) {
				double grad1 = this->imagePixels[i * imgWidth + j - 1];
				double grad2 = this->imagePixels[i * imgWidth + j + 1];

				if (!(grad > grad1 && grad > grad2)) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}
				
			}*/
			else if ((gradAngle > 0 && gradAngle <= 45) || (gradAngle > 180 && gradAngle <= -135)) {
				double grad1 = this->imagePixels[i * imgWidth + j - 1];
				double grad2 = this->imagePixels[i * imgWidth + j + 1];
				double grad3 = this->imagePixels[(i - 1) * imgWidth + j + 1];
				double grad4 = this->imagePixels[(i + 1) * imgWidth + j - 1];

				double temp1 = grad4 * weight + (1 - weight) * grad2;
				double temp2 = grad3 * weight + (1 - weight) * grad1;

				if (grad < temp1 || grad < temp2) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}
			}
			else if((gradAngle > 45 && gradAngle <= 90) || (gradAngle > -135 && gradAngle <= -90)){
				double grad1 = this->imagePixels[(i + 1) * imgWidth + j];
				double grad2 = this->imagePixels[(i - 1)* imgWidth + j];
				double grad3 = this->imagePixels[(i - 1) * imgWidth + j + 1];
				double grad4 = this->imagePixels[(i + 1) * imgWidth + j - 1];

				double temp1 = grad3 * weight + (1 - weight) * grad2;
				double temp2 = grad4 * weight + (1 - weight) * grad1;

				if (grad < temp1 || grad < temp2) {
					newImagePixels[i * imgWidth + j] = 0;
					grad = 0;
				}
				else {
					newImagePixels[i * imgWidth + j] = grad;
				}

			}

			// double threshold exam & weak edges inhibition
			if (grad < 50 && grad > 0) {
				grad = 0;
				newImagePixels[i * imgWidth + j] = 0;
			}
			else if (grad < 100) {

				double temp;
				for (int k = i - 1; k < i + 2; k++) {
					for (int l = j - 1; l < j + 2; l++) {
						temp = (double) * (imagePixels + l + k * imgWidth) >= 200 ? grad : 0;
						if (temp) {
							break;
						}
					}
				}
				grad = temp;
				newImagePixels[i * imgWidth + j] = temp;
			}
			

		}
	}

	// copy
	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			if (i == 0 || j == 0 || i == imgHeight - 1 || j == imgWidth - 1) {
				*(imagePixels + j + i * imgWidth) = 0;
			}
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}

	if (gradAngleForCanny) {
		delete[] gradAngleForCanny;
		this->gradAngleForCanny = nullptr;
	}
	
	if (weightForCanny) {
		delete[] weightForCanny;
		this->weightForCanny = nullptr;
	}

}

void BitMap::pixelate()
{
	BYTE* newImagePixels = new BYTE[imgHeight * lineBytesNum];
	for (int i(4); i < imgHeight - 4; i = i + 5) {
		for (int j(4); j < imgWidth - 4; j = j + 5) {
			int area[81];
			int count = 0;

			for (int k = i - 4; k < i + 5; k++) {
				for (int l = j - 4; l < j + 5; l++) {
					area[count] = (int) * (imagePixels + l + k * imgWidth);
					count++;
				}
			}

			int result = calculatePixelate(area);
			for (int k = i - 4; k < i + 5; k++) {
				for (int l = j - 4; l < j + 5; l++) {
					*(newImagePixels + l + k * imgWidth) = result;
				}
			}

		}
	}
	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			if (i == 0 || j == 0 || i == imgHeight - 1 || j == imgWidth - 1) {
				*(imagePixels + j + i * imgWidth) = 0;
			}
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

void BitMap::relievo()
{
	BYTE* newImagePixels = new BYTE[imgHeight * lineBytesNum];
	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			if (i == imgHeight - 1 || j == imgWidth - 1 || i == 0 || j == 0) {
				*(newImagePixels + j + i * imgWidth) = 128;
			}
			else {
				int error =  (*(imagePixels + j + 1 + (i + 1) * imgWidth)) - (*(imagePixels + j - 1 + (i - 1) * imgWidth));
				//error = error > 0 ? error : -error;
				*(newImagePixels + j + i * imgWidth) = (BYTE)error + 128;
			}
		}
	}
	

	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			if (i == 0 || j == 0 || i == imgHeight - 1 || j == imgWidth - 1) {
				*(imagePixels + j + i * imgWidth) = 0;
			}
			*(imagePixels + j + i * imgWidth) = *(newImagePixels + j + i * imgWidth);
		}
	}

	if (newImagePixels) {
		delete[] newImagePixels;
	}
}

double BitMap::calculateSobel(int area[], int pos)
{
	double sumx = 0;
	double sumy = 0;
	// edge detection on x axis
	int gx[9] = { -1, 0, 1,
				-2, 0, 2,
				-1, 0, 1 };
	int gy[9] = { 1, 2, 1,
				0, 0, 0,
				-1, -2, -1 };
	// do convolution
	for (int i(0); i < 9; i++) {
		sumx += (gx[i] * area[i]);
	}

	for (int i(0); i < 9; i++) {
		sumy += (gy[i] * area[i]);
	}

	double sum = sqrt(pow(sumx, 2)+pow(sumy, 2));
	if (gradAngleForCanny && weightForCanny) {
		if (sumx == 0) {
			sumx = 0.0000001;
		}
		
		this->gradAngleForCanny[pos] = atan(sumy / sumx) * 57.3; 
		this->weightForCanny[pos] = fabs(sumy) / fabs(sumx);
		
	}
	return sum;
}

int BitMap::calculateGauseTemplate25(int area[])
{
	int sum = 0;
	// 5*5 Gause template
	int Gause[25] = { 2, 4, 5, 4, 2,
					4, 9, 12, 9, 4, 
					5, 12, 15, 12, 5,
					4, 9, 12, 9, 4,
					2, 4, 5, 4, 2};
	// do convolution
	for (int i(0); i < 25; i++) {
		sum += (area[i] * Gause[i]);
	}
	sum /= 159;
	return sum;
}

int BitMap::calculatePixelate(int area[])
{
	int model[81] = { 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 2, 2, 2, 2, 2, 2, 2, 1,
					1, 2, 3, 3, 3, 3, 3, 2, 1,
					1, 2, 3, 4, 4, 4, 3, 2, 1,
					1, 2, 3, 4, 5, 4, 3, 2, 1,
					1, 2, 3, 4, 4, 4, 3, 2, 1,
					1, 2, 3, 3, 3, 3, 3, 2, 1,
					1, 2, 2, 2, 2, 2, 2, 2, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1};

	int sum = 0;
	for (int i(0); i < 81; i++) {
		sum += area[i] * model[i];
	}
	sum /= 149;
	return sum;
	
}

int BitMap::calculateGauseTemplate9(int area[])
{
	int sum = 0;
	// 3*3 Gause template
	int Gause[9] = { 1, 2, 1,
					2, 4, 2,
					1, 2, 1};
	// do convolution
	for (int i(0); i < 9; i++) {
		sum += (area[i] * Gause[i]);
	}
	sum /= 16;
	return sum;
}

int BitMap::calculateMedian(int area[])
{
	qSort(0, 24, area);
	return area[12];
}

int BitMap::calculateLaplace(int area[])
{
	int sum = 0;
	// 3 * 3 Laplace template
	int laplace[9] = { -1, -1, -1,
					   -1, 8, -1,
					   -1, -1, -1 };

	// do convolution
	for (int i(0); i < 9; i++) {
		sum += area[i] * laplace[i];
	}
	return sum;
}

int BitMap::calculateRobert(int area[])
{
	int sumx = 0, sumy = 0;
	int gx[4] = { -1, 0,
				0, 1 };
	int gy[4] = { 0, -1,
				1, 0 };
	for (int i(0); i < 4; i++) {
		sumx += area[i] * gx[i];
	}

	for (int i(0); i < 4; i++) {
		sumy += area[i] * gy[i];
	}

	double sum = sqrt(((long)sumx * sumx) + ((long)sumy * sumy));
	return round(sum);
}

double BitMap::calculateGrayStandardDeviation()
{
	map<int, int> grayDegree = getGrayHistogramData();
	map<int, int>::iterator it = grayDegree.begin();
	int total = 0;
	while (it != grayDegree.end()) {
		total += it->first * it->second;
		it++;
	}
	int ave = round(total / (imgHeight * imgWidth));
	
	it = grayDegree.begin();
	int totalSD = 0;
	while (it != grayDegree.end()) {
		totalSD += (it->first - ave) * (it->first - ave) * it->second;
		it++;
	}

	double sd = sqrt((double)totalSD / (imgHeight * imgWidth));
	return sd;
}

int BitMap::getBitCount()
{
	return infoHeader.biBitCount;
}

map<int,int> BitMap::getGrayHistogramData()
{

	// judge if image is gray image
	if (infoHeader.biBitCount != 8 && !hasRGBToGray) {
		MessageBox(NULL, _T("It's not a gray image!"), _T("Unspported"), 0);
		return *(new map<int, int>);
	}

	map<int, int> grayDegree = createZero_GrayDegreeMap();

	if (infoHeader.biBitCount == 8) {
		for (int i(0); i < imgHeight; i++) {
			for (int j(0); j < imgWidth; j++) {
				int gray = (int)*(imagePixels + j + i * imgWidth);
				int grayDegreePixNum = grayDegree[gray];
				grayDegreePixNum++;
				grayDegree[gray] = grayDegreePixNum;
			}
		}
	}
	else {
		for (int i(0); i < imgHeight; i++) {
			for (int j(0); j < imgWidth; j++) {
				int gray = (int)*(imagePixels + j * pixelSize + i * imgWidth * pixelSize);
				int grayDegreePixNum = grayDegree[gray];
				grayDegreePixNum++;
				grayDegree[gray] = grayDegreePixNum;
			}
		}
	}

	return grayDegree;
}

BitMap BitMap::clone()
{
	BITMAPFILEHEADER newHeader = header;
	BYTE* newInfoAndPalette = new BYTE[header.bfOffBits - 14];
	memcpy(newInfoAndPalette, this->infoAndPalette, header.bfOffBits - 14);
	BYTE* newImgPixels = new BYTE[lineBytesNum * imgHeight];
	memcpy(newImgPixels, this->imagePixels, (lineBytesNum * imgHeight));

	return BitMap(newHeader, newInfoAndPalette, newImgPixels, this->infoHeader,
		this->imgWidth, this->imgHeight, this->lineBytesNum, this->hasRGBToGray);
}

map<int, int> BitMap::createZero_GrayDegreeMap()
{
	map<int, int> grayDegree;
	for (int i(0); i <= 255; i++) {
		grayDegree[i] = 0;
	}
	return grayDegree;
}

void BitMap::qSort(int low, int high, int* target)
{
	if (low < high) {
		int pivotPos = partition(low, high, target);
		qSort(low, pivotPos - 1, target);
		qSort(pivotPos + 1, high, target);
	}
}

int BitMap::partition(int low, int high, int * target)
{
	int pivot = target[low];
	while (low < high) {
		while (low < high && target[high] >= pivot) {
			high--;
		}
		target[low] = target[high];


		while (low < high && target[low] <= pivot) {
			low++;
		}
		target[high] = target[low];
	}
	target[low] = pivot;
	return low;
}

void BitMap::getRGBHistogram(map<int, int> * &destination)
{
	//judge if image is RGB image
	if (pixelSize != 3) {
		MessageBox(NULL, _T("It's not a RGB image!"), _T("Unspported"), 0);
		destination = NULL;
	}

	map<int, int> redHisData;
	map<int, int> greenHisData;
	map<int, int> blueHisData;
	for (int i(0); i < imgHeight; i++) {
		for (int j(0); j < imgWidth; j++) {
			int blue = (int)*(imagePixels + j * pixelSize + i * imgWidth * pixelSize);
			int green = (int) *(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 1);
			int red = (int)*(imagePixels + j * pixelSize + i * imgWidth * pixelSize + 2);
			
			redHisData[red]++;
			blueHisData[blue]++;
			greenHisData[green]++;
			
		}
	}
	destination = new map<int, int>[3];
	destination[0] = redHisData;
	destination[1] = greenHisData;
	destination[2] = blueHisData;
	
}

