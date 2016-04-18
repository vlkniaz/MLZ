#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define _USE_MATH_DEFINES

#include <math.h>
#include <IMLib/MAMatrix.h>
#include <IMLib/PHOrientation.h>
#include <IMLib/PHCamera.h>

using namespace std;

double PHOrientation::makeDLT(double x[6], double y[6], double X[6], double Y[6], double Z[6])
{
	//ofstream outfile("matrix.txt");
	int n = 6; //Количество точек-ориентиров(>=6) 
	int u = 11; //Количество DLT параметров(11)
	MAMatrix A(2 * n, u); //design matrix A
	MAMatrix AT(u, n); //transposed matrix A
	MAMatrix P(2 * n, 2 * n); //weight matrix P
	P = P.Identity();
	
	MAMatrix N(u, u); //matrix of normal equations N
	MAMatrix _n(11, 1); //absolute term
	MAMatrix _l(2 * n, 1); //
	MAMatrix matrix(3, 3);
	int i, j;
	int isOdd = 0;
	// заполняем матрицы
	int elementNumber = 0;
	for (i = 0; i < A.rowsHigh(); i++)
	{
		for (j = 0; j < A.columnsWide(); j++)
		{
			isOdd = i % 2;
			if (isOdd)
			{
				A.set(i, 4, X[elementNumber]);
				A.set(i, 5, Y[elementNumber]);
				A.set(i, 6, Z[elementNumber]);
				A.set(i, 7, 1);
				A.set(i, 8, -y[elementNumber] * X[elementNumber]);
				A.set(i, 9, -y[elementNumber] * Y[elementNumber]);
				A.set(i, 10, -y[elementNumber] * Z[elementNumber]);
				_l.set(i, 0, y[elementNumber]);
			}
			else
			{
				A.set(i, 0, X[elementNumber]);
				A.set(i, 1, Y[elementNumber]);
				A.set(i, 2, Z[elementNumber]);
				A.set(i, 3, 1);
				A.set(i, 8, -x[elementNumber] * X[elementNumber]);
				A.set(i, 9, -x[elementNumber] * Y[elementNumber]);
				A.set(i, 10, -x[elementNumber] * Z[elementNumber]);
				_l.set(i, 0, x[elementNumber]);
			}
		}
		if (isOdd)
			elementNumber++;
	}
	//cout << "Matrix A:" << endl << A << endl;
	//outfile << "Matrix A:" << endl << A.toMatlab() << endl;

	//cout << "Matrix l:" << endl << _l << endl;
	//outfile << "Matrix l:" << endl << _l.toMatlab() << endl;

	// 2 - находим N = AT * P * A
	AT = A.transposed();
	N = P * A;
	N = AT * N;

	_n = P * _l;
	_n = AT * _n;

	//cout << "Matrix N:" << endl << N << endl;
	//outfile << "Matrix N:" << endl << N << endl;
	//outfile << "Matrix N:" << endl << N.toMatlab() << endl;

	// 3 - решаем систему линейных уравнений
	MAMatrix QQ(N);
	MAMatrix EE(11, 11);

	QQ.invert();
	//cout << "Matrix QQ:" << endl << QQ << endl;
	//outfile << "Matrix QQ:" << endl << QQ << endl;

	// проверка - единичная матрица
	EE = QQ * N;
	//cout << "Matrix EE:" << endl << EE << endl;
	//outfile << "Matrix EE:" << endl << EE << endl;
	//outfile << "Matrix EE:" << endl << EE.toMatlab() << endl;

	// один вариант - умножаем обратную матрицу на вектор наблюдений
	//_x = QQ * _n;

	// второй вариант - используем метод Гаусса-Жордана, чтобы сразу найти решение системы линейных уравнений - так точнее и быстрее
	N.gaussj(N, _n);
	L = _n;
	//cout << "matrix _x" << endl << L << endl;
	//outfile << "matrix _x : " << endl << L.toMatlab() << endl;
	// Проверка Perspective Centre[X0, Y0, Z0]
	// TEMP
	/*L.set(2, 0, 0.001132);
	L.set(6, 0, -0.005910);
	L.set(10, 0, -0.001003);*/
	MAMatrix Lin_matr(3, 3);
	MAMatrix Lin_matr_part(3, 1);
	MAMatrix Perspective_centre(3, 1);
	Lin_matr.set(0, 0, L.get(0, 0));
	Lin_matr.set(0, 1, L.get(1, 0));
	Lin_matr.set(0, 2, L.get(2, 0));
	Lin_matr_part.set(0, 0, L.get(3, 0));
	Lin_matr.set(1, 0, L.get(4, 0));
	Lin_matr.set(1, 1, L.get(5, 0));
	Lin_matr.set(1, 2, L.get(6, 0));
	Lin_matr_part.set(0, 1, L.get(7, 0));
	Lin_matr.set(2, 0, L.get(8, 0));
	Lin_matr.set(2, 1, L.get(9, 0));
	Lin_matr.set(2, 2, L.get(10, 0));
	Lin_matr_part.set(0, 2, 1);
	//cout << "Lin_matr" << endl << Lin_matr << endl;
	Lin_matr.invert();
	Lin_matr *= -1;
	Perspective_centre = Lin_matr * Lin_matr_part;

	//cout << "Lin_matr" << endl << Lin_matr << endl;
	//cout << "Lin_matr_part" << endl << Lin_matr_part << endl;
	//cout << "Perspective_centre" << endl << Perspective_centre << endl;



	// interior orientation
	double Lin;
	Lin = (-1.0) / sqrt(pow((L.get(8, 0)), 2) + pow((L.get(9, 0)), 2) + pow((L.get(10, 0)), 2)); // orientation parameters
	//cout << "Lin" << endl << Lin << endl << endl;
	// coordinates of principal point
	double x0_, y0_;
	x0_ = -pow(Lin, 2) * ((L.get(0, 0) * L.get(8, 0)) + (L.get(1, 0) * L.get(9, 0)) + (L.get(2, 0) * L.get(10, 0)));
	y0_ = -pow(Lin, 2) * ((L.get(4, 0) * L.get(8, 0)) + (L.get(5, 0) * L.get(9, 0)) + (L.get(6, 0) * L.get(10, 0)));
	// principal distance (different scales in x and y)
	double c_x, c_y;
	c_x = -sqrt(pow(Lin, 2) * (pow((L.get(0, 0)), 2) + pow((L.get(1, 0)), 2) + pow((L.get(2, 0)), 2)) - pow(x0_, 2));
	c_y = -sqrt(pow(Lin, 2) * (pow((L.get(4, 0)), 2) + pow((L.get(5, 0)), 2) + pow((L.get(6, 0)), 2)) - pow(y0_, 2));

	//cout << "c_x:" << endl << c_x << endl << endl;
	//cout << "c_y:" << endl << c_y << endl << endl;

	//cout << "x'0:" << endl << x0_ << endl << endl;
	//cout << "y'0:" << endl << y0_ << endl << endl;


	m_camera.R.set(0, 0, ((Lin*((x0_*L.get(8, 0)) - L.get(0, 0))) / c_x));
	m_camera.R.set(0, 1, ((Lin*((y0_*L.get(8, 0)) - L.get(4, 0))) / c_y));
	m_camera.R.set(0, 2, Lin * L.get(8, 0));
	m_camera.R.set(1, 0, ((Lin*((x0_*L.get(9, 0)) - L.get(1, 0))) / c_x));
	m_camera.R.set(1, 1, ((Lin*((y0_*L.get(9, 0)) - L.get(5, 0))) / c_y));
	m_camera.R.set(1, 2, Lin * L.get(9, 0));
	m_camera.R.set(2, 0, ((Lin*((x0_*L.get(10, 0)) - L.get(2, 0))) / c_x));
	m_camera.R.set(2, 1, ((Lin*((y0_*L.get(10, 0)) - L.get(6, 0))) / c_y));
	m_camera.R.set(2, 2, Lin * L.get(10, 0));

	m_camera.perspective_centre = Perspective_centre;
	
	//cout << "R - rotation matrix:" << endl << m_camera.R << endl;
	return 0;
}

