//
//  IMMotionAnalyser.h
//  Билиотека IM
//
//  Анализ видеопоследовательностей
//
//  Created by Vladimir Knyaz on 06.04.15.
//  Copyright (c) 2015 Vladimir Knyaz. All rights reserved.
//

#pragma once

#include <iostream>
#include <vector>
#include <IMLib/IMLib.h>

using namespace std;

// класс IMMotionAnalyser - анализ видеопоследовательностей
class IMMotionAnalyser
{
private:
    // последовательность изображений
    vector<IMImageFile*> m_imageSeqence;
    
    // последовательность найденых оптических потоков
    vector<IMImageFile*> m_flowSequence;
    
    // последовательность найденых оптических потоков
    vector<IMImageFile*> m_warpSequence;
    
    // последовательность найденых оптических потоков
    vector<IMImageFile*> m_groundTruthFlowSequence;
    
    // окно Хэмминга
    IMImageFile *m_hammingWindow;
    
    // размер выходного изображения
    IMSize m_outputImageSize;
    
    // папка для сохранения выходных изображений
    std::string m_outputImagePath;
    
    // формат названия выходных изображений
    std::string m_outputImageNameFormat;
    
public:
    // конструктор без парамтеров
    IMMotionAnalyser() :
    m_hammingWindow(0),
    m_outputImageSize(32, 32)
    {
        
    }
    
    // сдвиг от кадра I0 к кадру I1
    IMPoint shiftFromI0ToI1(IMImageFile *I0, IMImageFile*I1, double& correlation);
        
    // прослеживание объекта на последовательности изображений, начиная с кадра nFrame,
    // до тех пор пока коэффициент корреляции больше minCorrelation
    void trackObjectAtFrame(int nFrame, IMPoint point, IMSize searchWindowSize, double minCorrelation);

    
    // оценка оптического потока методом Хорна-Шунка
    void estimateHSFlow();
    
    // проверка функций FFT
    void fft();
    
    // проверка функций FFT
    void fft2();
    
    // проверка функций FFT
    void psf();
    
    // строит обратную проекцию оптического потока
    void warpImages();
    
    // вычисление оптического потока методом фазовой корреляции
    void phaseCorrelation(IMSize cellSize);
    
    // вычисляет конечную ошибку потока
    double endpointError();
    
    // функции доступа
    void setImageSequence(vector<IMImageFile*>& imageSequence)
    {
        m_imageSeqence = imageSequence;
    }
    vector<IMImageFile*> flowSequence()
    {
        return m_flowSequence;
    }
    void setFlowSequence(vector<IMImageFile*>& flowSequence)
    {
        m_flowSequence = flowSequence;
    }
    
    vector<IMImageFile*> warpSequence()
    {
        return m_warpSequence;
    }
    
    vector<IMImageFile*> groundTruthFlowSequence()
    {
        return m_groundTruthFlowSequence;
    }
    
    void setGroundTruthFlowSequence(vector<IMImageFile*>& groundTruthFlowSequence)
    {
        m_groundTruthFlowSequence = groundTruthFlowSequence;
    }
    
    void setOutputImageSize(IMSize outputImageSize)
    {
        m_outputImageSize = outputImageSize;
    }
    
    void setOutputImagePath(std::string outputImagePath)
    {
        m_outputImagePath = outputImagePath;
    }
    
    void setOutputImageNameFormat(std::string outputImageNameFormat)
    {
        m_outputImageNameFormat = outputImageNameFormat;
    }
};