#ifndef MESH_H
#define MESH_H
#include <GL/glew.h>
#include <vector>
#include "vec.h"
//#include "geometry.h"
#include "scenegraph.h"
#include "fileIO.h"

class Mesh
{
    struct FaceVertex
    {
        int posIndex;
        int normIndex;
        int texcoordIndex;

    
        FaceVertex()
        {}
        
        FaceVertex(int a, int b, int c)
            :posIndex(a), normIndex(b), texcoordIndex(c)
        {}            
    };
    
    class Face
    {
        FaceVertex faceVerts[3];
    public:
        Face()
        {}

        FaceVertex& operator [](const int i)
        {
            return faceVerts[i];
        }

        const FaceVertex& operator [](const int i) const
        {
            return faceVerts[i];
        }
    };

public:
    Mesh()
        :normalsComputed(false)
    {}
    
    

    Mesh(const Mesh& m)
    {
        *this = m;
    }

    void readFromObj(const char* fileName);
    void computeVertexNormals();
    void computeVertexBasis();
    // TODO: get rid of this
    void computeFaceNormals();
    Geometry* produceGeometryPNX();
    Geometry* produceGeometryPNXTBD();
    size_t geoListPNX(Geometry ***, char ***);
    size_t geoListPNXTBD(Geometry ***, char ***);
    void flipTexcoordY();
    void initialize(const GLfloat*, const GLfloat*, const GLfloat*, const size_t*, const size_t*, char**, const size_t,
                    const size_t, const size_t, const size_t, const size_t);
    void initialize(OBJData*);
private:
    unsigned int extractObjData(const char*, const size_t, GLfloat*, GLfloat *, GLfloat*, size_t*, size_t*, char**);
    unsigned int extractObjData(const char*, const size_t, GLfloat*, GLfloat*, size_t*, size_t*, char**);
    void vertexAttribPNX(GLfloat*, size_t*, const size_t, const size_t);
    void vertexAttribPNXTBD(GLfloat*, size_t*, const size_t, const size_t);
    Geometry* geometryFromGroupPNX(size_t);

    
    std::vector<Vec3> positions, normals;
    std::vector<Vec3> tangents, binormals;
    std::vector<Vec2> texcoords;
    std::vector<Face> faces;
    std::vector<std::pair<unsigned int, std::string> > groups;
    std::vector<float> determinants;
    bool normalsComputed;
};

static bool getGeoList(Mesh &mesh, Geometry *geometryGroups[], GeoGroupInfo groupInfoList[], size_t MAX_GEOMETRY_GROUPS,
                       size_t &groupSize, int &groupInfoSize, VertexAttrib va)
{
    Geometry **geoList;
    char **mtlNames;
    size_t numGroups = 0;;
    if(va == PNX)
        numGroups = mesh.geoListPNX(&geoList, &mtlNames);
    else if(va == PNXTBD)
        numGroups = mesh.geoListPNXTBD(&geoList, &mtlNames);

    if(groupSize + numGroups > MAX_GEOMETRY_GROUPS)
    {
        fprintf(stderr, "Too many groups\n");
        for(size_t i = 0; i < numGroups ; i++)
        {
            free(geoList[i]);
            free(mtlNames[i]);
        }
        free(geoList);
        free(mtlNames);
        return false;
    }else
    {
        for(size_t i = 0; i < numGroups; i++)
            geometryGroups[groupSize + i] = geoList[i];

        free(geoList);        
        groupInfoList[groupInfoSize].mtlNames = mtlNames;
        groupInfoList[groupInfoSize].offset = groupSize;
        groupInfoList[groupInfoSize].numGroups = numGroups;
        
        groupSize += numGroups;
        groupInfoSize++;
        
        return true;
    }
}

#endif
