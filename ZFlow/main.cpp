    // main.cpp
// демонстрация работы алгоритма прослеживания объектов

#include <iostream>
#include <vector>
#include <IMLib/IMLib.h>

using namespace std;

int main()
{
    cout << "FFT correlaction" << endl;
    
    IMImageFile *I0, *I1;
    IMMotionAnalyser motionAnalyser;
    
    
    I1 = new IMImageFile;
    char imageName[256];
    int curImageN = 0;
    
    vector<IMImageFile*> imageSequence;
    
    for(curImageN = 0; curImageN < 100; curImageN++)
    {
        sprintf(imageName, "./videoBW/ir%04d.tif", curImageN);
        I0 = new IMImageFile;
        I0->open(imageName);
        imageSequence.push_back(I0);
    }
    
    motionAnalyser.setImageSequence(imageSequence);
    motionAnalyser.setOutputImageSize(IMSize(64, 64));
    motionAnalyser.setOutputImageNameFormat("image%04d.tiff");
    motionAnalyser.setOutputImagePath("./out");

    IMPoint shift, point;
    IMSize searchWindowSize;
    searchWindowSize.width = 64;
    searchWindowSize.height = 64;
    point.x = 63;
    point.y = 220;
    int j = 0;
    for(j = 0; j < 1; j++)
    {
        motionAnalyser.trackObjectAtFrame(37, point, searchWindowSize, 0.01);
    }
    
    j = 8;


    return 0;
}