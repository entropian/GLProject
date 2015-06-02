#ifndef MESH_H
#define MESH_H
#include <GL/glew.h>
#include <vector>
#include "vec.h"
#include "geometry.h"


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

    GLfloat* vertexArray(int *numVertices)
    {
        if(((positions.size() == 0 || normals.size() == 0) || texcoords.size() == 0) || faces.size() == 0)
        {
            fprintf(stderr, "Mesh doesn't contain necessary data to produce a Geometry object.\n");
            return NULL;
        }

        /*
          NOTE: vertexCount used to be 1/3 of the correct number, so vertexArray was also 1/3 of the correct size.
          However, in the for loop below, instead of i++, I had it as i+=3, so it all somehow fit?
          What it caused, in addition to not having the correct geometry, is heap corruption (not sure how).
          In the function that called this one, freeing unrelated memory would cause a heap corruption error.
         */
        unsigned int vertexCount = int((float)faces.size() * 3 * 3 * (2.0f + 2.0f/3.0f));
        GLfloat *vertexArray = (GLfloat*)malloc(sizeof(GLfloat)*vertexCount);

        size_t vertexIndex = 0;
        for(unsigned int i = 0; i < faces.size(); i++)
        {
            vertexAttribPNX(vertexArray, &vertexIndex, i, 0);
            vertexAttribPNX(vertexArray, &vertexIndex, i, 1);
            vertexAttribPNX(vertexArray, &vertexIndex, i, 2);                        
        }
        assert((vertexIndex % 8) == 0);
        *numVertices = vertexIndex / 8;

        return vertexArray;
    }

    void readFromObj(const char* fileName);
    void computeVertexNormals();
    void computeVertexBasis();
    Geometry* produceGeometryPNX();
    Geometry* produceGeometryPNXTBD();
    size_t geoListPNX(Geometry ***, char ***);
    void flipTexcoordY();
    void initialize(const GLfloat*, const GLfloat*, const GLfloat*, const size_t*, const size_t*, char**, const size_t,
                    const size_t, const size_t, const size_t, const size_t);

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

#endif
