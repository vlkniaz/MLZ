#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>
#include <IMLib/MAMatrix.h>
#include <IMLib/PHCamera.h>
using namespace std;

class PHOrientation
{
private:
	MAMatrix L;
	PHCamera m_camera;
	std::vector<MAVector3> m_imagePoints;
	std::vector<MAVector3> m_objectPoints;

public:
	PHOrientation() :L(11, 1)
	{
	}

	PHCamera camera()
	{
		return m_camera;
	}
    
    void setCamera(PHCamera camera)
    {
        m_camera = camera;
    }

	// загрузка координат XYZ опорных точек
	void loadVertices();
	double makeDLT(double *x, double *y, double *X, double *Y, double *Z); // оценка внешнего ориентирования методом DLT
	double makeDLT(); // оценка внешнего ориентирования методом DLT
	double makeReducedDLT(double theta, double gamma); // оценка DLT по 2ум точкам
	double makeMLZ(double omega, double kappa); // оценка DLT по 2ум точкам

	double makeSolvePnP(); // оценка внешнего ориентирования методом DLT
    
    std::vector<MAVector3> objectPoints()
    {
        return m_objectPoints;
    }
    
    std::vector<MAVector3> imagePoints()
    {
        return m_imagePoints;
    }
    
    void setObjectPoints(std::vector<MAVector3> objectPoints)
    {
        m_objectPoints = objectPoints;
    }
    
    void setImagePoints(std::vector<MAVector3> imagePoints)
    {
        m_imagePoints = imagePoints;
    }

	void estimateParameters();
	int vertex[50];
	double allPointsX[50];
	double allPointsY[50];
	double allPointsZ[50];
	double X[6];
	double Y[6];
	double Z[6];
	double m_alpha;

	double x[6];
	double y[6];
	int Waypoints_[20];
	double WaypointsX[20];
	double WaypointsY[20];
	double WaypointsZ[20];
};