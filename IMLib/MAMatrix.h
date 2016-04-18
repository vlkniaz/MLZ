//
//  MAMatrix.h
//  Библиотека MA
//
//  класс MAMatrix математические операции с матрицами
//
//  Created by Vladimir Knyaz on 24.07.14.
//  Copyright (c) 2014 Vladimir Knyaz. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>
using namespace std;

// класс MAVector3
class MAVector3
{
public:
    double x;
    double y;
    double z;
    
public:
    MAVector3():
    x(0.0),
    y(0.0),
    z(0.0)
    {
        
    }

    MAVector3(MAVector3 const &vec):
    x(vec.x),
    y(vec.y),
    z(vec.z)
    {
        
    }
    
    // векторное произведение
    MAVector3 crossProduct(MAVector3 vec) const
    {
        MAVector3 result;
        
        result.x = y * vec.z - z * vec.y;
        result.y = z * vec.x - x * vec.z;
        result.z = x * vec.y - y * vec.x;
        
        return result;
    }
    
    // скалярное произведение
    double dotProduct(MAVector3 vec) const
    {
        return x * vec.x + y * vec.y + z * vec.z;
    }
    
    // модуль вектора
    double modulus() const
    {
        return sqrt(x*x + y*y + z*z);
    }
    
    // нормирует данный вектор
    void normalize()
    {
        double k = modulus();
        
        x /= k;
        y /= k;
        z /= k;
    }
    
    // умножение на число
    MAVector3 operator* (const double &d) const
    {
        MAVector3 result;
        
        result.x = x * d;
        result.y = y * d;
        result.z = z * d;
        
        return result;
    }
    
    // умножение на число
    MAVector3& operator*= (const double &d)
    {
        x *= d;
        y *= d;
        z *= d;
        
        return *this;
    }
    
    // деление на число
    MAVector3 operator/ (const double &d) const
    {
        MAVector3 result;
        
        result.x = x / d;
        result.y = y / d;
        result.z = z / d;
        
        return result;
    }
    
    // деление на число
    MAVector3& operator/= (const double &d)
    {
        x /= d;
        y /= d;
        z /= d;
        
        return *this;
    }
    
    
    // сложение
    MAVector3 operator+ (const MAVector3 &vec) const
    {
        MAVector3 result;
        
        result.x = x + vec.x;
        result.y = y + vec.y;
        result.z = z + vec.z;
        
        return result;
    }
    
    // вычитание
    MAVector3 operator- (const MAVector3 &vec) const
    {
        MAVector3 result;
        
        result.x = x - vec.x;
        result.y = y - vec.y;
        result.z = z - vec.z;
        
        return result;
    }

    
    // сложение
    MAVector3& operator+= (const MAVector3 &vec)
    {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        
        return *this;
    }
    
    // сложение
    MAVector3& operator-= (const MAVector3 &vec)
    {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        
        return *this;
    }
    
    // оператор вывода векотра в поток
    friend ostream& operator<< (ostream &out, MAVector3 &vec)
    {
        out << "|" << vec.x << "\t" << vec.y << "\t" << vec.z << "|";
        
        return out;
    }
    
    // расстояние между векотрами
    double distanceTo(MAVector3 vec) const
    {
        MAVector3 delta;
        
        delta = *this - vec;
        
        return delta.modulus();
    }
};

// класс MAMatrix3 математические операции с матрицами 3 на 3
class MAMatrix3
{
private:
    // элементы матрицы
    double m_matrix[3][3];
    
public:
    MAMatrix3()
    {
        memset(m_matrix, 0, sizeof(double[3][3]));
    }
    
    MAMatrix3(MAMatrix3 const &matrix)
    {
        memcpy(m_matrix, matrix.m_matrix, sizeof(double[3][3]));
    }
    
    // установка значений элементов матрицы
    void set(unsigned int row, unsigned int column, double value)
    {
        m_matrix[row][column] = value;
    }
    
    // получение значений элементов матрицы
    double get(unsigned int row, unsigned column) const
    {
        return m_matrix[row][column];
    }
    
