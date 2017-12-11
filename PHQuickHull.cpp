#include <IMLib/PHQuickHull.h>
#include <IMLib/PHConvexHull.h>
#include <deque>

using namespace std;

std::array<size_t,6> PHQuickHull::getExtremeValues()
{
    std::array<size_t,6> outIndices{0,0,0,0,0,0};
    double extremeVals[6] = {m_vertices[0].x,m_vertices[0].x,m_vertices[0].y,m_vertices[0].y,m_vertices[0].z,m_vertices[0].z};
    const size_t vCount = m_vertices.size();
    for (size_t i=1;i<vCount;i++) {
        const MAVector3 pos = m_vertices[i];
        if (pos.x>extremeVals[0]) {
            extremeVals[0]=pos.x;
            outIndices[0]=(size_t)i;
        }
        else if (pos.x<extremeVals[1]) {
            extremeVals[1]=pos.x;
            outIndices[1]=(size_t)i;
        }
        if (pos.y>extremeVals[2]) {
            extremeVals[2]=pos.y;
            outIndices[2]=(size_t)i;
        }
        else if (pos.y<extremeVals[3]) {
            extremeVals[3]=pos.y;
            outIndices[3]=(size_t)i;
        }
        if (pos.z>extremeVals[4]) {
            extremeVals[4]=pos.z;
            outIndices[4]=(size_t)i;
        }
        else if (pos.z<extremeVals[5]) {
            extremeVals[5]=pos.z;
            outIndices[5]=(size_t)i;
        }
    }
    return outIndices;
}

// построить поверхность
void PHQuickHull::buildMesh(const std::vector<MAVector3>& pointCloud, bool CCW, bool useOriginalIndices, double epsilon)
{
    m_extremeValues = getExtremeValues();
    
    m_scale = getScale(m_extremeValues);
    
    // рассчитываем эпсилон
    m_epsilon = epsilon*m_scale;
    m_epsilonSquared = m_epsilon*m_epsilon;
    
    // если все точки лежат в одной плоскости
    m_planar = false;
    
    // запускаем алгоритм
    createConvexHalfEdgeMesh();
}

