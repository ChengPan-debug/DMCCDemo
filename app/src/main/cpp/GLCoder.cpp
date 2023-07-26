//
// Created by DELL on 2020/11/25.
//

#include "GLCoder.h"


GLCoder::GLCoder(const char *vsSource, const char *fsSource) {
    GLint vShader = initShader(vsSource, GL_VERTEX_SHADER);
    GLint fShader = initShader(fsSource, GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    if (ID == 0) {
        LOGW("glCreate Program error.");
    }
    glAttachShader(ID, vShader);
    glAttachShader(ID, fShader);

    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void GLCoder::printGLString(const char *name, EGLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

GLint GLCoder::initShader(const char *source, GLint type) {

    GLint shader = glCreateShader(type);
    if (shader == 0) {
        LOGW("glCreateShader %d failed", type);
        return 0;
    }

    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    if (!checkCompileErrors(shader, "FRAGMENT")) {
        return 0;
    }
    LOGD("initShader %d success", type);
    return shader;
}

int GLCoder::checkCompileErrors(GLint shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            LOGE("ERROR::SHADER_COMPILATION_ERROR of type: %s \n%s", type.c_str(), infoLog);
            return 0;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            LOGE("ERROR::PROGRAM_LINKING_ERROR of type: : %s \n%s", type.c_str(), infoLog);
            return 0;
        }
    }

    return 1;
}


void GLCoder::use() {
    glUseProgram(ID);
}

void GLCoder::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void GLCoder::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void GLCoder::setFloat(const std::string &name, float value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
