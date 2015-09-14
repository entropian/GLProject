#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include "aabb.h"

const size_t MAX_NUM_SINGLE_GEOMETRY = 40;
const size_t MAX_GEOMETRY_GROUPS = 1000;

// Vertex attribute indices
const GLuint POSITION_ATTRIB_INDEX = 0;
const GLuint NORMAL_ATTRIB_INDEX = 1;
const GLuint TEXCOORD_ATTRIB_INDEX = 2;
const GLuint TANGENT_ATTRIB_INDEX = 3;
const GLuint BINORMAL_ATTRIB_INDEX = 4;
const GLuint DET_ATTRIB_INDEX = 5;

enum VertexAttrib{
    PNX,                      // position, normal, texcoord
    PNXTBD                    // position, normal, texcoord, tangent, binormal, determinant
};

static void setVertexAttributes(const GLuint vertexSize)
{
    glEnableVertexAttribArray(POSITION_ATTRIB_INDEX);
    glVertexAttribPointer(POSITION_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(NORMAL_ATTRIB_INDEX);
    glVertexAttribPointer(NORMAL_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(TEXCOORD_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat),
                          (void*)(6 * sizeof(GLfloat)));
    if(vertexSize == 15)
    {
        glEnableVertexAttribArray(TANGENT_ATTRIB_INDEX);
        glVertexAttribPointer(TANGENT_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat),
                              (void*)(8 * sizeof(GLfloat)));
        glEnableVertexAttribArray(BINORMAL_ATTRIB_INDEX);
        glVertexAttribPointer(BINORMAL_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat),
                              (void*)(11 * sizeof(GLfloat)));
        glEnableVertexAttribArray(DET_ATTRIB_INDEX);
        glVertexAttribPointer(DET_ATTRIB_INDEX, 1, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat),
                              (void*)(14 * sizeof(GLfloat)));
    }        
}

struct Geometry {
    GLuint vao;                // Vertex array object
    GLuint vbo;                // Vertex buffer object
    GLuint ebo;                // Index buffer object
    int vboLen = 0, eboLen = 0;                // number of vertices and number of indices
    int vertexSize = 0;
    //GLuint shaderProgram;      // Handle to the shader program that last drew this geometry
    char name[20];
    //AABB aabb;

    // Constructor for geometry with vertex indices
    Geometry(GLfloat vtx[], GLuint edx[], int vboLen, int eboLen, int vertexSize) {
        this->vboLen = vboLen;
        this->eboLen = eboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * vertexSize, vtx, GL_STATIC_DRAW);

        setVertexAttributes(vertexSize);
        this->vertexSize = vertexSize;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * eboLen, edx, GL_STATIC_DRAW);        
        glBindVertexArray(0);
    }

    // Constructor for geometry without vertex indices
    Geometry(GLfloat vtx[], int vboLen, int vertexSize) {
        this->vboLen = vboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * vertexSize, vtx, GL_STATIC_DRAW);
        
        setVertexAttributes(vertexSize);

        glBindVertexArray(0);

        this->vertexSize = vertexSize;
    }

    ~Geometry()
     {
         glDeleteVertexArrays(1, &vao);
         glDeleteBuffers(1, &vbo);
         if(eboLen > 0)
             glDeleteBuffers(1, &ebo);
     }

    Geometry(const Geometry &g)
    {
        vao = g.vao;
        vbo = g.vbo;
        ebo = g.ebo;
        vboLen = g.vboLen;
        eboLen = g.eboLen;
        //shaderProgram = g.shaderProgram;
    }

    Geometry& operator= (const Geometry &rhs)
    {
        vao = rhs.vao;
        vbo = rhs.vbo;
        ebo = rhs.ebo;
        vboLen = rhs.vboLen;
        eboLen = rhs.eboLen;
        //shaderProgram = rhs.shaderProgram;
        return *this;
    }

};

struct GeoGroupInfo
{
    char name[20];
    size_t offset;
    size_t numGroups;
    char **mtlNames;  // Since each group has a corresponding material, mtlNames should have numGroups number of entries
                      // once the struct is populated
};

struct Geometries{
    Geometry *groupGeo[MAX_GEOMETRY_GROUPS];
    int numGroupGeo = 0;
    GeoGroupInfo groupInfoList[MAX_GEOMETRY_GROUPS];
    const int groupLen = MAX_GEOMETRY_GROUPS;
    int numGroupInfo = 0;
};

#endif