    // умножение матрицы на вектор
    MAVector3 operator* (MAVector3 vec)
    {
        MAVector3 result;
        
        result.x = m_matrix[0][0] * vec.x + m_matrix[0][1] * vec.y + m_matrix[0][2] * vec.z;
        result.y = m_matrix[1][0] * vec.x + m_matrix[1][1] * vec.y + m_matrix[1][2] * vec.z;
        result.z = m_matrix[2][0] * vec.x + m_matrix[2][1] * vec.y + m_matrix[2][2] * vec.z;
        
        return result;
    }

	// умножение матрицы на матрицу
	MAMatrix3 operator* (MAMatrix3 &mat)
	{
		MAMatrix3 result;

		int i, j, k;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				double value = 0.0;
				for (k = 0; k < 3; k++)
				{
					//cout << "mat.get(" << i << "," << j << ") =" << get(i, j) << endl;
					value += get(i, k) * mat.get(k, j);
					result.set(i, j, value);
				}
			}
		}
		return result;
	}
    
    // оператор вывода матрицы в поток
    friend ostream& operator<< (ostream &out, MAMatrix3 &matrix)
    {
        out << "|" << matrix.m_matrix[0][0] << "\t" << matrix.m_matrix[0][1] << "\t" << matrix.m_matrix[0][2] << "|" << endl
        << "|" << matrix.m_matrix[1][0] << "\t" << matrix.m_matrix[1][1] << "\t" << matrix.m_matrix[1][2] << "|" << endl
        << "|" << matrix.m_matrix[2][0] << "\t" << matrix.m_matrix[2][1] << "\t" << matrix.m_matrix[2][2] << "|";
        
        return out;
    }
};

// класс MAMatrix математические операции с матрицами произвольного размера
class MAMatrix
{
private:
	// элементы матрицы
	double *m_matrix;

	// число строк матрицы
	int m_rowsHigh;

	// число столбцов матрицы
	int m_columnsWide;

public:
	// конструктор матрицы произвольного размера
	
	MAMatrix(int rowsHigh, int columnsWide)
	{
		m_rowsHigh = rowsHigh;
		m_columnsWide = columnsWide;

		m_matrix = new double[rowsHigh * columnsWide];

		memset(m_matrix, 0, sizeof(double) * rowsHigh * columnsWide);
	}

	MAMatrix(MAMatrix const &matrix):
		m_rowsHigh(matrix.m_rowsHigh),
		m_columnsWide(matrix.m_columnsWide)
	{
		m_matrix = new double[m_rowsHigh * m_columnsWide];
		memcpy(m_matrix, matrix.m_matrix, sizeof(double) * m_rowsHigh * m_columnsWide);
	}

	// установка значений элементов матрицы
	void set(unsigned int row, unsigned int column, double value)
	{
		m_matrix[row * m_columnsWide + column] = value;
	}

	// получение значений элементов матрицы
	double get(unsigned int row, unsigned column) const
	{
		return m_matrix[row * m_columnsWide + column];
	}

	double* matrix()
	{
		return m_matrix;
	}

	int rowsHigh()
	{
		return m_rowsHigh;
	}

	int columnsWide()
	{
		return m_columnsWide;
	}

	// умножение матрицы на матрицу
	MAMatrix operator* (MAMatrix &mat)
	{
		if (m_columnsWide != mat.m_rowsHigh)
		{
			return MAMatrix(0, 0);
		}

		MAMatrix result(m_rowsHigh, mat.columnsWide());

		int i, j, k;
		
		for (i = 0; i < result.rowsHigh(); i++)
		{
			for (j = 0; j < mat.columnsWide(); j++)
			{
				double value = 0.0;
				for (k = 0; k < mat.rowsHigh(); k++)
				{
					//cout << "mat.get(" << i << "," << j << ") =" << get(i, j) << endl;
					value += get(i, k) * mat.get(k, j);
						result.set(i, j, value);
				}
			}
		}
		return result;
	}

	// умножение матрицы на число
	MAMatrix& operator*= (double value)
	{
		MAMatrix result(m_rowsHigh , m_columnsWide);

		int i, j;

		for (i = 0; i < m_rowsHigh; i++)
		{
			for (j = 0; j < m_columnsWide; j++)
			{	
				double val = get(i, j);
				set(i, j, val * value );
			}
		}
		return *this;
	}
	// перегрузка оператора умножение
	MAMatrix operator* (double value)
	{
		MAMatrix result(m_rowsHigh, m_columnsWide);

		int i, j;

		for (i = 0; i < m_rowsHigh; i++)
		{
			for (j = 0; j < m_columnsWide; j++)
			{
				double val = get(i, j);
				result.set(i, j, val * value);
			}
		}
		return result;
	}

