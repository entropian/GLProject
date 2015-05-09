#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Not sure why 
#define MAX_VERTICES 8000
#define MAX_POLYGONS 8000


/*
class Geometry
{
public:
    Geometry(GLfloat vtx[], GLuint edx[], int vl, int el)
        :vboLen(vl), eboLen(el)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        // Now create the VBO and IBO
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * eboLen, edx, GL_STATIC_DRAW);
        
        glBindVertexArray(0);       
    }

    Geometry(GLfloat vtx[], int vl)
        :vboLen(vl);
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }        
    
private:
    GLuint vao, vbo, ebo;
    int vboLen, eboLen;
    GLuint shaderProgram;
};
*/

struct Geometry {
    GLuint vao, vbo, ebo;
    int vboLen = 0, eboLen = 0;
    GLuint shaderProgram;

    Geometry(GLfloat vtx[], GLuint edx[], int vboLen, int eboLen) {
        this->vboLen = vboLen;
        this->eboLen = eboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        // Now create the VBO and IBO
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * eboLen, edx, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
    }

    Geometry(GLfloat vtx[], int vboLen) {
        this->vboLen = vboLen;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vboLen * 8, vtx, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
};

#endif