double PHOrientation::makeReducedDLT(double theta, double gamma)
{
	double A, B, C, q1, q2, phi1, phi2;
	int i = 0;
	// v == y
	// u == x
	double u1, v1, u2, v2;
	double width = 640;
	double height = 360;
	// переход к системе координат матрицы
	u1 = (x[1] - width / 2.0)*5.0E-6;
	u2 = (x[2] - width / 2.0)*5.0E-6;
	v1 = ((height - y[1]) - height / 2.0)*5.0E-6;
	v2 = ((height - y[2]) - height / 2.0)*5.0E-6;


	double t1, t2;

	t1 = Z[1];
	Z[1] = Y[1];
	Y[1] = t1;
	t2 = Z[2];
	Z[2] = Y[2];
	Y[2] = t2;

	A = -v2 * X[2] + v2 * Z[2] * u1 + Y[1] * u2 + v1 * X[2] + v1 * Z[1] * u2 - v1 * X[1] - v1 * u2 * Z[2] + Y[2] * u1 - Y[1] * u1 + v2 * X[1] - Y[2] * u2 - v2 * u1 * Z[1];
	B = -2 * v1 * X[1] * u2 - 2 * v2 * Z[2] - 2 * v1 * Z[1] + 2 * v1 * Z[2] - 2 * v2 * X[2] * u1 + 2 * v1 * u2 * X[2] + 2 * v2 * Z[1] + 2 * v2 * u1 * X[1];
	C = Y[1] * u2 - Y[1] * u1 - v2 * Z[2] * u1 + Y[2] * u1 - v2 * X[1] + v2 * u1 * Z[1] - v1 * X[2] - Y[2] * u2 + v1 * X[1] + v2 * X[2] - v1 * Z[1] * u2 + v1 * u2 * Z[2];
	double test = sqrt(B*B - 4 * A*C);
	q1 = ((-B + sqrt(B*B - 4 * A*C)) / 2 * A);
	q2 = ((-B - sqrt(B*B - 4 * A*C)) / 2 * A);
	phi1 = 2 * atan(q1);
	phi2 = 2 * atan(q2);
	return 0;
}