	// деление на число
	MAMatrix& operator/= (double value)
	{
		MAMatrix result(m_rowsHigh, m_columnsWide);

		int i, j;

		for (i = 0; i < m_rowsHigh; i++)
		{
			for (j = 0; j < m_columnsWide; j++)
			{
				double val = get(i, j);
				set(i, j, val / value);
			}
		}
		return *this;
	}
	// перегрузка оператора деление
	MAMatrix& operator/ (double value)
	{
		MAMatrix result(m_rowsHigh, m_columnsWide);

		int i, j;

		for (i = 0; i < m_rowsHigh; i++)
		{
			for (j = 0; j < m_columnsWide; j++)
			{
				double val = get(i, j);
				set(i, j, val / value);
			}
		}
		return *this;
	}

	// сложение матриц
	MAMatrix operator+ (MAMatrix &mat)
	{
		if ((m_columnsWide != mat.columnsWide()) || (m_rowsHigh != mat.rowsHigh()))
		{
			return MAMatrix(0, 0);
		}

		MAMatrix result(m_rowsHigh, m_columnsWide);

		int i, j;

		for (i = 0; i < mat.rowsHigh(); i++)
		{
			double value = 0.0;
			for (j = 0; j < mat.columnsWide(); j++)
			{
					value = get(i, j) + mat.get(i, j);
					result.set(i, j, value);
			}
		}
		return result;
	}
	
	// вычитание матриц
	MAMatrix operator- (MAMatrix &mat)
	{
		if ((m_columnsWide != mat.columnsWide()) || (m_rowsHigh != mat.rowsHigh()))
		{
			return MAMatrix(0, 0);
		}

		MAMatrix result(m_rowsHigh, m_columnsWide);

		int i, j;

		for (i = 0; i < mat.rowsHigh(); i++)
		{
			double value = 0.0;
			for (j = 0; j < mat.columnsWide(); j++)
			{
				value = get(i, j) - mat.get(i, j);
				result.set(i, j, value);
			}
		}
		return result;
	}

	// оператор присваивания
	MAMatrix& operator= (MAMatrix matrix)
	{
		if ((m_columnsWide != matrix.columnsWide()) || (m_rowsHigh != matrix.rowsHigh()))
		{
			if (m_matrix != 0)
			{
				delete m_matrix;
			}
			m_rowsHigh = matrix.m_rowsHigh;
			m_columnsWide = matrix.m_columnsWide;
			m_matrix = new double[m_rowsHigh * m_columnsWide];
		}
		memcpy(m_matrix, matrix.m_matrix, sizeof(double)* m_rowsHigh * m_columnsWide);

		return *this;
	}


	// траспонированная матрица
	MAMatrix transposed ()
	{
		MAMatrix result(m_columnsWide, m_rowsHigh);

		int i, j;

		for (i = 0; i < result.rowsHigh(); i++)
		{
			for (j = 0; j < result.columnsWide(); j++)
			{
				result.set(i, j, get(j, i));
			}
		}

		return result;
	}

	// вывод матрицы в формате Matlab
	string toMatlab()
	{
		stringstream stream;
		string result;

		stream.clear();
		stream << "[";
		int i, j;

		for (i = 0; i < m_rowsHigh; i++)
		{
			for (j = 0; j < m_columnsWide; j++)
			{
				stream << get(i, j);
				if (j != m_columnsWide - 1)
				{
					stream << ", ";
				}
			}
			if (i != m_rowsHigh - 1)
			{
				stream << "; ";
			}
			
		}

		stream << "]";

		result = stream.str();
		return result;
	}

	MAMatrix Identity()
	{
		MAMatrix result(m_columnsWide, m_rowsHigh);

		int i, j;

		for (i = 0; i < result.rowsHigh(); i++)
		{
			for (j = 0; j < result.columnsWide(); j++)
			{
				if (i == j)
				{
					result.set(i, j, 1);
				}
				else
				{
					result.set(i, j, 0);
				}
			}
		}

		return result;
	}

