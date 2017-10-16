// main.cpp
// example of IMImageConverter class

#include <iostream>
#include <string>
#include <IMLib/IMLib.h>
#include <IMLib/IMImageConverter.h>
#include <IMLib/IMTIFFImageFileFormat.h>

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        cout << "Not enough arguments. Please enter image name." << endl;
    }
    string file = argv[1];
    size_t lastindex = file.find_last_of(".");
    string filename = file.substr(0, lastindex);
    
    
    IMImageFile image, *thresholdImage, *randomImage, *orderedImage, *diffusionForwardImage, *diffusionForwardBackwardImage, *FloydImage;
    IMImageConverter converter;
    IMTIFFImageFileFormat tiff;
    
    image.open(file);
    image.allocAndLoadImage();
    
    converter.setImage(&image);
    thresholdImage = converter.newImageWithDithering(IMImageConverter::DITHERING_THRESHOLD);
    thresholdImage->setFileFormat(&tiff);
    thresholdImage->save(filename + "_threshold.tif");
    
    randomImage = converter.newImageWithDithering(IMImageConverter::DITHERING_RANDOM);
    randomImage->setFileFormat(&tiff);
    randomImage->save(filename + "_random.tif");
    
    orderedImage = converter.newImageWithDithering(IMImageConverter::DITHERING_ORDERD);
    orderedImage->setFileFormat(&tiff);
    orderedImage->save(filename + "_ordered.tif");
    
    diffusionForwardImage = converter.newImageWithDithering(IMImageConverter::DITHERING_DIFFUSION_FORWARD);
    diffusionForwardImage->setFileFormat(&tiff);
    diffusionForwardImage->save(filename + "_diffusionForward.tif");
    
    diffusionForwardBackwardImage = converter.newImageWithDithering(IMImageConverter::DITHERING_DIFFUSION_FORWARD_BACKWARD);
    diffusionForwardBackwardImage->setFileFormat(&tiff);
    diffusionForwardBackwardImage->save(filename + "_diffusionForwardBackward.tif");
    
    FloydImage = converter.newImageWithDithering(IMImageConverter::DITHERING_FLOYD);
    FloydImage->setFileFormat(&tiff);
    FloydImage->save(filename + "_diffusionFloyd.tif");
    
    return 0;
}