// оценка ориентирования по двум точкам и известному направлению вектора тяжести omega - тангаж, kappa - крен (радианы)
double PHOrientation::makeMLZ(double omega, double kappa)
{
    //cout << "makeMLZ" << endl;
    
    MAVector3 x1, x2, xc1, xc2, xk1, xk2, x0, X1, X2, oy, oyb, n, n2, xo1, X0;
    MAMatrix Rb(3,3), Rk(3,3), Ro(3,3), Ra(3,3), Rkoa(3,3), Rok0(3,3), Rot(3,3), Rkt(3,3), RotT(3,3), Rok(3,3);
    double f, m, k, M, alpha, alphaDeg, sOmega, cOmega, sPhi, cPhi, dy, dxz, beta, cb, sb, phi, phiDeg;
    double sk, ck, so, co, sa, ca;
    
    // фокусное расстояние (миллиметры)
    f = m_camera.m_f;
    // размер пиксела (миллиметры)
    m = m_camera.m_mx;
    
    // координаты центра кадра (главная точка)
    x0.x = m_camera.m_b.x;
    x0.y = m_camera.m_b.y;
    
    // сортируем точки по координате X
    int j = 0, nMin = 0, nMax = 0;
    double xMin = 1e6, xMax = -1e6;
    auto imagePointIter = m_imagePoints.begin();
    for (imagePointIter = m_imagePoints.begin(); imagePointIter != m_imagePoints.end(); imagePointIter++)
    {
        if (imagePointIter->x < xMin)
        {
            xMin = imagePointIter->x;
            nMin = j;
        }
        if (imagePointIter->x > xMax)
        {
            xMax = imagePointIter->x;
            nMax = j;
        }
        j++;
    }
    
    // входные точки на изображении (пикселы)
    x1.x = m_imagePoints[nMin].x;
    x1.y = m_imagePoints[nMin].y;
    x2.x = m_imagePoints[nMax].x;
    x2.y = m_imagePoints[nMax].y;
    
    // исправляем дисторсию
    x1 = m_camera.undistortedPoint(x1);
    x2 = m_camera.undistortedPoint(x2);
    
    // входные точки в системе координат камеры (миллиметры)
    xc1.x = (x1.x - x0.x) * m;
    xc1.y = -(x1.y - x0.y) * m;
    xc1.z = -f;
    xc2.x = (x2.x - x0.x) * m;
    xc2.y = -(x2.y - x0.y) * m;
    xc2.z = -f;
    
    // входные точки в системе коордиант объекта (миллиметры)
    X1.x = m_objectPoints[nMin].x;
    X1.y = m_objectPoints[nMin].y;
    X1.z = m_objectPoints[nMin].z;
    X2.x = m_objectPoints[nMax].x;
    X2.y = m_objectPoints[nMax].y;
    X2.z = m_objectPoints[nMax].z;
    
    // Rok0
    Ro = MAMatrix::rotationAboutX(omega);
    Rk = MAMatrix::rotationAboutZ(kappa);
    
    Rok0 = Rk * Ro;
    
    // Rot
    double alphaT, kappaT, dX, dY, dZ;
    dX = X2.x - X1.x;
    dY = X2.y - X1.y;
    dZ = X2.z - X1.z;
    alphaT = atan2(-dZ, dX);
    kappaT = atan2(dY, dX);
    
    Ra = MAMatrix::rotationAboutY(alphaT);
    Rk = MAMatrix::rotationAboutZ(kappaT);
    
    Rot = Rk * Ra;
    
    //cout << "Rot" << endl;
    //cout << Rot << endl;
    
    // Rkt
    
    // get alpha'
    
    // ось OY
    oy.x = 0.0;
    oy.y = 1.0;
    oy.z = 0.0;
    
    oy.normalize();
    
    Rok = Rot.transposed() * Rok0;
    xk1 = Rok * xc1;
    xk2 = Rok * xc2;
    
    //cout << "Rok'" << endl;
    //cout << Rok << endl;
    
    n = xk2.crossProduct(xk1);
    n.normalize();
    if(n.z >= 0)
    {
        n2 = oy.crossProduct(n);
    }
    else
    {
        n2 = n.crossProduct(oy);
    }
    n2.normalize();
    
    alpha = atan2(-n2.z, n2.x);
    alphaDeg = alpha * 180.0 / M_PI;
    
    Rkt = MAMatrix::rotationAboutY(alpha);
    
    //cout << Rkt << endl;
    
    // Rok
    Rok = Rot * Rok0;
    Rok = Rkt * Rok;
    Rok = Rot.transposed() * Rok;
    
    //cout << Rok << endl;
    
    double alph, alphDeg;
    alph = -atan2(Rok.get(2,0), Rok.get(2,2));
    alphDeg = alpha * 180.0 / M_PI;
    
    // направляющие косинусы точек на изображени
    MAVector3 d1, d2, dif, XS1, XS2, XS;
    MAMatrix MA(2,2), MB(2,2), MC(2,2), NUM(3,3), DENUM(3,3);
    double lamda, mu, a, b, c, a1, b1, c1, a2, b2, c2, num, denum;
    d1 = Rok * xc1;
    d1.normalize();
    d2 = Rok * xc2;
    d2.normalize();
    
    a1 = d1.x;
    b1 = d1.y;
    c1 = d1.z;
    
    a2 = d2.x;
    b2 = d2.y;
    c2 = d2.z;
    
    MA.set(0, 0, a1);
    MA.set(0, 1, b1);
    MA.set(1, 0, a2);
    MA.set(1, 1, b2);
    a = MA.determinant();
    
    MB.set(0, 0, b1);
    MB.set(0, 1, c1);
    MB.set(1, 0, b2);
    MB.set(1, 1, c2);
    b = MB.determinant();
    
    MC.set(0, 0, c1);
    MC.set(0, 1, a1);
    MC.set(1, 0, c2);
    MC.set(1, 1, a2);
    c = MC.determinant();
    
    dif = X1 - X2;
    
    NUM.set(0, 0, dif.x);
    NUM.set(0, 1, dif.y);
    NUM.set(0, 2, dif.z);
    NUM.set(1, 0, a);
    NUM.set(1, 1, b);
    NUM.set(1, 2, c);
    NUM.set(2, 0, a2);
    NUM.set(2, 1, b2);
    NUM.set(2, 2, c2);
    
    DENUM.set(0, 0, a1);
    DENUM.set(0, 1, b1);
    DENUM.set(0, 2, c1);
    DENUM.set(1, 0, a2);
    DENUM.set(1, 1, b2);
    DENUM.set(1, 2, c2);
    DENUM.set(2, 0, a);
    DENUM.set(2, 1, b);
    DENUM.set(2, 2, c);
    
    num = NUM.determinant();
    denum = DENUM.determinant();
    lamda = num/denum;
    
    NUM.set(2, 0, a1);
    NUM.set(2, 1, b1);
    NUM.set(2, 2, c1);
    num = NUM.determinant();
    mu = num/denum;
    
    XS1.x = X1.x + lamda*a1;
    XS1.y = X1.y + lamda*b1;
    XS1.z = X1.z + lamda*c1;
    
    XS2.x = X2.x + mu*a2;
    XS2.y = X2.y + mu*b2;
    XS2.z = X2.z + mu*c2;
    
    XS = (XS1 + XS2) / 2.0;
    
    // вычисляем масштаб
    double scale, dimage, dobject;
    dimage = (xk2 - xk1).modulus();
    dobject = (X2 - X1).modulus();
    scale = dobject / dimage;

    // back projection
    xo1 = Rok * (xc1 * -1.0);
    xo1 *= scale;
    X0 = X1 + xo1;
    
    m_camera.m_X0 = XS;
    
    m_alpha = alpha;
    
    return 0.0;
}