	void gaussj(MAMatrix &a, MAMatrix &mat)
	{

		if (a.m_columnsWide != mat.m_rowsHigh)
		{
			cout << "Unable to calculate! Wrong wide and high!";
			system("pause");
			exit(1);
		}

		int i, icol, irow, j, k, l, ll, n = a.m_rowsHigh, m = mat.m_columnsWide;
		
		double big, dum, pivinv;
		vector<int> indxc, indxr, ipiv;
		indxc.resize(n);
		indxr.resize(n);
		ipiv.resize(n);
		for (j = 0; j < n; j++) ipiv[j] = 0;
		for (i = 0; i < n; i++) {
			big = 0.0;
			for (j = 0; j < n; j++)
			if (ipiv[j] != 1)
			for (k = 0; k < n; k++) {
				if (ipiv[k] == 0) {
					if (fabs(a.m_matrix[j * a.m_columnsWide + k]) >= big) {
						big = fabs(a.m_matrix[j * a.m_columnsWide + k]);
						irow = j;
						icol = k;
					}
				}
			}
			++(ipiv[icol]);

			if (irow != icol) {
				for (l = 0; l < n; l++) swap(a.m_matrix[irow * a.m_columnsWide + l], a.m_matrix[icol * a.m_columnsWide + l]);
				for (l = 0; l < m; l++) swap(mat.m_matrix[icol * mat.m_columnsWide + l], mat.m_matrix[icol * mat.m_columnsWide + l]);
			}

			indxr[i] = irow;
			indxc[i] = icol;
			if (a.m_matrix[icol * a.m_columnsWide + icol] == 0.0) printf("gaussj: Singular Matrix\n");
			pivinv = 1.0 / a.m_matrix[icol * a.m_columnsWide + icol];
			a.m_matrix[icol * a.m_columnsWide + icol] = 1.0;
			for (l = 0; l < n; l++) a.m_matrix[icol * a.m_columnsWide + l] *= pivinv;
			for (l = 0; l < m; l++) mat.m_matrix[icol * mat.m_columnsWide + l] *= pivinv;
			for (ll = 0; ll < n; ll++)
			if (ll != icol) {
				dum = a.m_matrix[ll * a.m_columnsWide + icol];
				a.m_matrix[ll * a.m_columnsWide + icol] = 0.0;
				for (l = 0; l < n; l++) a.m_matrix[ll * a.m_columnsWide + l] -= a.m_matrix[icol * a.m_columnsWide + l] * dum;
				for (l = 0; l < m; l++) mat.m_matrix[ll * mat.m_columnsWide + l] -= mat.m_matrix[icol * mat.m_columnsWide + l] * dum;
			}
		}
		for (l = n - 1; l >= 0; l--) {
			if (indxr[l] != indxc[l])
			for (k = 0; k < n; k++)
				swap(a.m_matrix[k * a.m_columnsWide + indxr[l]], a.m_matrix[k * a.m_columnsWide + indxc[l]]);
		}
	}
	

	void gaussj(MAMatrix& a)
	{
		MAMatrix b(a.m_rowsHigh, a.m_columnsWide);
		b = b.Identity();
		gaussj(a,b);
	}

	void invert()
	{
		gaussj(*this);
	}
    
    // ввод матрицы в формате Matlab
    // пример: [1 0 0; 0 1 0; 0 0 1]
    void fromMatlab(std::string matrix)
    {
        stringstream stream(matrix);
        
        int i, j;
        double value;
        char temp;
        
        // [ открывающая скобка
        stream >> temp;
        
        for (i = 0; i < m_rowsHigh; i++)
        {
            for (j = 0; j < m_columnsWide; j++)
            {
                stream >> value;
                set(i, j, value);
            }
            stream >> temp;
        }
    }
    
    // оператор присваивания
    MAMatrix& operator= (const initializer_list<double> &data)
    {
        int i, j;
        auto iter = data.begin();
        
        for (i = 0; i < m_rowsHigh; i++)
        {
            for (j = 0; j < m_columnsWide; j++)
            {
                set(i, j, *iter);
                iter++;
            }
        }
        
        return *this;
    }
    
