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
    {}
    
    

    Mesh(const Mesh& m)
    {
        *this = m;
    }


    void vertexAttributePerVertex(GLfloat *vertexArray, int *vertexIndex, int i, int j)
    {
        // NOTE: what's with all the reversal?
        // the z component of position and normal are both reversed
        // normal used to be untouched, and the object was lit on the wrong side        
        vertexArray[(*vertexIndex)++] = positions[faces[i][j].posIndex][0];
        vertexArray[(*vertexIndex)++] = positions[faces[i][j].posIndex][1];
        vertexArray[(*vertexIndex)++] = -positions[faces[i][j].posIndex][2];
        // Normal
        vertexArray[(*vertexIndex)++] = normals[faces[i][j].normIndex][0];
        vertexArray[(*vertexIndex)++] = normals[faces[i][j].normIndex][1];
        vertexArray[(*vertexIndex)++] = -normals[faces[i][j].normIndex][2];
        // Texcoord
        // also reversed the v component of texcoord
        vertexArray[(*vertexIndex)++] = texcoords[faces[i][j].texcoordIndex][0];
        vertexArray[(*vertexIndex)++] = 1.0f - texcoords[faces[i][j].texcoordIndex][1];
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

        int vertexIndex = 0;
        for(unsigned int i = 0; i < faces.size(); i++)
        {
            
            vertexAttributePerVertex(vertexArray, &vertexIndex, i, 0);
            vertexAttributePerVertex(vertexArray, &vertexIndex, i, 1);
            vertexAttributePerVertex(vertexArray, &vertexIndex, i, 2);
        }
        assert((vertexIndex % 8) == 0);
        *numVertices = vertexIndex / 8;

        return vertexArray;
    }

    void readFromObj(const char* fileName, int *numVertices);
    void computeNormals();
    void initialize(const GLfloat*, const GLfloat*, const int*, unsigned int, unsigned int, unsigned int);
    void initialize(const GLfloat*, const GLfloat*, const GLfloat*, const int*, unsigned int, unsigned int, unsigned int, unsigned int);

private:
    int extractObjData(const char*, const int, GLfloat*, GLfloat *, GLfloat*, int*);
    int extractObjData(const char*, const int, GLfloat*, GLfloat*, int*);
    
    std::vector<Vec3> positions, normals;
    std::vector<Vec2> texcoords;
    std::vector<Face> faces;
};