// оценка ориентирования по двум точкам и известному направлению вектора тяжести omega - тангаж, kappa - крен (радианы)
/*double PHOrientation::makeMLZ(double omega, double kappa)
{
	//cout << "makeMLZ" << endl;

	MAVector3 x1, x2, xc1, xc2, x0, X1, X2, oy, oyb, n, n2, xo1, X0;
	MAMatrix3 Rb, Rk, Ro, Ra, Rkoa;
	double f, m, k, a, b, c, M, alpha, alphaDeg, sOmega, cOmega, sPhi, cPhi, dy, dxz, beta, cb, sb, phi, phiDeg;
	double sk, ck, so, co, sa, ca;

	// фокусное расстояние (миллиметры)
	f = m_camera.m_f;
	// размер пиксела (миллиметры)
	m = m_camera.m_mx;

	// координаты центра кадра (главная точка)
	x0.x = m_camera.m_b.x;
	x0.y = m_camera.m_b.y;

	// сортируем точки по координате X
	int j = 0, nMin = 0, nMax = 0;
	double xMin = 1e6, xMax = -1e6;
	auto imagePointIter = m_imagePoints.begin();
	for (imagePointIter = m_imagePoints.begin(); imagePointIter != m_imagePoints.end(); imagePointIter++)
	{
		if (imagePointIter->x < xMin)
		{
			xMin = imagePointIter->x;
			nMin = j;
		}
		if (imagePointIter->x > xMax)
		{
			xMax = imagePointIter->x;
			nMax = j;
		}
		j++;
	}

	// входные точки на изображении (пикселы)
	x1.x = m_imagePoints[nMin].x;
	x1.y = m_imagePoints[nMin].y;
	x2.x = m_imagePoints[nMax].x;
	x2.y = m_imagePoints[nMax].y;

	// исправляем дисторсию
	x1 = m_camera.undistortedPoint(x1);
	x2 = m_camera.undistortedPoint(x2);

	// входные точки в системе координат камеры (миллиметры)
	xc1.x = (x1.x - x0.x) * m;
	xc1.y = -(x1.y - x0.y) * m;
	xc1.z = -f;
	xc2.x = (x2.x - x0.x) * m;
	xc2.y = -(x2.y - x0.y) * m;
	xc2.z = -f;

	// входные точки в системе коордиант объекта (миллиметры)
	X1.x = m_objectPoints[nMin].y;
	X1.y = m_objectPoints[nMin].z;
	X1.z = m_objectPoints[nMin].x;
	X2.x = m_objectPoints[nMax].y;
	X2.y = m_objectPoints[nMax].z;
	X2.z = m_objectPoints[nMax].x;

	// ось OY
	oy.x = 0.0;
	oy.y = 1.0;
	oy.z = 0.0;

	oy.normalize();

	dy = X2.y - X1.y;
	dxz = sqrt((X2.x - X1.x)*(X2.x - X1.x) + (X2.z - X1.z)*(X2.z - X1.z));
	beta = atan2(dy, dxz) + kappa;
	cb = cos(beta);
	sb = sin(beta);
	Rb.set(0, 0, cb);
	Rb.set(0, 1, -sb);
	Rb.set(0, 2, 0.0);
	Rb.set(1, 0, sb);
	Rb.set(1, 1, cb);
	Rb.set(1, 2, 0.0);
	Rb.set(2, 0, 0.0);
	Rb.set(2, 1, 0.0);
	Rb.set(2, 2, 1.0);

	double om = 30 * M_PI / 180.0;
	co = cos(om);
	so = sin(om);

	Ro.set(0, 0, 1.0);
	Ro.set(0, 1, 0.0);
	Ro.set(0, 2, 0.0);
	Ro.set(1, 0, 0.0);
	Ro.set(1, 1, co);
	Ro.set(1, 2, -so);
	Ro.set(2, 0, 0.0);
	Ro.set(2, 1, so);
	Ro.set(2, 2, co);

//	system("cls");
//	cout << "Rb:" << endl << Rb << endl;
	
	//oyb = Ro * oy;
	oyb = Rb * oy;
	
//	cout << "oyb" << oyb << endl;

	xc1.normalize();
//	cout << "xc1" << xc1 << endl;
	xc2.normalize();
//	cout << "xc2" << xc2 << endl;

	n = xc2.crossProduct(xc1);

	if (n.z <= 0.0)
	{		
		n2 = n.crossProduct(oyb);
	}
	else
	{
		n2 = oyb.crossProduct(n);
	}
	

	phi = atan2(-(X2.z - X1.z), X2.x - X1.x);
	phiDeg = phi * 180.0 / M_PI;

	// угол поворота от системы координат камеры к системе координат объекта
	alpha = atan2(n2.z, n2.x) + phi;
	//temp
	alpha = 0.0;
	/*if (alpha > M_PI_2)
	{
		alpha -= M_PI;
	}
	if (alpha < -M_PI_2)
	{
		alpha += M_PI;
	}
	alphaDeg = alpha * 180.0 / M_PI;

	// вычисляем масштаб
	double scale, dimage, dobject;
	dimage = (xc2 - xc1).modulus();
	dobject = (X2 - X1).modulus();
	scale = dobject / dimage;

	ca = cos(alpha);
	sa = sin(alpha);
	co = cos(omega);
	so = sin(omega);
	ck = cos(kappa);
	sk = sin(kappa);

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
	Rkoa = Rk * Rkoa;

	//cout << "Rkoa" << Rkoa << endl;

	xo1 = Rkoa * (xc1 * -1.0);
	xo1 *= scale;
	X0 = X1 + xo1;
	X0 /= 1e3;

	m_camera.perspective_centre.set(0, 0, X0.z);
	m_camera.perspective_centre.set(1, 0, X0.x);
	m_camera.perspective_centre.set(2, 0, X0.y);
    
    m_camera.m_X0 = X0;

	m_alpha = alpha;

	return 0.0;
}*/

