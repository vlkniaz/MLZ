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
    
public:
    // сдвиг от кадра I0 к кадру I1
    MAVector3 shiftFromI0ToI1(IMImageFile *I0, IMImageFile*I1);
    
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
};