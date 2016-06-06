//
//  IMMotionAnalyser.cpp
//  Билиотека IM
//
//  Анализ видеопоследовательностей
//
//  Created by Vladimir Knyaz on 06.04.15.
//  Copyright (c) 2015 Vladimir Knyaz. All rights reserved.
//

#include <IMLib/IMLib.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <chrono>
#include <math.h>
#include <string>
#include <random>

#import <fftw3.h>					// для быстрого преобразования фурье

using namespace std;

struct IMOpticalFlow
{
    double u;
    double v;
};

IMPoint maxLocation(fftw_complex *image, int width, int height, IMPoint& secondMax, double& max1val, double& max2val);

void iFFTToD(unsigned int width, unsigned int alignedWidth, unsigned int height, unsigned char *iData, double *dData, int logarithm)
{
    if(iData == 0 || dData == 0)
    {
        return;
    }
    unsigned int i, j, y, size, shift;
    i = 0;
    j = 0;
    y = 0;
    size = alignedWidth * height;
    shift = alignedWidth - width;
    
    if(!logarithm)
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            *dData = iData[j] * pow(-1.0, i + y);
            dData++;
            i++;
        }
    }
    else
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            *dData = iData[j];
            *dData = log(*dData + 1.0);
            *dData *= pow(-1.0, i + y);
            dData++;
            i++;
        }
    }
}

IMImageFile* window2d(int width, int height)
{
    int x, y, index;
    double r, w2, h2, dx, dy, alpha, beta, w;
    double *data;
    
    IMImageFile *window = new IMImageFile;
    window->setPixelsWide(width);
    window->setPixelsHigh(height);
    window->setBitsPerSample(64);
    window->setSamplesPerPixel(1);
    window->setBytesPerRow(width*sizeof(double));
    window->allocImage();
    
    MAMatrix row(1, width), column(height, 1);
    
    // окно 'periodic'
    w2 = (width+1) / 2;
    h2 = (height+1) / 2;
    
    alpha = 0.54;
    beta = 1.0 - alpha;
    
    data = reinterpret_cast<double*>(window->image());
    
    for(x = 0; x < width; x++)
    {
        w = alpha - beta*cos((M_PI*2*x)/(width+1));
        row.set(0, x, w);
        column.set(x, 0, w);
    }
    
    MAMatrix mwindow =  column * row;
    //cout << mwindow.toMatlab() << endl;
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            index = x + y * height;
            
            data[index] = mwindow.get(x, y);
        }
    };
    
    //window->setFileFormat(new IMTIFFImageFileFormat);
    //window->save((BASE_PATH + "window.tif").c_str());
    
    return window;
}

void iFFTToFFT(unsigned int width, unsigned int alignedWidth, unsigned int height, unsigned char *iData, fftw_complex *dData, int logarithm, bool center = true, IMImageFile* window = 0)
{
    if(iData == 0 || dData == 0)
    {
        return;
    }
    unsigned int i, j, y, size, shift;
    i = 0;
    j = 0;
    y = 0;
    size = alignedWidth * height;
    shift = alignedWidth - width;
    
    double w,*windowData = 0;
    
    if(window != 0)
    {
        windowData = reinterpret_cast<double*>(window->image());
    }
    
    if(!logarithm)
    {
        if(center)
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                dData[j][0] = iData[j] * pow(-1.0, i + y);
                dData[j][1] = 0.0;
                //dData++;
                i++;
            }
        }
        else
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                
                if(window != 0)
                {
                    w = windowData[i + y*width];
                    dData[j][0] = iData[j] * w;
                }
                else
                {
                    dData[j][0] = iData[j];
                }
                dData[j][1] = 0.0;
                i++;
            }
        }
    }
    else
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            *dData[0] = iData[j];
            *dData[0] = log(*dData[0] + 1.0);
            *dData[0] *= pow(-1.0, i + y);
            *dData[1] = 0.0;
            dData++;
            i++;
        }
    }
}

void iFFTToFFTWithMask(unsigned int width, unsigned int alignedWidth, unsigned int height, unsigned char *iData, fftw_complex *dData, int logarithm, bool center = true, unsigned char* mask = 0, double code = 1)
{
    if(iData == 0 || dData == 0)
    {
        return;
    }
    unsigned int i, j, y, size, shift;
    i = 0;
    j = 0;
    y = 0;
    size = alignedWidth * height;
    shift = alignedWidth - width;
    
    IMImageFile *test;
    test = new IMImageFile;
    test->setPixelsWide(width);
    test->setPixelsHigh(height);
    test->setBitsPerSample(32);
    test->setSamplesPerPixel(1);
    test->setBytesPerRow(sizeof(float) * width);
    test->allocImage();
    
    float *testPix = reinterpret_cast<float*>(test->image());
    
    if(!logarithm)
    {
        if(center)
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                dData[j][0] = iData[j] * pow(-1.0, i + y);
                dData[j][1] = 0.0;
                //dData++;
                i++;
            }
        }
        else
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                if(mask[j] == 0)
                {
                    dData[j][0] = iData[j];
                }
                else
                {
                    dData[j][0] = code*iData[j];
                }
                testPix[j] = dData[j][0];
                dData[j][1] = 0.0;
                i++;
            }
        }
    }
    else
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            *dData[0] = iData[j];
            *dData[0] = log(*dData[0] + 1.0);
            *dData[0] *= pow(-1.0, i + y);
            *dData[1] = 0.0;
            dData++;
            i++;
        }
    }
    
    test->setFileFormat(new IMTIFFImageFileFormat);
    //test->save((BASE_PATH + "hopsel.tif").c_str());
}

void dFFTToF(unsigned int width, unsigned int alignedWidth, unsigned int height, float *fData, double *dData, int logarithm, int imageBPS)
{
    if(fData == 0 || dData == 0)
    {
        return;
    }
    double sizeD, maxValue, scale;
    unsigned int i, j, y, size, shift;
    i = 0;
    j = 0;
    y = 0;
    size = alignedWidth * height;
    maxValue = (pow(2, imageBPS) -1.0);
    if(imageBPS == 32)
    {
        maxValue = 1.0;
    }
    sizeD = alignedWidth * height;
    scale = sizeD * maxValue;
    shift = alignedWidth - width;
    
    if(!logarithm)
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            fData[j] = *dData * pow(-1.0, i + y) / scale;
            dData++;
            i++;
        }
    }
    else
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            fData[j] = *dData;
            fData[j] = fData[j] * pow(-1.0, i + y) / sizeD;
            fData[j] = exp(fData[j]) - 1.0;
            fData[j] /= maxValue;
            dData++;
            i++;
        }
    }
}

void FFTToF(unsigned int width, unsigned int alignedWidth, unsigned int height, float *fData, fftw_complex *fftData, int logarithm, int imageBPS, bool center = true)
{
    if(fData == 0 || fftData == 0)
    {
        return;
    }
    double sizeD, maxValue, scale;
    unsigned int i, j, y, size, shift;
    i = 0;
    j = 0;
    y = 0;
    size = alignedWidth * height;
    maxValue = (pow(2, imageBPS) -1.0);
    if(imageBPS == 32)
    {
        maxValue = 1.0;
    }
    sizeD = alignedWidth * height;
    scale = sizeD * maxValue;
    shift = alignedWidth - width;
    
    if(!logarithm)
    {
        if(center)
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                fData[j] = *fftData[0];// * pow(-1.0, i + y)/* / scale*/;
                fftData++;
                i++;
            }
        }
        else
        {
            for(j = 0; j < size; j++)
            {
                if(i == width)
                {
                    j += shift;
                    i = 0;
                    y++;
                }
                fData[j] = *fftData[0] / scale;
                fftData++;
                i++;
            }
        }
    }
    else
    {
        for(j = 0; j < size; j++)
        {
            if(i == width)
            {
                j += shift;
                i = 0;
                y++;
            }
            fData[j] = *fftData[0];
            fData[j] = fData[j] * pow(-1.0, i + y) / sizeD;
            fData[j] = exp(fData[j]) - 1.0;
            fData[j] /= maxValue;
            fftData++;
            i++;
        }
    }
}

