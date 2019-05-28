# Bit-map-processing
基于mfc的数字图像处理程序，天朝某大学课程上机程序
# What is this？
天朝某大学数字图像处理课程的部分上机实验程序。希望能帮助有该门课程的学弟学妹们更好地完成课程任务。

# Tips
1. 仅上传BitMap类，关于BitMap类的注释见下。
2. 该程序使用c++语言，VS2019中的mfc完成开发（虽然好像没什么用但是还是提一下）。

# class members
## CString path
   存放位图在硬盘中的路径
## BITMAPFILEHEADER header
   该位图的文件头
## BYTE* infoAndPalette
   存放该位图的信息头和调色板的二进制数组名
## BYTE* imagePixels
   存放该位图像素信息的二进制数组名
## BITMAPINFOHEADER infoHeader
   该位图的信息头
## LONG imgWidth, imgHeight,lineBytesNum
   该位图的图像宽，高和每行像素数量
## int pixelSize
   该位图一个像素所占字节数
## bool hasRGBToGray
   指示该图像是否从彩色转为灰度图像
## double* gradAngleForCanny
   为canny边缘检测存储梯度方向（角度值）
## double* weightForCanny
   为canny边缘检测存储y方向梯度与x方向梯度的比值

# Function introduction
## clone
   该类所有非静态成员函数都会覆盖调用函数的imagePixel，建议调用前使用clone函数获得一个新的对象
## set[a-zA-Z]\*
   设置成员值
## get[a-zA-Z]\*
   获得成员值
## loadFile
   读取位图
## save, saveAs
   保存和另存为
## RGBToGray
   彩色转灰度
## [a-zA-Z]\*Smoothing
   各种平滑滤波
## [a-zA-Z]\*Sharpening
   各种图像锐化
## [a-zA-Z]\*EdgeDetection
   各种边缘检测
## calculate[a-zA-Z]\*
   各种功能需要的辅助计算函数，如卷积
## pixelate
   马赛克
## relievo
   浮雕特效
## opposite
   反色
## binarization
   二值化
## addPepperAndSalt()
   添加椒盐噪声
## getGrayHistogram，getRGBHistogram
   获得绘制灰度直方图需要的灰度分布（灰度图和彩色图）
## qsort()
   快速排序
   