double PHOrientation::makeDLT()
{
	makeDLT(x, y, X, Y, Z);

	return 0;
}

void PHOrientation::estimateParameters()
{

}

// загрузка координат XYZ опорных точек
void PHOrientation::loadVertices()
{
	ifstream pointsCoordsFile("vertex_room7.xyz", std::ifstream::in);
	//pointsCoordsFile.open("vertex.txt");
	ifstream WaypointsFile("waypoints.xyz", std::ifstream::in);

	bool isGood = pointsCoordsFile.good();
	bool isGood_ = WaypointsFile.good();
	if (!isGood || !isGood_)
	{
		return;
	}

	int CoordIndex = 0;
	int pointNumber = 0;
	while (!pointsCoordsFile.eof() && CoordIndex < 50)
	{
		pointsCoordsFile >> vertex[CoordIndex];
		int pointNumber = vertex[CoordIndex];
		pointsCoordsFile >> allPointsX[pointNumber] >> allPointsY[pointNumber] >> allPointsZ[pointNumber];
		++CoordIndex;
	}
	pointsCoordsFile.close();
	CoordIndex = 0;
	int waypointNumber = 0;
	while (!WaypointsFile.eof() && CoordIndex < 15)
	{
		WaypointsFile >> Waypoints_[CoordIndex];
		waypointNumber = Waypoints_[CoordIndex];
		WaypointsFile >> WaypointsX[waypointNumber] >> WaypointsY[waypointNumber] >> WaypointsZ[waypointNumber];
		++CoordIndex;
	}
	WaypointsFile.close();
}
