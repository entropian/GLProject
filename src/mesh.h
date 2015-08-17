#ifndef MESH_H
#define MESH_H
#include <GL/glew.h>
#include <vector>
#include <string>
#include "vec.h"
#include "geometry.h"
//#include "scenegraph.h"
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

    // Load mesh data from an OBJ file specified by fileName
    void loadOBJFile(const char* fileName);
    
    void computeVertexNormals();
    
    // Compute the orthonormal basis for every mesh vertex
    void computeVertexBasis();
    
    // Returns a Geometry* with positions, normals, and texcoords as its vertex attributes
    Geometry* produceGeometryPNX();
    
    // Returns a Geometry* with positions, normals, texcoords, tangent, bitangent, and determinant as its vertex attributes    
    Geometry* produceGeometryPNXTBD();

    Geometry* produceGeometry(VertexAttrib);

    /*
      Store an array of Geometry objects in Geometry***, each with the vertex attributes
      position, normal, and texcoord. char*** will contain the material name for each Geometry
      object
    */
    size_t geoListPNX(Geometry ***, char ***);

    /*
      Store an array of Geometry objects in Geometry***, each with the vertex attributes
      position, normal, texcoord, tangent, bitangent, and determinant. char*** will contain
      the material name for each Geometry object.
    */
    size_t geoListPNXTBD(Geometry ***, char ***);

    size_t geoList(Geometry ***, char ***, VertexAttrib);

    std::string getName();

    int getNumGroups()
    {
        return groups.size();
    }

private:
    void vertexAttribPNX(GLfloat*, size_t*, const size_t, const size_t);
    void vertexAttribPNXTBD(GLfloat*, size_t*, const size_t, const size_t);
    void initialize(OBJData*);
    Geometry* geometryFromGroupPNX(size_t);
    Geometry* geometryFromGroupPNXTBD(size_t);
    Geometry* geometryFromGroup(size_t, VertexAttrib);
    void flipTexcoordY();
    
    std::vector<Vec3> positions, normals;
    std::vector<Vec3> tangents, binormals;
    std::vector<Vec2> texcoords;
    std::vector<Face> faces;
    std::vector<std::pair<unsigned int, std::string> > groups;
    std::vector<float> determinants;
    bool normalsComputed;
    std::string name;
};

/*
  Takes the vertex attributes in mesh, and produces Geometry objects according to the
  grouping info in the mesh. The Geometry objects are stored in geometryGrousp[].
  Additional information about the groups of Geometry objects are stored in groupInfoList.
 */
static bool getGeoList(Mesh &mesh, Geometry *geometryGroups[], GeoGroupInfo groupInfoList[], const size_t MAX_GEOMETRY_GROUPS,
                       int &groupSize, int &groupInfoSize, VertexAttrib va)
{
    Geometry **geoList;
    char **mtlNames;
    size_t numGroups = 0;;

    numGroups = mesh.geoList(&geoList, &mtlNames, va);
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
        strcpy(groupInfoList[groupInfoSize].name, mesh.getName().c_str());
        groupInfoList[groupInfoSize].mtlNames = mtlNames;
        groupInfoList[groupInfoSize].offset = groupSize;
        groupInfoList[groupInfoSize].numGroups = numGroups;
        
        groupSize += numGroups;
        groupInfoSize++;
        
        return true;
    }
}

#endif
