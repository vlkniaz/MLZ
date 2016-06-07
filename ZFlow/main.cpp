// main.cpp
// демонстрация работы алгоритма прослеживания объектов

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <IMLib/IMLib.h>

using namespace std;

int main()
{
    cout << "FFT correlaction" << endl;
    
    IMImageFile *I0, *I1;
    IMMotionAnalyser motionAnalyser;
    
    IMMarkupObject object, newObject;
    IMImageMarkup markup, newMarkup;
    IMImageSeqeunceMarkup seqeunceMarkup, newSeqeunceMarkup;
    object.id = 1;
    object.objectClass = 1;
    object.region = IMRegion(14, 16, 100, 100);
    
    stringstream stream, stream2;
    stream << object;
    //cout << object << endl;
    
    IMPoint pnt;
    stream >> newObject;
    
    markup.objects.push_back(object);
    markup.objects.push_back(newObject);
    
    //cout << markup << endl;
    stream << markup;
    stream >> newMarkup;
    
    seqeunceMarkup.markup.push_back(markup);
    seqeunceMarkup.markup.push_back(newMarkup);
    
    cout << seqeunceMarkup << endl;
    stream.str("1");
    stream2 << seqeunceMarkup;
    std::string str;
    str = stream.str();
    stream2 >> newSeqeunceMarkup;
    
    ifstream file("test.json");
    file >> newSeqeunceMarkup;
    cout << newSeqeunceMarkup << endl;
    
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