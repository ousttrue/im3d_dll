#include "scene.h"
#include "im3d_math.h"
#include <GL/glew.h>
#include "glutil.h"
#include "teapot.h"
#include <Windows.h>

Scene::Scene()
{
}



void Scene::DrawTeapot(const float *viewProj, const float *world)
{
    static GLuint shTeapot;
    static GLuint vbTeapot;
    static GLuint ibTeapot;
    static GLuint vaTeapot;
    if (shTeapot == 0)
    {
        GLuint vs = LoadCompileShader(GL_VERTEX_SHADER, "model.glsl", "VERTEX_SHADER\0");
        GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "model.glsl", "FRAGMENT_SHADER\0");
        if (vs && fs)
        {
            shTeapot = glCreateProgram();
            glAttachShader(shTeapot, vs);
            glAttachShader(shTeapot, fs);
            bool ret = LinkShaderProgram(shTeapot);
            glDeleteShader(vs);
            glDeleteShader(fs);
            if (!ret)
            {
                return;
            }
        }
        else
        {
            return;
        }
        glGenBuffers(1, &vbTeapot);
        glGenBuffers(1, &ibTeapot);
        glGenVertexArrays(1, &vaTeapot);
        glBindVertexArray(vaTeapot);
        glBindBuffer(GL_ARRAY_BUFFER, vbTeapot);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec3) * 2, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)(sizeof(float) * 3));
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid *)s_teapotVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid *)s_teapotIndices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
    glUseProgram(shTeapot);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, world);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, viewProj);
    glBindVertexArray(vaTeapot);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid *)0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    glUseProgram(0);
}
