#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>

struct Geometry {
    GLuint vao;                // Vertex array object
    GLuint vbo;                // Vertex buffer object
    GLuint ebo;                // Index buffer object
    int vboLen = 0, eboLen = 0;                // number of vertices and number of indices
    int vertexSize = 0;
    GLuint shaderProgram;      // Handle to the shader program that last drew this geometry

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
        shaderProgram = g.shaderProgram;
    }

    Geometry& operator= (const Geometry &rhs)
    {
        vao = rhs.vao;
        vbo = rhs.vbo;
        ebo = rhs.ebo;
        vboLen = rhs.vboLen;
        eboLen = rhs.eboLen;
        shaderProgram = rhs.shaderProgram;
        return *this;
    }

};

struct GeoGroupInfo
{
    // NOTE: object name?
    size_t offset;
    size_t numGroups;
    char **mtlNames;  // Since each group has a corresponding material, mtlNames should have numGroups number of entries
                      // once the struct is populated
};

#endif