// производит фильтрацию изображения в частотной области
void FFTFilter(unsigned int width, unsigned int alignedWidth, unsigned int height, fftw_complex *imageData, int filterType, int filterRadius, double lowGamma, double highGamma, double butterworthN)
{
    if(imageData == 0)
    {
        return;
    }
    unsigned int x, y, index, index2, width_2;
    double u, v, q;
    float D, xf, yf, yScale;
    double sample;
    x = 0;
    y = 0;
    yScale = alignedWidth;
    yScale /= height;
    width_2 = alignedWidth / 2.0 + 1.0;
    
    double dx, dy;
    dx = 10;
    dy = 0;
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width_2; x++)
        {
            index = x + y * width_2;
            xf = (float)x - (float)width / 2;
            yf = (float)y - (float)height / 2;
            yf *= yScale;
            D = sqrt(xf * xf + yf * yf);
            u = x;
            u /= alignedWidth;
            v = y;
            v /= height;
            
            switch(filterType)
            {
                    // идеальный фильтр низких частот
                case 0:
                {
                    if(D > filterRadius)
                    {
                        imageData[index][0] *= 0.0;
                        imageData[index][1] *= 0.0;
                    }
                    
                    break;
                }
                    // Гауссов фильтр низких частот
                case 2:
                {
                    sample = exp(-(D * D)/(2 * (filterRadius * filterRadius)));
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // Гауссов фильтр высоких частот
                case 6:
                {
                    sample = 1 - exp(-(D * D)/(2 * (filterRadius * filterRadius)));
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // Гомоморфный фильтр
                case 8:
                {
                    sample = (highGamma - lowGamma) * (1 - exp(-butterworthN * (D * D) / (filterRadius * filterRadius))) + lowGamma;
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // сдвиг фазы
                case 9:
                {
                    q = u * dx + v * dy;
                    fftw_complex f;
                    f[0] = cos(2*M_PI*q);
                    f[1] = -sin(2*M_PI*q);
                    
                    imageData[index][0] = (imageData[index][0]*f[0] - imageData[index][1]*f[1]);
                    imageData[index][1] = (imageData[index][0]*f[1] + imageData[index][1]*f[0]);
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

// производит фильтрацию изображения в частотной области
void FFTFilterFull(unsigned int width, unsigned int alignedWidth, unsigned int height, fftw_complex *imageData, int filterType, int filterRadius, double lowGamma, double highGamma, double butterworthN)
{
    if(imageData == 0)
    {
        return;
    }
    unsigned int x, y, index, index2, width_2;
    double u, v, q;
    float D, xf, yf, yScale;
    double sample;
    x = 0;
    y = 0;
    yScale = alignedWidth;
    yScale /= height;
    width_2 = alignedWidth / 2.0 + 1.0;
    
    double dx, dy;
    dx = 256;
    dy = 256;
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            index = x + y * width;
            xf = (float)x - (float)width / 2;
            yf = (float)y - (float)height / 2;
            yf *= yScale;
            D = sqrt(xf * xf + yf * yf);
            u = x;
            u -= width/2;
            u /= alignedWidth;
            v = y;
            v -= width/2;
            v /= height;
            
            switch(filterType)
            {
                    // идеальный фильтр низких частот
                case 0:
                {
                    if(D > filterRadius)
                    {
                        imageData[index][0] *= 0.0;
                        imageData[index][1] *= 0.0;
                    }
                    
                    break;
                }
                    // Гауссов фильтр низких частот
                case 2:
                {
                    sample = exp(-(D * D)/(2 * (filterRadius * filterRadius)));
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // Гауссов фильтр высоких частот
                case 6:
                {
                    sample = 1 - exp(-(D * D)/(2 * (filterRadius * filterRadius)));
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // Гомоморфный фильтр
                case 8:
                {
                    sample = (highGamma - lowGamma) * (1 - exp(-butterworthN * (D * D) / (filterRadius * filterRadius))) + lowGamma;
                    
                    imageData[index][0] *= sample;
                    imageData[index][1] *= sample;
                    
                    break;
                }
                    // сдвиг фазы
                case 9:
                {
                    q = u * dx + v * dy;
                    fftw_complex f;
                    f[0] = cos(2*M_PI*q);
                    f[1] = -sin(2*M_PI*q);
                    
                    index2 = (width - x) + y * width;
                    
                    imageData[index][0] = imageData[index2][0];
                    imageData[index][1] = imageData[index2][1];
                    
                    //imageData[index][0] = (imageData[index][0]*f[0] - imageData[index][1]*f[1]);
                    //imageData[index][1] = (imageData[index][0]*f[1] + imageData[index][1]*f[0]);
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

// сдвиг от кадра I0 к кадру I1
IMPoint IMMotionAnalyser::shiftFromI0ToI1(IMImageFile *I0, IMImageFile*I1, double& correlation)
{
    IMPoint resultShift;
    
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();

    fftw_complex *baseImageDataFFT, *baseImageDataFFTSrc, *baseImage2DataFFT, *baseImage2DataFFTSrc;
    fftw_plan baseImagePlan, baseImageInversePlan, baseImage2Plan, baseImage2InversePlan;
    int baseImageSize, baseImageWidth, baseImageHeight, x, y, index;
    double sample, sample2, a, b, c, d, e, f, modulus;
    float *fftImageData, *newImageData32;
    IMImageFile fftImage, newImage32;
    IMSize cellSize;
    
    baseImageWidth = I0->pixelsWide();
    baseImageHeight = I0->pixelsHigh();
    
    cellSize.width = I0->pixelsWide();
    cellSize.height = I0->pixelsHigh();
    
    baseImageSize = baseImageWidth * baseImageHeight;


    // выделяем память под данные исходного изображения
    baseImageDataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImageDataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImage2DataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImage2DataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));

    // создаём планы прямого и обратного преобразовния Фурье
    baseImagePlan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImageDataFFTSrc, baseImageDataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
    baseImage2Plan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImage2DataFFTSrc, baseImage2DataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
    baseImage2InversePlan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImage2DataFFT, baseImage2DataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);

    // окно Хэмминга
    if(m_hammingWindow == 0)
    {
        m_hammingWindow = window2d(cellSize.width, cellSize.height);
    }

    // вычисляем преобразование Фурье

    // приводим изображение к типу double
    iFFTToFFT(cellSize.width, cellSize.width, cellSize.height, I0->image(), baseImageDataFFTSrc, false, false, m_hammingWindow);
    iFFTToFFT(cellSize.width, cellSize.width, cellSize.height, I1->image(), baseImage2DataFFTSrc, false, false, m_hammingWindow);

    // производим преобразование Фурье
    fftw_execute(baseImagePlan);
    fftw_execute(baseImage2Plan);

    // свёртка в частотной области
    if(true)
    {
        if(0)
        {
            // создаём изображение - Фурье спектр
            fftImage.setPixelsWide(baseImageWidth);
            fftImage.setPixelsHigh(baseImageHeight);
            fftImage.setSamplesPerPixel(1);
            fftImage.setBitsPerSample(32);
            fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
            fftImage.setFileFormat(new IMTIFFImageFileFormat);
            fftImage.allocImage();

            fftImageData = (float*)fftImage.image();
        }

        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < baseImageWidth; x++)
            {
                index = x + y * baseImageWidth;

                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;

                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                sample2 = sqrt(baseImage2DataFFT[index][0] * baseImage2DataFFT[index][0] + baseImage2DataFFT[index][1] * baseImage2DataFFT[index][1]) / (baseImageWidth * baseImageHeight);


                a = baseImageDataFFT[index][0];
                b = baseImageDataFFT[index][1];

                c = baseImage2DataFFT[index][0];
                d = -baseImage2DataFFT[index][1];

                // произведение комплексных чисел
                e = a*c - b*d;
                f = a*d + b*c;

                // модуль произведения
                modulus = sqrt(e*e + f*f);

                if(modulus == 0)
                {
                    modulus = 1e-32;
                }

                e /= modulus;
                f /= modulus;

                baseImage2DataFFT[index][0] = e;
                baseImage2DataFFT[index][1] = f;

                //sample = modulus;
                if(0)
                {
                    fftImageData[x + y * baseImageWidth] = sample;
                }
            }
        }
        if(0)
        {
            fftImage.save("fft.tif");
        }
    }

    // производим обратное преобразование Фурье
    fftw_execute(baseImage2InversePlan);
    
    if(0)
    {
        newImage32.setPixelsWide(baseImageWidth);
        newImage32.setPixelsHigh(baseImageHeight);
        newImage32.setSamplesPerPixel(1);
        newImage32.setBitsPerSample(32);
        newImage32.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        newImage32.setFileFormat(new IMTIFFImageFileFormat);
        newImage32.allocImage();
        newImageData32 = reinterpret_cast<float*>(newImage32.image());
        FFTToF(baseImageWidth, baseImageWidth, baseImageHeight, newImageData32, baseImage2DataFFTSrc, false, 32, false);
        newImage32.save("correlation.tiff");
        newImage32.freeImage();
    }
    
    double max1val, max2val;
    IMPoint secondMax;

    IMPoint point = maxLocation(baseImage2DataFFTSrc, baseImageWidth, baseImageHeight, secondMax, max1val, max2val);
    
    if(point.x > cellSize.width / 2)
    {
        point.x -= cellSize.width;
    }
    if(point.y > cellSize.height / 2)
    {
        point.y -= cellSize.height;
    }
    point.x = -point.x;
    point.y = -point.y;
    
    auto t2 = Clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    //std::cout << time_span.count() << "\n";

    // освобождаем память
    fftw_destroy_plan(baseImagePlan);
    fftw_destroy_plan(baseImage2Plan);
    fftw_destroy_plan(baseImage2InversePlan);
    fftw_free(baseImageDataFFT);
    fftw_free(baseImageDataFFTSrc);
    fftw_free(baseImage2DataFFT);
    fftw_free(baseImage2DataFFTSrc);
    //free(newImageData32);
    fftw_cleanup();
    fftImage.freeImage();
    
    resultShift.x = point.x;
    resultShift.y = point.y;
    
    correlation = max1val / baseImageSize;
    return resultShift;
}

// прослеживание объекта на последовательности изображений, начиная с кадра nFrame,
// до тех пор пока коэффициент корреляции больше minCorrelation
void IMMotionAnalyser::trackObjectAtFrame(int nFrame, IMPoint point, IMSize searchWindowSize, double minCorrelation)
{
    //cout << "trackObjectAtFrame" << endl;
    
    // проверяем, что указанный кадр есть в последовательности
    if(nFrame >= m_imageSeqence.size())
    {
        return;
    }
    
    IMImageResizer resizer;
    IMImageFile *baseImage, *curImage, *outputImage;
    int curImageN;
    
    
    // создаём эталон
    m_imageSeqence[nFrame]->allocAndLoadImage();
    resizer.setImage(m_imageSeqence[nFrame]);
    baseImage = resizer.newImageWithCenterAtPoint(point, searchWindowSize);
    
    char curImageName[256];
    
    // цикл по всем изображениям
    for(curImageN = nFrame+1; curImageN < m_imageSeqence.size(); curImageN++)
    {
        sprintf(curImageName, (m_outputImagePath + "/" + m_outputImageNameFormat).c_str(), curImageN);
        
        m_imageSeqence[curImageN]->allocAndLoadImage();
        resizer.setImage(m_imageSeqence[curImageN]);
        curImage = resizer.newImageWithCenterAtPoint(point, searchWindowSize);

        double correlation = 1.0;
        IMPoint shift;
        // вычисляем сдвиг между изображениями
        shift = shiftFromI0ToI1(baseImage, curImage, correlation);
        
        if(correlation < minCorrelation)
        {
            m_imageSeqence[curImageN]->freeImage();
            curImage->freeImage();
            break;
        }
        
        curImage->freeImage();
        
        resizer.setImage(m_imageSeqence[curImageN]);
        
        // перемещаем точку слежения
        point.x += shift.x;
        point.y += shift.y;
        
        // сохраняем фрагмент текущего изображения, содержащий прослеживаемый объект
        outputImage = resizer.newImageWithCenterAtPoint(point, m_outputImageSize);
        outputImage->setFileFormat(new IMTIFFImageFileFormat);
        outputImage->save(curImageName);
        
        outputImage->freeImage();
        m_imageSeqence[curImageN]->freeImage();
    }
    
    // освобождаем ресурсы
    m_imageSeqence[nFrame]->freeImage();
    baseImage->freeImage();
    
    m_hammingWindow->freeImage();
    delete m_hammingWindow;
    m_hammingWindow = 0;
}

// оценка оптического потока методом Хорна-Шунка
void IMMotionAnalyser::estimateHSFlow()
{
    cout << "Estimating optical flow by Horn-Shunk algorithm" << endl;
    
    IMImageFile *flow, *avgFlow, *Ex, *Ey, *Et, *result32;
    int i, j, n, width, height, index;
    double *curData, *ExData, *EyData, *EtData;
    double alpha, value;
    IMOpticalFlow *flowData, *avgFlowData;
    unsigned char *image1Data, *image2Data;
    char filename[128];
    
    // проверяем, что заданы два изображения
    if(m_imageSeqence.size() < 2)
    {
        cout << "Error: not enough images in sequence" << endl;
        return;
    }
    
    width = m_imageSeqence[0]->pixelsWide();
    height = m_imageSeqence[0]->pixelsHigh();
    image1Data = m_imageSeqence[0]->image();
    image2Data = m_imageSeqence[1]->image();
    index = 0;
    n = 0;
    alpha = 1.0;
    
    // создаём массив оптивческого потока
    flow = new IMImageFile;
    flow->setPixelsWide(m_imageSeqence[0]->pixelsWide());
    flow->setPixelsHigh(m_imageSeqence[0]->pixelsHigh());
    flow->setBitsPerSample(64);
    flow->setSamplesPerPixel(2);
    flow->setBytesPerRow(sizeof(double) * 2 * flow->pixelsWide());
    flow->allocImage();
    
    avgFlow = new IMImageFile;
    avgFlow->setPixelsWide(m_imageSeqence[0]->pixelsWide());
    avgFlow->setPixelsHigh(m_imageSeqence[0]->pixelsHigh());
    avgFlow->setBitsPerSample(64);
    avgFlow->setSamplesPerPixel(2);
    avgFlow->setBytesPerRow(sizeof(double) * 2 * flow->pixelsWide());
    avgFlow->allocImage();
    
    // присваиваем начальное значение – нулевой оптический поток
    memset(flow->image(), 0, sizeof(double) * 2 * flow->pixelsWide());
    
    // ----------
    // вычисляем частные производные Ex, Ey, Et
    // ----------
    Ex = new IMImageFile;
    Ey = new IMImageFile;
    Et = new IMImageFile;
    
    Ex->setPixelsWide(width);
    Ex->setPixelsHigh(height);
    Ex->setBitsPerSample(64);
    Ex->setSamplesPerPixel(1);
    Ex->setBytesPerRow(sizeof(double) * width);
    Ex->allocImage();
    
    Ey->setPixelsWide(width);
    Ey->setPixelsHigh(height);
    Ey->setBitsPerSample(64);
    Ey->setSamplesPerPixel(1);
    Ey->setBytesPerRow(sizeof(double) * width);
    Ey->allocImage();
    
    Et->setPixelsWide(width);
    Et->setPixelsHigh(height);
    Et->setBitsPerSample(64);
    Et->setSamplesPerPixel(1);
    Et->setBytesPerRow(sizeof(double) * width);
    Et->allocImage();
    
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();
    
    // вычисляем Ex
    curData = reinterpret_cast<double*>(Ex->image());
    // цикл по строкам исходного изображения
    for(j = 0; j < width-1; j++)
    {
        // цикл по столбцам исходного изображения
        for(i = 0; i < height-1; i++)
        {
            index = i * width + j;
            
            curData[index] = image1Data[index + 1] - image1Data[index] + image1Data[index + width + 1] - image1Data[index + width];
            curData[index] += image2Data[index + 1] - image2Data[index] + image2Data[index + width + 1] - image2Data[index + width];
            curData[index] /= 4.0;
        }
    }
    
    auto t2 = Clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << time_span.count() << '\n';
    
    // переводим результат в 32-бит
    /*result32 = convert64biTo32bit(Ex);
    result32->setFileFormat(new IMTIFFImageFileFormat);
    result32->save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/Ex.tiff");
    delete result32;*/
    
    // вычисляем Ey
    curData = reinterpret_cast<double*>(Ey->image());
    // цикл по строкам исходного изображения
    for(j = 0; j < width-1; j++)
    {
        // цикл по столбцам исходного изображения
        for(i = 0; i < height-1; i++)
        {
            index = i * width + j;
            
            curData[index] = image1Data[index + width] - image1Data[index] + image1Data[index + width + 1] - image1Data[index + 1];
            curData[index] += image2Data[index + width] - image2Data[index] + image2Data[index + width + 1] - image2Data[index + 1];
            curData[index] /= 4.0;
        }
    }
    
    // переводим результат в 32-бит
    /*result32 = convert64biTo32bit(Ey);
    result32->setFileFormat(new IMTIFFImageFileFormat);
    result32->save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/Ey.tiff");
    delete result32;*/
    
    // вычисляем Et
    curData = reinterpret_cast<double*>(Et->image());
    // цикл по строкам исходного изображения
    for(j = 0; j < width-1; j++)
    {
        // цикл по столбцам исходного изображения
        for(i = 0; i < height-1; i++)
        {
            index = i * width + j;
            
            curData[index] = image2Data[index] - image1Data[index] + image2Data[index + width] - image1Data[index + width];
            curData[index] += image2Data[index + 1] - image1Data[index + 1] + image2Data[index + width + 1] - image1Data[index + width + 1];
            curData[index] /= 4.0;
        }
    }
    
    // переводим результат в 32-бит
    /*result32 = convert64biTo32bit(Et);
    result32->setFileFormat(new IMTIFFImageFileFormat);
    result32->save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/Et.tiff");
    delete result32;*/
    
    // TEMP
    /*flowData = reinterpret_cast<IMOpticalFlow*>(flow->image());
    for(j = 0; j < width; j++)
    {
        // цикл по столбцам исходного потока
        for(i = 0; i < height; i++)
        {
            index = i * width + j;
            
            flowData[index].u = image1Data[index];
            flowData[index].v = image2Data[index];
        }
    }
    // переводим результат в 32-бит
    saveFlowTo32bit(flow, "/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow", "test")*/
    
    flowData = reinterpret_cast<IMOpticalFlow*>(flow->image());
    avgFlowData = reinterpret_cast<IMOpticalFlow*>(avgFlow->image());
    ExData = reinterpret_cast<double*>(Ex->image());
    EyData = reinterpret_cast<double*>(Ey->image());
    EtData = reinterpret_cast<double*>(Et->image());
    
    // ----------
    // итеративный цикл вычисления оптического потока
    // ----------
    while(n < 4)
    {
        // ----------
        // вычисляем среднеие значения оптического потока flowAvg
        // ----------
        // цикл по строкам исходного потока
        for(j = 1; j < width-1; j++)
        {
            // цикл по столбцам исходного потока
            for(i = 1; i < height-1; i++)
            {
                index = i * width + j;
                
                avgFlowData[index].u = (flowData[index - width].u + flowData[index + 1].u + flowData[index + width].u + flowData[index - 1].u)/6.0;
                avgFlowData[index].u += (flowData[index - width - 1].u + flowData[index - width + 1].u + flowData[index + width + 1].u + flowData[index + width - 1].u)/12.0;
                
                avgFlowData[index].v = (flowData[index - width].v + flowData[index + 1].v + flowData[index + width].v + flowData[index - 1].v)/6.0;
                avgFlowData[index].v += (flowData[index - width - 1].v + flowData[index - width + 1].v + flowData[index + width + 1].v + flowData[index + width - 1].v)/12.0;
            }
        }
        
        // сохраняем результат в 32-бит
        //saveFlowTo32bit(avgFlow, "/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow", "avg");
        
        // ----------
        // вычисляем значение оптического потока на текущем шаге итерации
        // ----------
        // цикл по строкам исходного потока
        for(j = 1; j < width-1; j++)
        {
            // цикл по столбцам исходного потока
            for(i = 1; i < height-1; i++)
            {
                index = i * width + j;
                
                value = (ExData[index] * avgFlowData[index].u + EyData[index] * avgFlowData[index].v + EtData[index])/(alpha*alpha + ExData[index]*ExData[index] + EyData[index]*EyData[index]);
                
                flowData[index].u = avgFlowData[index].u - ExData[index]*value;
                flowData[index].v = avgFlowData[index].v - EyData[index]*value;
            }
        }
        
        n++;
    }
    
    sprintf(filename, "%03d", n);
    // сохраняем результат в 32-бит
    //saveFlowTo32bit(avgFlow, "/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow", filename);
    
    m_flowSequence.push_back(flow);
    
    // освобождаем память
    delete Ex;
    delete Ey;
    delete Et;
}

// проверка функций FFT
void IMMotionAnalyser::fft()
{
    cout << "FFT" << endl;
    
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();

    
    fftw_complex *baseImageDataFFT;
    fftw_plan baseImagePlan, baseImageInversePlan;
    int baseImageSize, baseImageWidth, baseImageHeight, logarithm, x, y, index, filterType, filterRadius;
    double sample, lowGamma, highGamma, butterworthN;
    unsigned char *image1Data, *image2Data;
    double *baseImageDataR64;
    float *fftImageData, *newImageData32;
    IMImageFile fftImage, newImage32;
    
    baseImageWidth = m_imageSeqence[0]->pixelsWide();
    baseImageHeight = m_imageSeqence[0]->pixelsHigh();
    baseImageSize = baseImageWidth * baseImageHeight;
    image1Data = m_imageSeqence[0]->image();
    image2Data = m_imageSeqence[1]->image();
    logarithm = false;
    
    // выделяем память под данные исходного изображения
    newImageData32 = reinterpret_cast<float*>(malloc(baseImageSize * sizeof(float)));
    baseImageDataR64 = reinterpret_cast<double*>(malloc(baseImageSize * sizeof(double)));
    baseImageDataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    
    // приводим изображение к типу double
    iFFTToD(baseImageWidth, baseImageWidth, baseImageHeight, image1Data, baseImageDataR64, logarithm);
    
    // создаём планы прямого и обратного преобразовния Фурье
    baseImagePlan = fftw_plan_dft_r2c_2d(baseImageHeight, baseImageWidth, baseImageDataR64, baseImageDataFFT, FFTW_ESTIMATE);
    baseImageInversePlan = fftw_plan_dft_c2r_2d(baseImageHeight, baseImageWidth, baseImageDataFFT, baseImageDataR64, FFTW_ESTIMATE);
    
    // производим преобразование Фурье
    fftw_execute(baseImagePlan);
    
    if(true)
    {
        // создаём изображение - Фурье спектр
        fftImage.setPixelsWide(baseImageWidth);
        fftImage.setPixelsHigh(baseImageHeight);
        fftImage.setSamplesPerPixel(1);
        fftImage.setBitsPerSample(32);
        fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        fftImage.setFileFormat(new IMTIFFImageFileFormat);
        fftImage.allocImage();

        fftImageData = (float*)fftImage.image();
        
        
        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < (baseImageWidth / 2.0 + 1.0); x++)
            {
                index = x + y * (baseImageWidth / 2.0 + 1.0);
                
                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;
                
                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                fftImageData[x + y * baseImageWidth] = sample;
                fftImageData[(baseImageWidth - x - 1) + (baseImageHeight - y - 1) * baseImageWidth] = sample;
            }
        }
        
        fftImage.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fft.tiff");
    }
    
    filterType = 9;
    filterRadius = 10;
    
    // производим фильтрацию
    FFTFilter(baseImageWidth, baseImageWidth, baseImageHeight, baseImageDataFFT, filterType, filterRadius, lowGamma, highGamma, butterworthN);
    
    // производим обратное преобразование Фурье
    fftw_execute(baseImageInversePlan);
    
    dFFTToF(baseImageWidth, baseImageWidth, baseImageHeight, newImageData32, baseImageDataR64, logarithm, 8);
    newImage32.setPixelsWide(baseImageWidth);
    newImage32.setPixelsHigh(baseImageHeight);
    newImage32.setSamplesPerPixel(1);
    newImage32.setBitsPerSample(32);
    newImage32.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
    newImage32.setFileFormat(new IMTIFFImageFileFormat);
    newImage32.setImage(reinterpret_cast<unsigned char*>(newImageData32));
    newImage32.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fftRes.tiff");
    
    auto t2 = Clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << time_span.count() << '\n';
    
    // освобождаем память
    fftw_free(baseImageDataFFT);
}

// проверка функций FFT
void IMMotionAnalyser::fft2()
{
    cout << "FFT" << endl;
    
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();
    
    
    fftw_complex *baseImageDataFFT, *baseImageDataFFTSrc;
    fftw_plan baseImagePlan, baseImageInversePlan;
    int baseImageSize, baseImageWidth, baseImageHeight, logarithm, x, y, index, filterType, filterRadius;
    double sample, lowGamma, highGamma, butterworthN;
    unsigned char *image1Data, *image2Data;
    double *baseImageDataR64;
    float *fftImageData, *newImageData32;
    IMImageFile fftImage, newImage32;
    
    baseImageWidth = m_imageSeqence[0]->pixelsWide();
    baseImageHeight = m_imageSeqence[0]->pixelsHigh();
    baseImageSize = baseImageWidth * baseImageHeight;
    image1Data = m_imageSeqence[0]->image();
    image2Data = m_imageSeqence[1]->image();
    logarithm = false;
    
    // выделяем память под данные исходного изображения
    newImageData32 = reinterpret_cast<float*>(malloc(baseImageSize * sizeof(float)));
    baseImageDataR64 = reinterpret_cast<double*>(malloc(baseImageSize * sizeof(double)));
    baseImageDataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImageDataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    
    // приводим изображение к типу double
    iFFTToFFT(baseImageWidth, baseImageWidth, baseImageHeight, image1Data, baseImageDataFFTSrc, logarithm);
    
    // создаём планы прямого и обратного преобразовния Фурье
    baseImagePlan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImageDataFFTSrc, baseImageDataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
    baseImageInversePlan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImageDataFFT, baseImageDataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // производим преобразование Фурье
    fftw_execute(baseImagePlan);
    
    if(false)
    {
        // создаём изображение - Фурье спектр
        fftImage.setPixelsWide(baseImageWidth);
        fftImage.setPixelsHigh(baseImageHeight);
        fftImage.setSamplesPerPixel(1);
        fftImage.setBitsPerSample(32);
        fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        fftImage.setFileFormat(new IMTIFFImageFileFormat);
        fftImage.allocImage();
        
        fftImageData = (float*)fftImage.image();
        
        
        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < (baseImageWidth / 2.0 + 1.0); x++)
            {
                index = x + y * (baseImageWidth / 2.0 + 1.0);
                
                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;
                
                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                fftImageData[x + y * baseImageWidth] = sample;
                fftImageData[(baseImageWidth - x - 1) + (baseImageHeight - y - 1) * baseImageWidth] = sample;
            }
        }
        
        fftImage.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fft.tiff");
    }
    
    if(true)
    {
        // создаём изображение - Фурье спектр
        fftImage.setPixelsWide(baseImageWidth);
        fftImage.setPixelsHigh(baseImageHeight);
        fftImage.setSamplesPerPixel(1);
        fftImage.setBitsPerSample(32);
        fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        fftImage.setFileFormat(new IMTIFFImageFileFormat);
        fftImage.allocImage();
        
        fftImageData = (float*)fftImage.image();
        
        
        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < baseImageWidth; x++)
            {
                index = x + y * baseImageWidth;
                
                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;
                
                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                fftImageData[x + y * baseImageWidth] = sample;
            }
        }
        
        fftImage.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fft.tiff");
    }
    
    filterType = 9;
    filterRadius = 10;
    
    // производим фильтрацию
    FFTFilterFull(baseImageWidth, baseImageWidth, baseImageHeight, baseImageDataFFT, filterType, filterRadius, lowGamma, highGamma, butterworthN);
    
    if(true)
    {
        // создаём изображение - Фурье спектр
        fftImage.setPixelsWide(baseImageWidth);
        fftImage.setPixelsHigh(baseImageHeight);
        fftImage.setSamplesPerPixel(1);
        fftImage.setBitsPerSample(32);
        fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        fftImage.setFileFormat(new IMTIFFImageFileFormat);
        fftImage.allocImage();
        
        fftImageData = (float*)fftImage.image();
        
        
        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < baseImageWidth; x++)
            {
                index = x + y * baseImageWidth;
                
                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;
                
                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                fftImageData[x + y * baseImageWidth] = sample;
            }
        }
        
        fftImage.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fft2.tiff");
    }
    
    // производим обратное преобразование Фурье
    fftw_execute(baseImageInversePlan);
    
    FFTToF(baseImageWidth, baseImageWidth, baseImageHeight, newImageData32, baseImageDataFFTSrc, logarithm, 8);
    newImage32.setPixelsWide(baseImageWidth);
    newImage32.setPixelsHigh(baseImageHeight);
    newImage32.setSamplesPerPixel(1);
    newImage32.setBitsPerSample(32);
    newImage32.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
    newImage32.setFileFormat(new IMTIFFImageFileFormat);
    newImage32.setImage(reinterpret_cast<unsigned char*>(newImageData32));
    newImage32.save("/Users/vladimirknyaz/Documents/WORK/2015/150406_Hercules/OpticalFlow/fftRes2.tiff");
    
    auto t2 = Clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << time_span.count() << '\n';
    
    // освобождаем память
    fftw_free(baseImageDataFFT);
}

