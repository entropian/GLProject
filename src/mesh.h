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
    
    void initalize(const GLfloat *posArray, const GLfloat *normArray, const GLfloat *texcoordArray, const int *faceArray,
         unsigned int posArraySize, unsigned int normArraySize, unsigned int texcoordArraySize,
        unsigned int faceArraySize)
    {
        assert((posArraySize % 3) == 0);
        for(unsigned int i = 0; i < posArraySize; i += 3)
        {
            positions.push_back(Vec3(posArray[i], posArray[i+1], posArray[i + 2]));
        }
        assert(posArraySize == positions.size() * 3);

        assert((normArraySize % 3) == 0);
        for(unsigned int i = 0; i < normArraySize; i += 3)
        {
            normals.push_back(Vec3(normArray[i], normArray[i+1], normArray[i + 2]));
        }
        assert(normArraySize == normals.size() * 3);

        assert((texcoordArraySize % 2) == 0);
        for(unsigned int i = 0; i < texcoordArraySize; i += 2)
        {
            texcoords.push_back(Vec2(texcoordArray[i], texcoordArray[i+1]));
        }
        assert(texcoordArraySize == texcoords.size() * 2);

        assert((faceArraySize % 9) == 0);
        for(unsigned int i = 0; i < faceArraySize; i += 9)
        {
            Face face;
            face[0] = FaceVertex(faceArray[i], faceArray[i+2], faceArray[i+1]);
            face[1] = FaceVertex(faceArray[i+3], faceArray[i+5], faceArray[i+4]);
            face[2] = FaceVertex(faceArray[i+6], faceArray[i+8], faceArray[i+7]);
            faces.push_back(face);
        }
        assert(faceArraySize == (faces.size() * 3 * 3));

        /*
        for(unsigned int i = 0; i < positions.size(); i++)
        {
            if((positions[i][0] != posArray[i*3] || positions[i][1] != posArray[i*3 + 1]) ||
               positions[i][2] != posArray[i*3 + 2])
            {
                fprintf(stderr, "Positions vector incorrect.\n");
            }
        }

        for(unsigned int i = 0; i < normals.size(); i++)
        {
            if((normals[i][0] != normArray[i*3] || normals[i][1] != normArray[i*3 + 1]) ||
               normals[i][2] != normArray[i*3 + 2])
            {
                fprintf(stderr, "Normals vector incorrect.\n");
            }
        }

        for(unsigned int i = 0; i < texcoords.size(); i++)
        {
            if((texcoords[i][0] != texcoordArray[i*2] || texcoords[i][1] != texcoordArray[i*2 + 1]))
            {
                fprintf(stderr, "Texcoords vector incorrect.\n");
            }
        }


        for(unsigned int i = 0; i < faces.size(); i++)
        {
            if((faces[i][0] != faceArray[i*3] || faces[i][1] != faceArray[i*3 + 1]) ||
               faces[i][2] != faceArray[i*3 + 2])
            {
                fprintf(stderr, "Faces vector incorrect.\n");
            }
        }
        */
    }

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

private:
    std::vector<Vec3> positions, normals;
    std::vector<Vec2> texcoords;
    std::vector<Face> faces;
};
