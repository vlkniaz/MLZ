#include <IMLib/PHCamera.h>

using namespace std;

// проецирует точку на плоскость матрицы
MAVector3 PHCamera::project(MAVector3 point)
{
    MAVector3 result;
    
    double num, denum;
    
    num = R.get(0,0) * (point.x - m_X0.x) + R.get(0,1) * (point.y - m_X0.y) + R.get(0,2) * (point.z - m_X0.z);
    denum = R.get(2,0) * (point.x - m_X0.x) + R.get(2,1) * (point.y - m_X0.y) + R.get(2,2) * (point.z - m_X0.z);
    
    result.x = -m_f * (num / denum);
    result.x = result.x / m_mx + m_b.x;
    
    num = R.get(1,0) * (point.x - m_X0.x) + R.get(1,1) * (point.y - m_X0.y) + R.get(1,2) * (point.z - m_X0.z);
    
    result.y = -m_f * (num / denum);
    result.y = -result.y / m_my + m_b.y;
    
    return result;
}

// читает файл параметров камеры в формате CImgGeom
bool PHCamera::loadParameters(std::string filename)
{
    std::fstream parameters(filename);
    
    if(!parameters.good())
    {
        return false;
    }
    
    std::string temp;
    
    parameters >> temp;
    if(temp != "CImgGeom")
    {
        return false;
    }
    
    parameters >> temp;
    if(temp != "CProjCentr")
    {
        return false;
    }
    
    // фокусное расстояние
    parameters >> temp;
    if(temp != "f")
    {
        return false;
    }
    parameters >> m_f;
    
    parameters >> temp;
    if(temp != "fnom")
    {
        return false;
    }
    parameters >> m_fnom;
    
    // углы Эйлера
    parameters >> temp;
    if(temp != "alf,om,kap")
    {
        return false;
    }
    parameters >> m_alpha >> m_omega >> m_kappa;
    
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
    
    // координаты центра проекция в системе координат объекта
    parameters >> temp;
    if(temp != "X0")
    {
        return false;
    }
    parameters >> m_X0.x >> m_X0.y >> m_X0.z;
    
    parameters >> temp;
    if(temp != "CParamDig1i")
    {
        return false;
    }
    
    // масштаб пиксела
    parameters >> temp;
    if(temp != "m")
    {
        return false;
    }
    parameters >> m_mx >> m_my;
    
    // координаты главной точки
    parameters >> temp;
    if(temp != "b")
    {
        return false;
    }
    parameters >> m_b.x >> m_b.y;
    
    // параметры дисторсии Брауна
    parameters >> temp;
    if(temp != "adp")
    {
        return false;
    }
    parameters >> m_a >> m_K1 >> m_K2 >> m_K3 >> m_P1 >> m_P2;
    
    return true;
}