// оценка движения и ядра смаза
void IMMotionAnalyser::psf()
{
    cout << "PSF" << endl;
    
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now();

    
    fftw_complex *baseImageDataFFT, *baseImageDataFFTSrc, *baseImage2DataFFT, *baseImage2DataFFTSrc;
    fftw_plan baseImagePlan, baseImageInversePlan, baseImage2Plan, baseImage2InversePlan;
    int baseImageSize, baseImageWidth, baseImageHeight, logarithm, x, y, index, filterType, filterRadius;
    double sample, lowGamma, highGamma, butterworthN, sample2, numerator, denomniator, a, b, c, d, e, f, modulus;
    unsigned char *image1Data, *image2Data;
    double *baseImageDataR64;
    float *fftImageData, *newImageData32;
    IMImageFile fftImage, newImage32;
    
    baseImageWidth = m_imageSeqence[0]->pixelsWide();
    baseImageHeight = m_imageSeqence[0]->pixelsHigh();
    baseImageSize = baseImageWidth * baseImageHeight;
    image1Data = m_imageSeqence[0]->image();
    image2Data = m_imageSeqence[1]->image();
    logarithm = false;
    
    // выделяем память под данные исходного изображения
    newImageData32 = reinterpret_cast<float*>(malloc(baseImageSize * sizeof(float)));
    baseImageDataR64 = reinterpret_cast<double*>(malloc(baseImageSize * sizeof(double)));
    baseImageDataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImageDataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImage2DataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
    baseImage2DataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));


    
    // приводим изображение к типу double
    iFFTToFFT(baseImageWidth, baseImageWidth, baseImageHeight, image1Data, baseImageDataFFTSrc, logarithm);
    iFFTToFFT(baseImageWidth, baseImageWidth, baseImageHeight, image2Data, baseImage2DataFFTSrc, logarithm);
    
    // создаём планы прямого и обратного преобразовния Фурье
    baseImagePlan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImageDataFFTSrc, baseImageDataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
    baseImageInversePlan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImageDataFFT, baseImageDataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);
    baseImage2Plan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImage2DataFFTSrc, baseImage2DataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
    baseImage2InversePlan = fftw_plan_dft_2d(baseImageHeight, baseImageWidth, baseImage2DataFFT, baseImage2DataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);

    
    // производим преобразование Фурье
    fftw_execute(baseImagePlan);
    fftw_execute(baseImage2Plan);
    
    if(true)
    {
        // создаём изображение - Фурье спектр
        fftImage.setPixelsWide(baseImageWidth);
        fftImage.setPixelsHigh(baseImageHeight);
        fftImage.setSamplesPerPixel(1);
        fftImage.setBitsPerSample(32);
        fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
        fftImage.setFileFormat(new IMTIFFImageFileFormat);
        fftImage.allocImage();
        
        fftImageData = (float*)fftImage.image();
        
        
        // заполняем изображение - Фурье спектр
        for(y = 0; y < baseImageHeight; y++)
        {
            for(x = 0; x < baseImageWidth; x++)
            {
                index = x + y * baseImageWidth;
                
                //u = x - baseImageWidth / 2.0 ;
                //v = y - baseImageHeight / 2.0;
                
                sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                sample2 = sqrt(baseImage2DataFFT[index][0] * baseImage2DataFFT[index][0] + baseImage2DataFFT[index][1] * baseImage2DataFFT[index][1]) / (baseImageWidth * baseImageHeight);
                
                
                a = baseImageDataFFT[index][0];
                b = baseImageDataFFT[index][1];
                
                c = baseImage2DataFFT[index][0];
                d = -baseImage2DataFFT[index][1];
                
                // произведение комплексных чисел
                e = a*c;
                f = b*d;
                
                // модуль произведения
                modulus = sqrt(e*e + f*f);
                
                e /= modulus;
                f /= modulus;
                
                baseImage2DataFFT[index][0] = e;
                baseImage2DataFFT[index][1] = f;
                
                sample = modulus;
                
                fftImageData[x + y * baseImageWidth] = sample;
            }
        }
        
        //fftImage.save((BASE_PATH + "fft.tiff").c_str());
    }
    
    // производим обратное преобразование Фурье
    fftw_execute(baseImageInversePlan);
    fftw_execute(baseImage2InversePlan);
    
    FFTToF(baseImageWidth, baseImageWidth, baseImageHeight, newImageData32, baseImage2DataFFTSrc, logarithm, 8);
    newImage32.setPixelsWide(baseImageWidth);
    newImage32.setPixelsHigh(baseImageHeight);
    newImage32.setSamplesPerPixel(1);
    newImage32.setBitsPerSample(32);
    newImage32.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
    newImage32.setFileFormat(new IMTIFFImageFileFormat);
    newImage32.setImage(reinterpret_cast<unsigned char*>(newImageData32));
    //newImage32.save((BASE_PATH + "correlation.tiff").c_str());
    
    auto t2 = Clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << time_span.count() << '\n';
    
    // освобождаем память
    fftw_free(baseImageDataFFT);

}