// создаёт начальный тетраэдр
PHMesh PHQuickHull::createInitialTetrahedron()
{
    PHMesh result;
    
    const size_t vertexCount = m_vertices.size();
    
    // если у нас меньше 4 точек, строим вырожденный тетраэдр
    if(vertexCount <= 4)
    {
        size_t v[4] =
        {
            0,
            std::min((size_t)1,vertexCount-1),
            std::min((size_t)2,vertexCount-1),
            std::min((size_t)3,vertexCount-1)
        };
        
        MAVector3 N = MAVector3::triangleNormal(m_vertices[v[0]], m_vertices[v[1]], m_vertices[v[2]]);
        const MAPlane trianglePlane(N,m_vertices[v[0]]);
        if (trianglePlane.isPointOnPositiveSide(m_vertices[v[3]]))
        {
            std::swap(v[0],v[1]);
        }
        
        result = PHMesh(v[0],v[1],v[2],v[3]);
        return result;
    }
    
    // найти две самые дальние точки
    double maxD = m_epsilonSquared;
    std::pair<size_t,size_t> selectedPoints;
    for (size_t i=0;i<6;i++)
    {
        for (size_t j=i+1;j<6;j++)
        {
            const double d = m_vertices[ m_extremeValues[i] ].squaredDistanceTo( m_vertices[ m_extremeValues[j] ] );
            if (d > maxD)
            {
                maxD=d;
                selectedPoints={m_extremeValues[i],m_extremeValues[j]};
            }
        }
    }
    if (maxD == m_epsilonSquared)
    {
        // вырожденный случай: координаты всех точек совпадают
        result =  PHMesh(0,std::min((size_t)1,vertexCount-1),std::min((size_t)2,vertexCount-1),std::min((size_t)3,vertexCount-1));
    }
    
    // находим самую дальнюю точку, относительно выбранных двух точек
    const MARay r(m_vertices[selectedPoints.first], (m_vertices[selectedPoints.second] - m_vertices[selectedPoints.first]));
    maxD = m_epsilonSquared;
    size_t maxI=std::numeric_limits<size_t>::max();
    const size_t vCount = m_vertices.size();
    for (size_t i=0;i<vCount;i++)
    {
        const double distToRay = MARay::squaredDistanceBetweenPointAndRay(m_vertices[i], r);
        if (distToRay > maxD)
        {
            maxD=distToRay;
            maxI=i;
        }
    }
    if (maxD == m_epsilonSquared)
    {
        // случай, когда всё облако точек лежит в одной плоскости
        auto it = std::find_if(m_vertices.begin(),m_vertices.end(),[&](const MAVector3& ve)
        {
            return ve != m_vertices[selectedPoints.first] && ve != m_vertices[selectedPoints.second];
        });
        const size_t thirdPoint = (it == m_vertices.end()) ? selectedPoints.first : std::distance(m_vertices.begin(),it);
        it = std::find_if(m_vertices.begin(),m_vertices.end(),[&](const MAVector3& ve) {
            return ve != m_vertices[selectedPoints.first] && ve != m_vertices[selectedPoints.second] && ve != m_vertices[thirdPoint];
        });
        const size_t fourthPoint = (it == m_vertices.end()) ? selectedPoints.first : std::distance(m_vertices.begin(),it);
        result = PHMesh(selectedPoints.first,selectedPoints.second,thirdPoint,fourthPoint);
    }
    
    // формируем опорный треугольник из выбранных трёх точек
    std::array<size_t,3> baseTriangle{{selectedPoints.first, selectedPoints.second, maxI}};
    const MAVector3 baseTriangleVertices[]={ m_vertices[baseTriangle[0]], m_vertices[baseTriangle[1]],  m_vertices[baseTriangle[2]] };
    
    // находим самую дальнуюю точку, чтобы построить четвёртую вершину тетраэдра
    maxD=m_epsilon;
    maxI=0;
    const MAVector3 N = MAVector3::triangleNormal(baseTriangleVertices[0],baseTriangleVertices[1],baseTriangleVertices[2]);
    MAPlane trianglePlane(N,baseTriangleVertices[0]);
    for (size_t i=0;i<vCount;i++)
    {
        const double d = std::abs(MAPlane::signedDistanceToPlane(m_vertices[i], trianglePlane));
        if (d>maxD)
        {
            maxD=d;
            maxI=i;
        }
    }
    if (maxD == m_epsilon)
    {
        // все точки лежат в одной плоскости
        // добавляем ещё одну точку в облако точек, чтобы у выпуклой оболчки был ненулевой объём
        m_planar = true;
        const MAVector3 N = MAVector3::triangleNormal(baseTriangleVertices[1],baseTriangleVertices[2],baseTriangleVertices[0]);
        m_planarPointCloudTemp.clear();
        m_planarPointCloudTemp.insert(m_planarPointCloudTemp.begin(),m_vertices.begin(),m_vertices.end());
        const MAVector3 extraPoint = N + m_vertices[0];
        m_planarPointCloudTemp.push_back(extraPoint);
        maxI = m_planarPointCloudTemp.size()-1;
    }
    
    // проверяем, что получили ориентацию CCW
    const MAPlane triPlane(N,baseTriangleVertices[0]);
    if (triPlane.isPointOnPositiveSide(m_vertices[maxI]))
    {
        std::swap(baseTriangle[0],baseTriangle[1]);
    }
    
    // создаём сеточную модель тетраэдра из выбранных треугольников
    PHMesh mesh(baseTriangle[0],baseTriangle[1],baseTriangle[2],maxI);
    for (auto& f : mesh.m_faces)
    {
        auto v = mesh.getVertexIndicesOfFace(f);
        const MAVector3& va = m_vertices[v[0]];
        const MAVector3& vb = m_vertices[v[1]];
        const MAVector3& vc = m_vertices[v[2]];
        const MAVector3 N = MAVector3::triangleNormal(va, vb, vc);
        const MAPlane trianglePlane(N,va);
        f.m_P = trianglePlane;
    }
    
    // присваиваем вершины к каждой грани тетраэдра
    // вершины, лежащие внутри тетраэдра нас больше не интересуют
    for (size_t i=0;i<vCount;i++)
    {
        for (auto& face : mesh.m_faces)
        {
            if (addPointToFace(face, i))
            {
                break;
            }
        }
    }
    
    return mesh;
}

