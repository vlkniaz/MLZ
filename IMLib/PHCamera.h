#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include "MAMatrix.h"

using namespace std;

class PHCamera
{
public:
	double c_x, c_y; // principal distance (different scales in x and y)
	double x0_, y0_; // coordinates of principal point
	double X0, Y0, Z0; // perspective centre
	MAMatrix perspective_centre;
	MAMatrix R; // rotation matrix
    MAVector3 m_X0;
    MAVector3 m_b;
    
    // фоусное расстояние
    double m_f;
    double m_fnom;
    double m_alpha;
    double m_omega;
    double m_kappa;
    double m_mx;
    double m_my;
    double m_a;
    double m_K1;
    double m_K2;
    double m_K3;
    double m_P1;
    double m_P2;

public:
	PHCamera() :R(3, 3), perspective_centre(3, 1)
	{

	}
    
    // конструктор чтение параметров камеры из файла
    PHCamera(std::string filename) : PHCamera()
    {
        loadParameters(filename);
    }
    
    
    // читает файл параметров камеры в формате CImgGeom
    bool loadParameters(std::string filename);
    
    // вычисляем неискажённые координаты по искажённым
    MAVector3 undistortedPoint(MAVector3 truePoint)
    {
        MAVector3 result, delta, x;
        double r2, r4, r6;
        
        x = truePoint - m_b;
        x.x *= m_mx;
        x.y *= m_my;
        r2 = x.modulus();
        r2 = r2*r2;
        r4 = r2*r2;
        r6 = r4*r2;
        
        delta.x = x.y*m_a + x.x*r2*m_K1 + x.x*r4*m_K2 + x.x*r6*m_K3 + (r2 + 2*x.x*x.x)*m_P1 + 2*x.x*x.y*m_P2;
        delta.y = x.x*m_a + x.y*r2*m_K1 + x.y*r4*m_K2 + x.y*r6*m_K3 + 2*x.x*x.y*m_P1 + (r2 + 2*x.y*x.y)*m_P2;
        
        result = x + delta;
        
        result.x /= m_mx;
        result.y /= m_my;
        
        result = result + m_b;
        
        return result;
    }
    
    // вычисляет искажённые координаты по искажённым
    MAVector3 distortedPoint(MAVector3 point)
    {
        MAVector3 result, delta, x, xa, xap, step;
        double r2, r4, r6;
        
        step.x = 1e10;
        step.y = 1e10;
        xa = point - m_b;
        xa.x *= m_mx;
        xa.y *= m_my;
        x = xa;
        
        int counter = 0;
        
        while(step.modulus() > m_mx*0.1 && counter < 1000)
        {
            r2 = xa.modulus();
            r2 = r2*r2;
            r4 = r2*r2;
            r6 = r4*r2;
            
            delta.x = xa.y*m_a + xa.x*r2*m_K1 + xa.x*r4*m_K2 + xa.x*r6*m_K3 + (r2 + 2*xa.x*xa.x)*m_P1 + 2*xa.x*xa.y*m_P2;
            delta.y = xa.x*m_a + xa.y*r2*m_K1 + xa.y*r4*m_K2 + xa.y*r6*m_K3 + 2*xa.x*xa.y*m_P1 + (r2 + 2*xa.y*xa.y)*m_P2;
            
            xap = xa;
            xa = x - delta;
            step = xap - xa;
            counter++;
        }
        
        
        xa.x /= m_mx;
        xa.y /= m_my;
        
        xa = xa + m_b;
        
        return xa;
    }
    
    // проецирует точку на плоскость матрицы
    MAVector3 project(MAVector3 point);
};