IMPoint maxLocation(fftw_complex *image, int width, int height, IMPoint& secondMax, double& max1val, double& max2val)
{
    IMPoint point;
    point.x = 0.0;
    point.y = 0.0;
    secondMax.x = 0.0;
    secondMax.y = 0.0;
    
    int x, y;
    double max = -1e10;
    double max2 = -1e10;
    double dist, dx, dy;
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            if(image[x + y*height][0] > max)
            {
                max = image[x + y*height][0];
                point.x = x;
                point.y = y;
            }
        }
    }
    
    max1val = max;
    max2val = max2;
    
    return point;
}

/*
int m, n;      // FFT row and column dimensions might be different
int m2, n2;
int i, k;
complex x[m][n];
complex tmp13, tmp24;

m2 = m / 2;    // half of row dimension
n2 = n / 2;    // half of column dimension

// interchange entries in 4 quadrants, 1 <--> 3 and 2 <--> 4

for (i = 0; i < m2; i++)
{
    for (k = 0; k < n2; k++)
    {
        tmp13         = x[i][k];
        x[i][k]       = x[i+m2][k+n2];
        x[i+m2][k+n2] = tmp13;
        
        tmp24         = x[i+m2][k];
        x[i+m2][k]    = x[i][k+n2];
        x[i][k+n2]    = tmp24;
    }
}
*/

