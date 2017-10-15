// main.cpp
// example of IMImageConverter class

#include <iostream>
#include <IMLib/IMLib.h>
#include <IMLib/IMImageConverter.h>
#include <IMLib/IMTIFFImageFileFormat.h>

using namespace std;

int main()
{
    cout << "Hello, world!" << endl;
    
    IMImageFile image, *thresholdImage, *randomImage;
    IMImageConverter converter;
    IMTIFFImageFileFormat tiff;
    
    image.open("gradient-h.tif");
    image.allocAndLoadImage();
    
    converter.setImage(&image);
    thresholdImage = converter.newImageWithDithering(IMImageConverter::DITHERING_THRESHOLD);
    thresholdImage->setFileFormat(&tiff);
    thresholdImage->save("gradient-h_threshold.tif");
    
    randomImage = converter.newImageWithDithering(IMImageConverter::DITHERING_RANDOM);
    randomImage->setFileFormat(&tiff);
    randomImage->save("gradient-h_random.tif");
    
    return 0;
}
