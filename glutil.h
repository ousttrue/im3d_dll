#pragma once
#include <gl/glew.h>

GLuint LoadCompileShader(GLenum _stage, const char *_path, const char *_defines);
bool LinkShaderProgram(GLuint _handle);

void GLClearState(int w, int h);