void fftshift(fftw_complex *x, int m, int n)
{
    //int m, n;      // FFT row and column dimensions might be different
    int m2, n2;
    int i, k;
    //fftw_complex x[m][n];
    fftw_complex tmp13, tmp24;
    
    m2 = m / 2;    // half of row dimension
    n2 = n / 2;    // half of column dimension
    
    // interchange entries in 4 quadrants, 1 <--> 3 and 2 <--> 4
    
    for (i = 0; i < m2; i++)
    {
        for (k = 0; k < n2; k++)
        {
            memcpy(&tmp13, &x[i*m + k], sizeof(fftw_complex));
            memcpy(&x[i*m + k], &x[(i+m2)*m + (k+n2)], sizeof(fftw_complex));
            memcpy(&x[(i+m2)*m + (k+n2)], &tmp13, sizeof(fftw_complex));
            
            memcpy(&tmp24, &x[(i+m2)*m + k], sizeof(fftw_complex));
            memcpy(&x[(i+m2)*m + k], &x[i*m + k+n2], sizeof(fftw_complex));
            memcpy(&x[i*m + k+n2], &tmp24, sizeof(fftw_complex));
        }
    }
}

// вычисление оптического потока методом фазовой корреляции
void IMMotionAnalyser::phaseCorrelation(IMSize cellSize)
{
    cout << "phaseCorrelation()" << endl;
    
//    typedef std::chrono::high_resolution_clock Clock;
//    auto t1 = Clock::now();
//    
//    
//    fftw_complex *baseImageDataFFT, *baseImageDataFFTSrc, *baseImage2DataFFT, *baseImage2DataFFTSrc;
//    fftw_plan baseImagePlan, baseImageInversePlan, baseImage2Plan, baseImage2InversePlan;
//    int baseImageSize, baseImageWidth, baseImageHeight, logarithm, x, y, index, filterType, filterRadius;
//    double sample, lowGamma, highGamma, butterworthN, sample2, numerator, denomniator, a, b, c, d, e, f, modulus;
//    unsigned char *image0Data, *image1Data, *image2Data;
//    double *baseImageDataR64;
//    float *timeDiffData;
//    float *fftImageData, *newImageData32;
//    IMImageFile fftImage, newImage32;
//    
//    baseImageWidth = m_imageSeqence[0]->pixelsWide();
//    baseImageHeight = m_imageSeqence[0]->pixelsHigh();
//    
//    image1Data = m_imageSeqence[0]->image();
//    image2Data = m_imageSeqence[1]->image();
//    logarithm = false;
//    
//    int rows, cols, curRow, curCol;
//    cols = baseImageWidth / cellSize.width;
//    rows = baseImageHeight / cellSize.height;
//    IMImageFile** subimages0 = new IMImageFile*[cols*rows];
//    IMImageFile** subimages1 = new IMImageFile*[cols*rows];
//    IMImageResizer resizer0, resizer1;
//    resizer0.setImage(m_imageSeqence[0]);
//    resizer1.setImage(m_imageSeqence[1]);
//    IMRegion curRegion;
//    curRegion.size = cellSize;
//    baseImageSize = cellSize.width * cellSize.height;
//    baseImageWidth = cellSize.width;
//    baseImageHeight = cellSize.height;
//    
//    // выделяем память под данные исходного изображения
//    newImageData32 = reinterpret_cast<float*>(malloc(baseImageSize * sizeof(float)));
//    baseImageDataR64 = reinterpret_cast<double*>(malloc(baseImageSize * sizeof(double)));
//    baseImageDataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
//    baseImageDataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
//    baseImage2DataFFT = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
//    baseImage2DataFFTSrc = reinterpret_cast<fftw_complex*>(fftw_malloc(baseImageSize * sizeof(fftw_complex)));
//    
//    // создаём планы прямого и обратного преобразовния Фурье
//    baseImagePlan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImageDataFFTSrc, baseImageDataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
//    baseImageInversePlan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImageDataFFT, baseImageDataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);
//    baseImage2Plan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImage2DataFFTSrc, baseImage2DataFFT, FFTW_FORWARD, FFTW_ESTIMATE);
//    baseImage2InversePlan = fftw_plan_dft_2d(cellSize.height, cellSize.width, baseImage2DataFFT, baseImage2DataFFTSrc, FFTW_BACKWARD, FFTW_ESTIMATE);
//    
//    // создаём массив оптического потока
//    IMImageFile *flow;
//    flow = new IMImageFile;
//    flow->setPixelsWide(m_imageSeqence[0]->pixelsWide());
//    flow->setPixelsHigh(m_imageSeqence[0]->pixelsHigh());
//    flow->setBitsPerSample(64);
//    flow->setSamplesPerPixel(2);
//    flow->setBytesPerRow(sizeof(double) * 2 * flow->pixelsWide());
//    flow->allocImage();
//    
//    // создаём массив оптического потока
//    IMImageFile *lowResFlow;
//    lowResFlow = new IMImageFile;
//    lowResFlow->setPixelsWide(cols);
//    lowResFlow->setPixelsHigh(rows);
//    lowResFlow->setBitsPerSample(64);
//    lowResFlow->setSamplesPerPixel(2);
//    lowResFlow->setBytesPerRow(sizeof(double) * 2 * lowResFlow->pixelsWide());
//    lowResFlow->allocImage();
//    
//    // создаём массив оптического потока
//    IMImageFile *lowResFlowObject;
//    lowResFlowObject = new IMImageFile;
//    lowResFlowObject->setPixelsWide(cols);
//    lowResFlowObject->setPixelsHigh(rows);
//    lowResFlowObject->setBitsPerSample(64);
//    lowResFlowObject->setSamplesPerPixel(2);
//    lowResFlowObject->setBytesPerRow(sizeof(double) * 2 * lowResFlowObject->pixelsWide());
//    lowResFlowObject->allocImage();
//    
//    // создаём массив производной времени
//    IMImageFile *timeDif;
//    timeDif = new IMImageFile;
//    timeDif->setPixelsWide(baseImageWidth);
//    timeDif->setPixelsHigh(baseImageHeight);
//    timeDif->setBitsPerSample(32);
//    timeDif->setSamplesPerPixel(1);
//    timeDif->setBytesPerRow(sizeof(float) * 1 * timeDif->pixelsWide());
//    timeDif->allocImage();
//    timeDiffData = reinterpret_cast<float*>(timeDif->image());
//    
//    IMImageFile *window = window2d(cellSize.width, cellSize.height);
//    
//    // создаём массив фрагментов изображения
//    for(curRow = 0; curRow < rows; curRow++)
//    {
//        curRegion.origin.y = curRow * cellSize.height;
//        
//        for(curCol = 0; curCol < cols; curCol++)
//        {
//            IMImageFile *curImage0, *curImage1;
//            curRegion.origin.x = curCol * cellSize.width;
//            
//            curImage0 = resizer0.newImageFromRegion(curRegion);
//            curImage1 = resizer1.newImageFromRegion(curRegion);
//            
//            stringstream filename;
//            filename << std::setw(2) << std::setfill('0') << curRow << "_" << std::setw(2) << std::setfill('0') << curCol;
//            
//            image0Data = curImage0->image();
//            image1Data = curImage1->image();
//            
//            // вычитаем изображения
//            for(y = 0; y < baseImageHeight; y++)
//            {
//                for(x = 0; x < baseImageWidth; x++)
//                {
//                    index = x + y * baseImageWidth;
//                    
//                    timeDiffData[index] = abs(static_cast<int>(image0Data[index]) - static_cast<int>(image1Data[index]));
//                }
//            }
//            timeDif->setFileFormat(new IMTIFFImageFileFormat);
//            timeDif->save((BASE_PATH + "/subimages1/" + filename.str() + "_diff.tiff").c_str());
//            
//            IMGraphCutFilter filter;
//            filter.setThreshold(30);
//            filter.setKeyImage(curImage1);
//            filter.setImage(timeDif);
//            filter.labelImage();
//            
//            IMImageFile *result = filter.result();
//            result->setFileFormat(new IMTIFFImageFileFormat);
//            result->save((BASE_PATH + "/subimages1/" + filename.str() + "_label.tiff").c_str());
//            
//            subimages0[curCol + curRow*rows] = curImage0;
//            subimages1[curCol + curRow*rows] = curImage1;
//            
//            curImage0->setFileFormat(new IMTIFFImageFileFormat);
//            curImage0->save((BASE_PATH + "/subimages1/" + filename.str() + "_0.tiff").c_str());
//            
//            curImage1->setFileFormat(new IMTIFFImageFileFormat);
//            curImage1->save((BASE_PATH + "/subimages1/" + filename.str() + "_1.tiff").c_str());
//            
//            
//            // вычисляем преобразование Фурье
//            
//            // приводим изображение к типу double
//            iFFTToFFT(cellSize.width, cellSize.width, cellSize.height, curImage0->image(), baseImageDataFFTSrc, logarithm, false, window);
//            iFFTToFFT(cellSize.width, cellSize.width, cellSize.height, curImage1->image(), baseImage2DataFFTSrc, logarithm, false, window);
//            //iFFTToFFTWithMask(cellSize.width, cellSize.width, cellSize.height, curImage0->image(), baseImageDataFFTSrc, logarithm, false, result->image(), 1.5);
//            //iFFTToFFTWithMask(cellSize.width, cellSize.width, cellSize.height, curImage1->image(), baseImage2DataFFTSrc, logarithm, false, result->image(), -1.5);
//            
//            // производим преобразование Фурье
//            fftw_execute(baseImagePlan);
//            fftw_execute(baseImage2Plan);
//            
//            // меняем квадранты
//            //fftshift(baseImageDataFFT, baseImageWidth, baseImageHeight);
//            //fftshift(baseImage2DataFFT, baseImageWidth, baseImageHeight);
//            
//            if(true)
//            {
//                // создаём изображение - Фурье спектр
//                fftImage.setPixelsWide(baseImageWidth);
//                fftImage.setPixelsHigh(baseImageHeight);
//                fftImage.setSamplesPerPixel(1);
//                fftImage.setBitsPerSample(32);
//                fftImage.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
//                fftImage.setFileFormat(new IMTIFFImageFileFormat);
//                fftImage.allocImage();
//                
//                fftImageData = (float*)fftImage.image();
//                
//                // заполняем изображение - Фурье спектр
//                for(y = 0; y < baseImageHeight; y++)
//                {
//                    for(x = 0; x < baseImageWidth; x++)
//                    {
//                        index = x + y * baseImageWidth;
//                        
//                        //u = x - baseImageWidth / 2.0 ;
//                        //v = y - baseImageHeight / 2.0;
//                        
//                        sample = sqrt(baseImageDataFFT[index][0] * baseImageDataFFT[index][0] + baseImageDataFFT[index][1] * baseImageDataFFT[index][1]) / (baseImageWidth * baseImageHeight);
//                        sample2 = sqrt(baseImage2DataFFT[index][0] * baseImage2DataFFT[index][0] + baseImage2DataFFT[index][1] * baseImage2DataFFT[index][1]) / (baseImageWidth * baseImageHeight);
//                        
//                        
//                        a = baseImageDataFFT[index][0];
//                        b = baseImageDataFFT[index][1];
//                        
//                        c = baseImage2DataFFT[index][0];
//                        d = -baseImage2DataFFT[index][1];
//                        
//                        // произведение комплексных чисел
//                        e = a*c - b*d;
//                        f = a*d + b*c;
//                        
//                        // модуль произведения
//                        modulus = sqrt(e*e + f*f);
//                        
//                        if(modulus == 0)
//                        {
//                            modulus = 1e-32;
//                        }
//                        
//                        e /= modulus;
//                        f /= modulus;
//                        
//                        baseImage2DataFFT[index][0] = e;
//                        baseImage2DataFFT[index][1] = f;
//                        
//                        //sample = modulus;
//                        
//                        fftImageData[x + y * baseImageWidth] = sample;
//                    }
//                }
//                
//                //fftImage.save((BASE_PATH + "/subimages1/" + filename.str() + "_fft.tiff").c_str());
//            }
//            
//            // производим обратное преобразование Фурье
//            fftw_execute(baseImageInversePlan);
//            fftw_execute(baseImage2InversePlan);
//            
//            IMPoint secondMax;
//            double max1val, max2val;
//            IMPoint point = maxLocation(baseImage2DataFFTSrc, baseImageWidth, baseImageHeight, secondMax, max1val, max2val);
//            if(point.x > cellSize.width / 2)
//            {
//                point.x -= cellSize.width;
//            }
//            if(point.y > cellSize.height / 2)
//            {
//                point.y -= cellSize.height;
//            }
//            point.x = -point.x;
//            point.y = -point.y;
//            if(secondMax.x > cellSize.width / 2)
//            {
//                secondMax.x -= cellSize.width;
//            }
//            if(secondMax.y > cellSize.height / 2)
//            {
//                secondMax.y -= cellSize.height;
//            }
//            secondMax.x = -secondMax.x;
//            secondMax.y = -secondMax.y;
//            
//            // TEMP
//            secondMax = point;
//            
//            /*if(secondMax.y >= 0 && curRow > 0)
//            {
//                secondMax.y = 3;
//            }
//            
//            if(point.x == 1.0)
//            {
//                point.x = -2;
//            }*/
//            
//            // TEMP
//            //secondMax = point;
//            
//            
//            /*if(secondMax.x > 60)
//            {
//                secondMax.x = 3;
//            }
//            if(secondMax.y == -14 || secondMax.y == 15)
//            {
//                secondMax.y = 0;
//            }
//            if(secondMax.y == 1 || secondMax.y == 30)
//            {
//                secondMax.y = 60;
//            }*/
//            
//            
//            /*if(max1val / max2val > 1)
//            {
//               secondMax = point;
//            }*/
//            /*if(point.x == 0 && point.y == 0)
//            {
//                 point = secondMax;
//            }*/
//            
//            // secondMax = point;
//            
//            double* lowResFlowImage = reinterpret_cast<double*>(lowResFlow->image());
//            lowResFlowImage[(curCol + curRow*cols)*2 + 0] = secondMax.x;
//            lowResFlowImage[(curCol + curRow*cols)*2 + 1] = secondMax.y;
//            
//            double* lowResFlowObjectImage = reinterpret_cast<double*>(lowResFlowObject->image());
//            lowResFlowObjectImage[(curCol + curRow*cols)*2 + 0] = point.x;
//            lowResFlowObjectImage[(curCol + curRow*cols)*2 + 1] = point.y;
//            
//
//            
//            /*if(secondMax.x == 0)
//            {
//                secondMax.x = point.x;
//            }
//            else if(point.x == 0)
//            {
//                point.x = secondMax.x;
//            }*/
//            
//            int xx, yy, maxX, maxY;
//            
//            xx = curCol*baseImageWidth;
//            yy = curRow*baseImageHeight;
//            maxX = m_imageSeqence[0]->pixelsWide();
//            maxY = m_imageSeqence[0]->pixelsHigh();
//            
//            // сохраняем результат
//            for(y = 0; y < baseImageHeight; y++)
//            {
//                for(x = 0; x < baseImageWidth; x++)
//                {
//                    double* flowImage = reinterpret_cast<double*>(flow->image());
//                    unsigned char* resultData = result->image();
//                    flowImage[((x + xx) + (y+yy)*maxX)*2 + 0] = resultData[x + y*baseImageWidth];
//                    /*if(resultData[x + y*baseImageWidth] > 0)
//                    {
//                        flowImage[((x + xx) + (y+yy)*maxX)*2 + 0] = point.x;
//                        flowImage[((x + xx) + (y+yy)*maxX)*2 + 1] = point.y;
//                    }
//                    else
//                    {
//                        flowImage[((x + xx) + (y+yy)*maxX)*2 + 0] = secondMax.x;
//                        flowImage[((x + xx) + (y+yy)*maxX)*2 + 1] = secondMax.y;
//                    }*/
//                    
//                }
//            }
//            cout << "[" << curRow << "; " << curCol << "]" <<  "(" << max1val << " : " << point.x << ", " << point.y << ")\t";
//            cout << "[" << max2val << " : " << secondMax.x << ", " << secondMax.y << "]\t";
//            
//            FFTToF(baseImageWidth, baseImageWidth, baseImageHeight, newImageData32, baseImage2DataFFTSrc, logarithm, 32, false);
//            newImage32.setPixelsWide(baseImageWidth);
//            newImage32.setPixelsHigh(baseImageHeight);
//            newImage32.setSamplesPerPixel(1);
//            newImage32.setBitsPerSample(32);
//            newImage32.setBytesPerRow(baseImageWidth * 1 * sizeof(float));
//            newImage32.setFileFormat(new IMTIFFImageFileFormat);
//            newImage32.setImage(reinterpret_cast<unsigned char*>(newImageData32));
//            newImage32.save((BASE_PATH + "/subimages1/" + filename.str() + "_correlation.tiff").c_str());
//        }
//        cout << endl;
//    }
//    
//    int maxX, maxY, halfCellWidth, halfCellHeight, xx, yy, width, height;
//    IMPoint p00, p01, p10, p11;
//    double* lowResFlowImage = reinterpret_cast<double*>(lowResFlow->image());
//    double* lowResFlowObjectImage = reinterpret_cast<double*>(lowResFlowObject->image());
//    double xRatio, yRatio, r1, r2, xRatioB, yRatioB, scaleX, scaleY;
//    
//    width = m_imageSeqence[0]->pixelsWide();
//    height = m_imageSeqence[0]->pixelsHigh();
//    
//    halfCellWidth = cellSize.width / 2;
//    halfCellHeight = cellSize.height / 2;
//    
//    float u00, u01, u10, u11, v00, v01, v10, v11, u, v, uo00, uo01, uo10, uo11, vo00, vo01, vo10, vo11;
//    double Q11, Q12, Q21, Q22, val;
//    int x1, x2, y1, y2;
//    MAVector3 curImagePoint;
//
//    maxX = lowResFlow->pixelsWide();
//    maxY = lowResFlow->pixelsHigh();
//    
//    scaleX = (double)(maxX - 1) / width;
//    scaleY = (double)(maxY - 1) / height;
//    
//    double* flowImage = reinterpret_cast<double*>(flow->image());
//    // сохраняем результат
//    for(y = 0; y < height; y++)
//    {
//        for(x = 0; x < width; x++)
//        {
//            curImagePoint.x = x * scaleX;
//            curImagePoint.y = y * scaleY;
//            
//            // производим билинейную интерполяцию
//            x1 = floor(curImagePoint.x);
//            x2 = ceil(curImagePoint.x);
//            y1 = floor(curImagePoint.y);
//            y2 = ceil(curImagePoint.y);
//            
//            if(x2 == x1)
//            {
//                x2 = x1 + 1.0;
//            }
//            if(x2 >= maxX)
//            {
//                x2 = x1;
//            }
//            if(y2 == y1)
//            {
//                y2 = y1 + 1.0;
//            }
//            if(y2 >= maxY)
//            {
//                y2 = y1;
//            }
//         
//
//            
//            index = x + y * width;
//            
//            // фон
//            if(flowImage[index*2 + 0] == 0)
//            {
//                Q11 = *(lowResFlowImage + (y1 * maxX + x1)*2 + 0);
//                Q12 = *(lowResFlowImage + (y2 * maxX + x1)*2 + 0);
//                Q21 = *(lowResFlowImage + (y1 * maxX + x2)*2 + 0);
//                Q22 = *(lowResFlowImage + (y2 * maxX + x2)*2 + 0);
//                
//                val = Q11*(x2 - curImagePoint.x)*(y2 - curImagePoint.y) + Q21*(curImagePoint.x - x1)*(y2 - curImagePoint.y) + Q12*(x2 - curImagePoint.x)*(curImagePoint.y - y1) + Q22*(curImagePoint.x - x1)*(curImagePoint.y - y1);
//                
//                flowImage[index*2 + 0] = val;
//                
//                Q11 = *(lowResFlowImage + (y1 * maxX + x1)*2 + 1);
//                Q12 = *(lowResFlowImage + (y2 * maxX + x1)*2 + 1);
//                Q21 = *(lowResFlowImage + (y1 * maxX + x2)*2 + 1);
//                Q22 = *(lowResFlowImage + (y2 * maxX + x2)*2 + 1);
//                
//                val = Q11*(x2 - curImagePoint.x)*(y2 - curImagePoint.y) + Q21*(curImagePoint.x - x1)*(y2 - curImagePoint.y) + Q12*(x2 - curImagePoint.x)*(curImagePoint.y - y1) + Q22*(curImagePoint.x - x1)*(curImagePoint.y - y1);
//                
//                flowImage[index*2 + 1] = val;
//            }
//            // объект
//            else
//            {
//                Q11 = *(lowResFlowObjectImage + (y1 * maxX + x1)*2 + 0);
//                Q12 = *(lowResFlowObjectImage + (y2 * maxX + x1)*2 + 0);
//                Q21 = *(lowResFlowObjectImage + (y1 * maxX + x2)*2 + 0);
//                Q22 = *(lowResFlowObjectImage + (y2 * maxX + x2)*2 + 0);
//                
//                val = Q11*(x2 - curImagePoint.x)*(y2 - curImagePoint.y) + Q21*(curImagePoint.x - x1)*(y2 - curImagePoint.y) + Q12*(x2 - curImagePoint.x)*(curImagePoint.y - y1) + Q22*(curImagePoint.x - x1)*(curImagePoint.y - y1);
//                
//                flowImage[index*2 + 0] = val;
//                
//                Q11 = *(lowResFlowObjectImage + (y1 * maxX + x1)*2 + 1);
//                Q12 = *(lowResFlowObjectImage + (y2 * maxX + x1)*2 + 1);
//                Q21 = *(lowResFlowObjectImage + (y1 * maxX + x2)*2 + 1);
//                Q22 = *(lowResFlowObjectImage + (y2 * maxX + x2)*2 + 1);
//                
//                val = Q11*(x2 - curImagePoint.x)*(y2 - curImagePoint.y) + Q21*(curImagePoint.x - x1)*(y2 - curImagePoint.y) + Q12*(x2 - curImagePoint.x)*(curImagePoint.y - y1) + Q22*(curImagePoint.x - x1)*(curImagePoint.y - y1);
//                
//                flowImage[index*2 + 1] = val;
//            }
//        }
//    }
//    
//    auto t2 = Clock::now();
//    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
//    std::cout << time_span.count() << "\n";
//    
//    // сохраняем результат в 32-бит
//    saveFlowTo32bit(flow, BASE_PATH.c_str(), "flow");
//    saveFlowTo32bit(lowResFlow, BASE_PATH.c_str(), "lowResFlow");
//    
//    m_flowSequence.clear();
//    m_flowSequence.push_back(flow );
//    
//    // освобождаем память
//    fftw_free(baseImageDataFFT);
}

