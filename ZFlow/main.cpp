// main.cpp
// демонстрация работы алгоритма прослеживания объектов

#include <iostream>
#include <IMLib/IMLib.h>

using namespace std;

int main()
{
    cout << "FFT correlaction" << endl;
    
    IMImageFile *I0, *I1;
    IMMotionAnalyser motionAnalyser;
    
    I0 = new IMImageFile;
    I1 = new IMImageFile;
    
    I0->open("./video2/video0000.tif");
    I1->open("./video2/video0001.tif");
    I0->allocAndLoadImage();
    I1->allocAndLoadImage();

    MAVector3 shift;
    int j = 0;
    for(j = 0; j < 1000; j++)
    {
        shift = motionAnalyser.shiftFromI0ToI1(I0, I1);
    }
    
    j = 8;


    return 0;
}