    // минор матрицы
    MAMatrix minor(int i, int j)
    {
        int rowsHigh = m_rowsHigh-1;
        int columnsWide= m_columnsWide-1;
        MAMatrix result(m_rowsHigh-1, m_columnsWide-1);
        
        int idst, jdst, isrc, jsrc;
        isrc = 0;
        jsrc = 0;
        for(idst = 0, isrc = 0; idst < rowsHigh; idst++, isrc++)
        {
            if(idst == i)
            {
                isrc++;
            }
            for(jdst = 0, jsrc = 0; jdst < columnsWide; jdst++, jsrc++)
            {
                if(jdst == j)
                {
                    jsrc++;
                }
                result.set(idst, jdst, get(isrc, jsrc));
            }
        }
        
        return result;
    }
    
    double determinant()
    {
        double result = 0;
        
        if(m_rowsHigh == 1)
        {
            result = get(0, 0);
        }
        else if(m_rowsHigh == 2)
        {
            result = get(0,0)*get(1,1) - get(1,0)*get(0,1);
        }
        else
        {
            int j;
            for(j = 0; j < m_columnsWide; j++)
            {
                MAMatrix m(minor(0, j));
                result += pow(-1.0, j) * get(0,j) * m.determinant();
            }
        }
        
        return result;
    }
    
    static MAMatrix rotationAboutX(double angle)
    {
        MAMatrix result(3,3);
        double co, so;
        co = cos(angle);
        so = sin(angle);
        
        result.set(0, 0, 1.0);
        result.set(0, 1, 0.0);
        result.set(0, 2, 0.0);
        result.set(1, 0, 0.0);
        result.set(1, 1, co);
        result.set(1, 2, -so);
        result.set(2, 0, 0.0);
        result.set(2, 1, so);
        result.set(2, 2, co);
        
        return result;
    }
    
    static MAMatrix rotationAboutY(double angle)
    {
        MAMatrix result(3,3);
        double ca, sa;
        ca = cos(angle);
        sa = sin(angle);
        
        result.set(0, 0, ca);
        result.set(0, 1, 0.0);
        result.set(0, 2, sa);
        result.set(1, 0, 0.0);
        result.set(1, 1, 1.0);
        result.set(1, 2, 0.0);
        result.set(2, 0, -sa);
        result.set(2, 1, 0.0);
        result.set(2, 2, ca);
        
        return result;
    }
    
    static MAMatrix rotationAboutZ(double angle)
    {
        MAMatrix result(3,3);
        double ck, sk;
        ck = cos(angle);
        sk = sin(angle);
        
        result.set(0, 0, ck);
        result.set(0, 1, -sk);
        result.set(0, 2, 0.0);
        result.set(1, 0, sk);
        result.set(1, 1, ck);
        result.set(1, 2, 0.0);
        result.set(2, 0, 0.0);
        result.set(2, 1, 0.0);
        result.set(2, 2, 1.0);
        
        return result;
    }


    // умножение матрицы на вектор
    MAVector3 operator* (MAVector3 vec)
    {
        MAVector3 result;
        
        result.x = get(0, 0) * vec.x + get(0, 1) * vec.y + get(0, 2) * vec.z;
        result.y = get(1, 0) * vec.x + get(1, 1) * vec.y + get(1, 2) * vec.z;
        result.z = get(2, 0) * vec.x + get(2, 1) * vec.y + get(2, 2) * vec.z;
        
        return result;
    }


	// умножение матрицы на вектор
	/*MAVector operator* (MAVector3 vec)
	{
		MAVector3 result;

		result.x = m_matrix[0][0] * vec.x + m_matrix[0][1] * vec.y + m_matrix[0][2] * vec.z;
		result.y = m_matrix[1][0] * vec.x + m_matrix[1][1] * vec.y + m_matrix[1][2] * vec.z;
		result.z = m_matrix[2][0] * vec.x + m_matrix[2][1] * vec.y + m_matrix[2][2] * vec.z;

		return result;
	}*/

	// оператор вывода матрицы в поток
	friend ostream& operator<< (ostream &out, MAMatrix &matrix)
	{
		int i, j;

		for (i = 0; i < matrix.m_rowsHigh; i++)
		{
			for (j = 0; j < matrix.m_columnsWide; j++)
			{
                double temp = matrix.get(i, j);
				out << "|" << temp << "\t";
			}
			out << "|" << endl;
		}

		return out;
	}

};