// строит обратную проекцию оптического потока
void IMMotionAnalyser::warpImages()
{
    cout << "IMMotionAnalyser::wrapImages()" << endl;
    
    IMImageFile *warpImage, *image0, *image1;
    int x, y, warpX, warpY, dx, dy, index, flowIndex, warpIndex, width, height;
    unsigned char *data0, *data1, *dataWarp;
    double *flow;
    MAVector3 curImagePoint;
    double Q11, Q12, Q21, Q22, val;
    int x1, x2, y1, y2, maxX, maxY, spp;
    
    
    image0 = m_imageSeqence[0];
    image1 = m_imageSeqence[1];
    
    spp = m_flowSequence[0]->samplesPerPixel();
    
    width = image0->pixelsWide();
    height = image0->pixelsHigh();
    data0 = image0->image();
    data1 = image1->image();
    
    flow = reinterpret_cast<double*>(m_flowSequence[0]->image());
    
    warpImage = image0;
    warpImage->allocImage();
    dataWarp = warpImage->image();
    
    maxX = width;
    maxY = height;
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            index = x + y * width;
            flowIndex = spp*index;
            
            curImagePoint.x = x + flow[flowIndex + 0];
            curImagePoint.y = y + flow[flowIndex + 1];

            
            if(curImagePoint.x > 0 && curImagePoint.x < width && curImagePoint.y > 0 && curImagePoint.y < height)
            {
                // производим билинейную интерполяцию
                x1 = floor(curImagePoint.x);
                x2 = ceil(curImagePoint.x);
                y1 = floor(curImagePoint.y);
                y2 = ceil(curImagePoint.y);
                
                if(x2 == x1)
                {
                    x2 = x1 + 1.0;
                }
                if(x2 >= maxX)
                {
                    x2 = x1;
                }
                if(y2 == y1)
                {
                    y2 = y1 + 1.0;
                }
                if(y2 >= maxY)
                {
                    y2 = y1;
                }
                
                Q11 = *(data1 + (y1 * maxX + x1));
                Q12 = *(data1 + (y2 * maxX + x1));
                Q21 = *(data1 + (y1 * maxX + x2));
                Q22 = *(data1 + (y2 * maxX + x2));
                
                val = Q11*(x2 - curImagePoint.x)*(y2 - curImagePoint.y) + Q21*(curImagePoint.x - x1)*(y2 - curImagePoint.y) + Q12*(x2 - curImagePoint.x)*(curImagePoint.y - y1) + Q22*(curImagePoint.x - x1)*(curImagePoint.y - y1);
                
                warpIndex = warpX + warpY * width;
                
                /*if(flow[flowIndex + 2])
                {
                    val = 255;
                }*/

                
                dataWarp[index] = val;
            }
            else
            {
                dataWarp[index] = 0;
            }
        }
    }
    
    m_warpSequence.push_back(warpImage);
}