// создаёт выпуклую облочку
void PHQuickHull::createConvexHalfEdgeMesh()
{
    // видимые грани
    std::vector<size_t> visibleFaces;
    // рёбра, лежащие на горизонте
    std::vector<size_t> horizonEdges;
    
    struct PHFaceData
    {
        // индекс грани
        size_t m_faceIndex;
        // отмечает р
        size_t m_enteredFromHalfEdge;
        PHFaceData(size_t faceIndex, size_t horizonEdge) : m_faceIndex(faceIndex),m_enteredFromHalfEdge(horizonEdge)
        {}
    };
    
    std::vector<PHFaceData> possiblyVisibleFaces;
    m_mesh = createInitialTetrahedron();
    
    // создаём стек граней с прикреплёнными вершинами
    std::deque<size_t> faceList;
    for (size_t i=0;i < 4;i++)
    {
        auto& f = m_mesh.m_faces[i];
        if (f.m_pointsOnPositiveSide != nullptr)
        {
            //cout << f.m_pointsOnPositiveSide << endl;
            cout << f.m_pointsOnPositiveSide->size() << endl;
            if(f.m_pointsOnPositiveSide->size()>0)
            {
                faceList.push_back(i);
                f.m_inFaceStack = 1;
            }
        }
    }
    
    // обрабатываем вершины пока стек не пуст
    size_t iter = 0;
    while (!faceList.empty())
    {
        iter++;
        if (iter == std::numeric_limits<size_t>::max())
        {
            // помечаем посещённые вершины счётчиком итераций
            iter = 0;
        }
        
        const size_t topFaceIndex = faceList.front();
        faceList.pop_front();
        
        auto& tf = m_mesh.m_faces[topFaceIndex];
        tf.m_inFaceStack = 0;
        
        if (!tf.m_pointsOnPositiveSide || tf.isDisabled()) {
            continue;
        }
        
        //  выбираем самую дальную точку для текущего треугольника
        const MAVector3& activePoint = m_vertices[tf.m_mostDistantPoint];
        const size_t activePointIndex = tf.m_mostDistantPoint;
        
        // находим грани, которые видны из текущей активной точки
        // первый кандидат: верхняя грань в стеке
        // параллельно создаём список рёбер, лежащих на горизонте
        horizonEdges.clear();
        possiblyVisibleFaces.clear();
        visibleFaces.clear();
        possiblyVisibleFaces.emplace_back(topFaceIndex,std::numeric_limits<size_t>::max());
        while (possiblyVisibleFaces.size())
        {
            const auto faceData = possiblyVisibleFaces.back();
            possiblyVisibleFaces.pop_back();
            auto& pvf = m_mesh.m_faces[faceData.m_faceIndex];
            
            if (pvf.m_visibilityCheckedOnIteration == iter)
            {
                if (pvf.m_isVisibleFaceOnCurrentIteration)
                {
                    continue;
                }
            }
            else
            {
                const MAPlane& P = pvf.m_P;
                pvf.m_visibilityCheckedOnIteration = iter;
                const double d = P.m_normal.dotProduct(activePoint)+P.m_D;
                if (d>0)
                {
                    pvf.m_isVisibleFaceOnCurrentIteration = 1;
                    pvf.m_horizonEdgesOnCurrentIteration = 0;
                    visibleFaces.push_back(faceData.m_faceIndex);
                    for (auto heIndex : m_mesh.getHalfEdgeIndicesOfFace(pvf))
                    {
                        if (m_mesh.m_halfEdges[heIndex].m_opp != faceData.m_enteredFromHalfEdge)
                        {
                            possiblyVisibleFaces.emplace_back( m_mesh.m_halfEdges[m_mesh.m_halfEdges[heIndex].m_opp].m_face,heIndex );
                        }
                    }
                    continue;
                }
            }
            
            // грань не видна – следовательно полуребро, из которого мы пришли, лежит на горизонте
            pvf.m_isVisibleFaceOnCurrentIteration = 0;
            horizonEdges.push_back(faceData.m_enteredFromHalfEdge);
            // сохраняем полуребро, которое лежит на горизонте
            const auto halfEdges = m_mesh.getHalfEdgeIndicesOfFace(m_mesh.m_faces[m_mesh.m_halfEdges[faceData.m_enteredFromHalfEdge].m_face]);
            const std::int8_t ind = (halfEdges[0]==faceData.m_enteredFromHalfEdge) ? 0 : (halfEdges[1]==faceData.m_enteredFromHalfEdge ? 1 : 2);
            m_mesh.m_faces[m_mesh.m_halfEdges[faceData.m_enteredFromHalfEdge].m_face].m_horizonEdgesOnCurrentIteration |= (1<<ind);
        }
        const size_t horizonEdgeCount = horizonEdges.size();
        
        // формируем замкнутый путь из рёбер, лежащих на горизонте
        // операция может быть неудачной в силу погрешностей округления
        if (!reorderHorizonEdges(horizonEdges))
        {
            std::cerr << "Failed to solve horizon edge." << std::endl;
            auto it = std::find(tf.m_pointsOnPositiveSide->begin(),tf.m_pointsOnPositiveSide->end(),activePointIndex);
            tf.m_pointsOnPositiveSide->erase(it);
            if (tf.m_pointsOnPositiveSide->size()==0)
            {
                reclaimToIndexVectorPool(tf.m_pointsOnPositiveSide);
            }
            continue;
        }
        
        // все рёбра, кроме тех, что лежат на горизонте, могут быть удалены
        // помечаем их отключёнными, чтобы использовать память для новых рёбер
        // сохраняем указатели на вершины, видимые из данной точки
        m_newFaceIndices.clear();
        m_newHalfEdgeIndices.clear();
        m_disabledFacePointVectors.clear();
        size_t disableCounter = 0;
        for (auto faceIndex : visibleFaces)
        {
            auto& disabledFace = m_mesh.m_faces[faceIndex];
            auto halfEdges = m_mesh.getHalfEdgeIndicesOfFace(disabledFace);
            for (size_t j=0;j<3;j++)
            {
                if ((disabledFace.m_horizonEdgesOnCurrentIteration & (1<<j)) == 0)
                {
                    if (disableCounter < horizonEdgeCount*2)
                    {
                        // помечаем для использования на данной итерации
                        m_newHalfEdgeIndices.push_back(halfEdges[j]);
                        disableCounter++;
                    }
                    else
                    {
                        // помечаем для использования на следующих итерациях
                        m_mesh.disableHalfEdge(halfEdges[j]);
                    }
                }
            }
            // помечаем грань, как удалённую, но сохраняем указатели на точки, видимые из данной грани
            // на сохранённых точках будут построены новые грани
            auto t = std::move(m_mesh.disableFace(faceIndex));
            if (t)
            {
                m_disabledFacePointVectors.push_back(std::move(t));
            }
        }
        if (disableCounter < horizonEdgeCount*2) {
            const size_t newHalfEdgesNeeded = horizonEdgeCount*2-disableCounter;
            for (size_t i=0;i<newHalfEdgesNeeded;i++) {
                m_newHalfEdgeIndices.push_back(m_mesh.addHalfEdge());
            }
        }
        
        // создаём новые грани, используя рёбра, лежащие на горизонте
        for (size_t i = 0; i < horizonEdgeCount; i++)
        {
            const size_t AB = horizonEdges[i];
            
            auto horizonEdgeVertexIndices = m_mesh.getVertexIndicesOfHalfEdge(m_mesh.m_halfEdges[AB]);
            size_t A,B,C;
            A = horizonEdgeVertexIndices[0];
            B = horizonEdgeVertexIndices[1];
            C = activePointIndex;
            
            const size_t newFaceIndex = m_mesh.addFace();
            m_newFaceIndices.push_back(newFaceIndex);
            
            const size_t CA = m_newHalfEdgeIndices[2*i+0];
            const size_t BC = m_newHalfEdgeIndices[2*i+1];
            
            m_mesh.m_halfEdges[AB].m_next = BC;
            m_mesh.m_halfEdges[BC].m_next = CA;
            m_mesh.m_halfEdges[CA].m_next = AB;
            
            m_mesh.m_halfEdges[BC].m_face = newFaceIndex;
            m_mesh.m_halfEdges[CA].m_face = newFaceIndex;
            m_mesh.m_halfEdges[AB].m_face = newFaceIndex;
            
            m_mesh.m_halfEdges[CA].m_endVertex = A;
            m_mesh.m_halfEdges[BC].m_endVertex = C;
            
            auto& newFace = m_mesh.m_faces[newFaceIndex];
            
            const MAVector3 planeNormal = MAVector3::triangleNormal(m_vertices[A],m_vertices[B],activePoint);
            newFace.m_P = MAPlane(planeNormal,activePoint);
            newFace.m_he = AB;
            
            m_mesh.m_halfEdges[CA].m_opp = m_newHalfEdgeIndices[i>0 ? i*2-1 : 2*horizonEdgeCount-1];
            m_mesh.m_halfEdges[BC].m_opp = m_newHalfEdgeIndices[((i+1)*2) % (horizonEdgeCount*2)];
        }
        
        // присваиваем точки, которые были видны из удалённых гранях, созданным только что граням
        for (auto& disabledPoints : m_disabledFacePointVectors)
        {
            for (const auto& point : *(disabledPoints))
            {
                if (point == activePointIndex)
                {
                    continue;
                }
                for (size_t j=0;j<horizonEdgeCount;j++)
                {
                    if (addPointToFace(m_mesh.m_faces[m_newFaceIndices[j]], point))
                    {
                        break;
                    }
                }
            }
            // точки больше не будут использоваться: помещаем их в кэш для последующего использования
            reclaimToIndexVectorPool(disabledPoints);
        }
        
        // если требуется, увеличиваем размер стека граней
        for (const auto newFaceIndex : m_newFaceIndices)
        {
            auto& newFace = m_mesh.m_faces[newFaceIndex];
            if (newFace.m_pointsOnPositiveSide)
            {
                if (!newFace.m_inFaceStack) {
                    faceList.push_back(newFaceIndex);
                    newFace.m_inFaceStack = 1;
                }
            }
        }
    }
    
    // очищаем временный список вершин
    m_indexVectorPool.clear();
}

PHConvexHull PHQuickHull::createConvexHull(const std::vector<MAVector3>& pointCloud, bool CCW, bool useOriginalIndices, double epsilon)
{
    buildMesh(pointCloud,CCW,useOriginalIndices,epsilon);
    return PHConvexHull(m_mesh, m_vertices, CCW, useOriginalIndices);
}
