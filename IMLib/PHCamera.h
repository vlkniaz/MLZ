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
    // центр проекции в системе коордиант сцены
    MAVector3 m_X0;
    // центр снимка
    MAVector3 m_b;
    
    // фоусное расстояние
    double m_f;
    // номинальное фокусное расстояние
    double m_fnom;
    // курс
    double m_alpha;
    // тангаж	
    double m_omega;
    // крен
    double m_kappa;
    // масштаб снимка X
    double m_mx;
    // масштаб снимка Y
    double m_my;
    // параметры дисторсии
    double m_a;
    // параметры дисторсии
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
    
    void updateR()
    {
        MAMatrix Rk(3,3), Ra(3,3), Ro(3,3), Rkoa(3,3);
        double ca, sa, ck, sk, co, so;
        
        ca = cos(m_alpha);
        sa = sin(m_alpha);
        co = cos(m_omega);
        so = sin(m_omega);
        ck = cos(m_kappa);
        sk = sin(m_kappa);
        
        // строим матрицы поворотов
        Rk.set(0, 0, ck);
        Rk.set(0, 1, -sk);
        Rk.set(0, 2, 0.0);
        Rk.set(1, 0, sk);
        Rk.set(1, 1, ck);
        Rk.set(1, 2, 0.0);
        Rk.set(2, 0, 0.0);
        Rk.set(2, 1, 0.0);
        Rk.set(2, 2, 1.0);
        
        Ro.set(0, 0, 1.0);
        Ro.set(0, 1, 0.0);
        Ro.set(0, 2, 0.0);
        Ro.set(1, 0, 0.0);
        Ro.set(1, 1, co);
        Ro.set(1, 2, -so);
        Ro.set(2, 0, 0.0);
        Ro.set(2, 1, so);
        Ro.set(2, 2, co);
        
        Ra.set(0, 0, ca);
        Ra.set(0, 1, 0.0);
        Ra.set(0, 2, sa);
        Ra.set(1, 0, 0.0);
        Ra.set(1, 1, 1.0);
        Ra.set(1, 2, 0.0);
        Ra.set(2, 0, -sa);
        Ra.set(2, 1, 0.0);
        Ra.set(2, 2, ca);
        
        Rkoa = Ro * Ra;
        R = Rk * Rkoa;
        
        R.set(0, 0, ca*ck - sa*so*sk);
        R.set(0, 1, co*sk);
        R.set(0, 2, sa*ck + ca*so*sk);
        
        R.set(1, 0, -ca*sk - sa*so*ck);
        R.set(1, 1, co*ck);
        R.set(1, 2, -sa*sk + ca*so*ck);
        
        R.set(2, 0, -sa*co);
        R.set(2, 1, -so);
        R.set(2, 2, ca*co);
    }
    
    void setAlpha(double alpha)
    {
        m_alpha = alpha;
        updateR();
    }
    
    void setOmega(double omega)
    {
        m_omega = omega;
        updateR();
    }
    
    void setKappa(double kappa)
    {
        m_kappa = kappa;
        updateR();
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
    
    // находит трёхмерные координаты для точки на плоскости матрицы
    MAVector3 unproject(MAVector3 point, double distance);
};
