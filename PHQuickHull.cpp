#include <IMLib/PHQuickHull.h>
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
void PHQuickHull::buildMesh(double epsilon)
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
        
        return PHMesh(v[0],v[1],v[2],v[3]);
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
        // A degenerate case: the point cloud seems to consists of a single point
        result =  PHMesh(0,std::min((size_t)1,vertexCount-1),std::min((size_t)2,vertexCount-1),std::min((size_t)3,vertexCount-1));
    }
    
    // Find the most distant point to the line between the two chosen extreme points.
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
        // It appears that the point cloud belongs to a 1 dimensional subspace of R^3: convex hull has no volume => return a thin triangle
        // Pick any point other than selectedPoints.first and selectedPoints.second as the third point of the triangle
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
    
    // These three points form the base triangle for our tetrahedron.
    std::array<size_t,3> baseTriangle{selectedPoints.first, selectedPoints.second, maxI};
    const MAVector3 baseTriangleVertices[]={ m_vertices[baseTriangle[0]], m_vertices[baseTriangle[1]],  m_vertices[baseTriangle[2]] };
    
    // Next step is to find the 4th vertex of the tetrahedron. We naturally choose the point farthest away from the triangle plane.
    maxD=m_epsilon;
    maxI=0;
    const MAVector3 N = MAVector3::triangleNormal(baseTriangleVertices[0],baseTriangleVertices[1],baseTriangleVertices[2]);
    MAPlane trianglePlane(N,baseTriangleVertices[0]);
    for (size_t i=0;i<vCount;i++) {
        const double d = std::abs(MAPlane::signedDistanceToPlane(m_vertices[i], trianglePlane));
        if (d>maxD) {
            maxD=d;
            maxI=i;
        }
    }
    if (maxD == m_epsilon) {
        // All the points seem to lie on a 2D subspace of R^3. How to handle this? Well, let's add one extra point to the point cloud so that the convex hull will have volume.
        m_planar = true;
        const MAVector3 N = MAVector3::triangleNormal(baseTriangleVertices[1],baseTriangleVertices[2],baseTriangleVertices[0]);
        m_planarPointCloudTemp.clear();
        m_planarPointCloudTemp.insert(m_planarPointCloudTemp.begin(),m_vertices.begin(),m_vertices.end());
        const MAVector3 extraPoint = N + m_vertices[0];
        m_planarPointCloudTemp.push_back(extraPoint);
        maxI = m_planarPointCloudTemp.size()-1;
        // TODO m_vertexData = VertexDataSource<T>(m_planarPointCloudTemp);
    }
    
    // Enforce CCW orientation (if user prefers clockwise orientation, swap two vertices in each triangle when final mesh is created)
    const MAPlane triPlane(N,baseTriangleVertices[0]);
    if (triPlane.isPointOnPositiveSide(m_vertices[maxI])) {
        std::swap(baseTriangle[0],baseTriangle[1]);
    }
    
    // Create a tetrahedron half edge mesh and compute planes defined by each triangle
    PHMesh mesh(baseTriangle[0],baseTriangle[1],baseTriangle[2],maxI);
    for (auto& f : mesh.m_faces) {
        auto v = mesh.getVertexIndicesOfFace(f);
        const MAVector3& va = m_vertices[v[0]];
        const MAVector3& vb = m_vertices[v[1]];
        const MAVector3& vc = m_vertices[v[2]];
        const MAVector3 N = MAVector3::triangleNormal(va, vb, vc);
        const MAPlane trianglePlane(N,va);
        f.m_P = trianglePlane;
    }
    
    // Finally we assign a face for each vertex outside the tetrahedron (vertices inside the tetrahedron have no role anymore)
    for (size_t i=0;i<vCount;i++) {
        for (auto& face : mesh.m_faces) {
            if (addPointToFace(face, i)) {
                break;
            }
        }
    }
    
    return result;
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
        if (f.m_pointsOnPositiveSide && f.m_pointsOnPositiveSide->size()>0) {
            faceList.push_back(i);
            f.m_inFaceStack = 1;
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
        
        // Pick the most distant point to this triangle plane as the point to which we extrude
        const MAVector3& activePoint = m_vertices[tf.m_mostDistantPoint];
        const size_t activePointIndex = tf.m_mostDistantPoint;
        
        // Find out the faces that have our active point on their positive side (these are the "visible faces"). The face on top of the stack of course is one of them. At the same time, we create a list of horizon edges.
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
            
            // The face is not visible. Therefore, the halfedge we came from is part of the horizon edge.
            pvf.m_isVisibleFaceOnCurrentIteration = 0;
            horizonEdges.push_back(faceData.m_enteredFromHalfEdge);
            // Store which half edge is the horizon edge. The other half edges of the face will not be part of the final mesh so their data slots can by recycled.
            const auto halfEdges = m_mesh.getHalfEdgeIndicesOfFace(m_mesh.m_faces[m_mesh.m_halfEdges[faceData.m_enteredFromHalfEdge].m_face]);
            const std::int8_t ind = (halfEdges[0]==faceData.m_enteredFromHalfEdge) ? 0 : (halfEdges[1]==faceData.m_enteredFromHalfEdge ? 1 : 2);
            m_mesh.m_faces[m_mesh.m_halfEdges[faceData.m_enteredFromHalfEdge].m_face].m_horizonEdgesOnCurrentIteration |= (1<<ind);
        }
        const size_t horizonEdgeCount = horizonEdges.size();
        
        // Order horizon edges so that they form a loop. This may fail due to numerical instability in which case we give up trying to solve horizon edge for this point and accept a minor degeneration in the convex hull.
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
        
        // Except for the horizon edges, all half edges of the visible faces can be marked as disabled. Their data slots will be reused.
        // The faces will be disabled as well, but we need to remember the points that were on the positive side of them - therefore
        // we save pointers to them.
        m_newFaceIndices.clear();
        m_newHalfEdgeIndices.clear();
        m_disabledFacePointVectors.clear();
        size_t disableCounter = 0;
        for (auto faceIndex : visibleFaces) {
            auto& disabledFace = m_mesh.m_faces[faceIndex];
            auto halfEdges = m_mesh.getHalfEdgeIndicesOfFace(disabledFace);
            for (size_t j=0;j<3;j++) {
                if ((disabledFace.m_horizonEdgesOnCurrentIteration & (1<<j)) == 0) {
                    if (disableCounter < horizonEdgeCount*2) {
                        // Use on this iteration
                        m_newHalfEdgeIndices.push_back(halfEdges[j]);
                        disableCounter++;
                    }
                    else {
                        // Mark for reusal on later iteration step
                        m_mesh.disableHalfEdge(halfEdges[j]);
                    }
                }
            }
            // Disable the face, but retain pointer to the points that were on the positive side of it. We need to assign those points
            // to the new faces we create shortly.
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
        
        // Create new faces using the edgeloop
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
        
        // Assign points that were on the positive side of the disabled faces to the new faces.
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
            // The points are no longer needed: we can move them to the vector pool for reuse.
            reclaimToIndexVectorPool(disabledPoints);
        }
        
        // Increase face stack size if needed
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