// вычисляет конечную ошибку потока
double IMMotionAnalyser::endpointError()
{
    double endpointError = 0;
    
    int width, height, x, y, index, indexGT, size, spp, sppGT, valid, dropCount;
    double u, v, ugt, vgt, du, dv;
    double *flowData, *gtFlowData;
    IMImageFile *flow, *gtFlow;
    
    dropCount = 0;
    
    flow = m_flowSequence[0];
    gtFlow = m_groundTruthFlowSequence[0];
    
    width = flow->pixelsWide();
    height = flow->pixelsHigh();
    size = width * height;
    spp = flow->samplesPerPixel();
    sppGT = gtFlow->samplesPerPixel();
    
    flowData = reinterpret_cast<double*>(flow->image());
    gtFlowData = reinterpret_cast<double*>(gtFlow->image());
    
    // цикл по всем пикселам
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            index = (x + y * width)*spp;
            indexGT = (x + y * width)*sppGT;
            
            valid = gtFlowData[indexGT + 2];
            
            // если есть истинный поток на данный пиксел
            if(valid > 0)
            {
                u = flowData[index + 0];
                v = flowData[index + 1];
                
                ugt = gtFlowData[indexGT + 0];
                vgt = gtFlowData[indexGT + 1];
                
                du = u - ugt;
                dv = v - vgt;
                
                endpointError += sqrt(du*du + dv*dv);
                dropCount++;
            }
        }
    }
    
    if(dropCount > 0)
    {
        endpointError /= dropCount;
    }
    
    return endpointError;
}