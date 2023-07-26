//
// Created by DELL on 2020/11/25.
//

#ifndef MFFMPEG_GLCODER_H
#define MFFMPEG_GLCODER_H
#include <jni.h>
#include <string>
#include "ipprotocol/ipprotocoldeflog.h"
#include <android/native_window_jni.h>
#include <iostream>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>


#define GETSTR(x) #x
static char* OUTPUT_VIEW_VERTEX_SHADER =
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTextCoord;\n"
        "varying vec2 vTextCoord;\n"
        "void main() {\n"
        "vTextCoord = vec2(aTextCoord.x, 1.0 - aTextCoord.y);\n"
        "gl_Position = aPosition;\n"
        "}\n"
        ;

static char* YUV_FRAME_FRAGMENT_SHADER =
        "precision mediump float;\n"
        "varying vec2 vTextCoord;\n"
        "uniform sampler2D yTexture;\n"
        "uniform sampler2D uTexture;\n"
        "uniform sampler2D vTexture;\n"
        "void main() {\n"
        "    vec3 yuv;\n"
        "    vec3 rgb;\n"
        "    yuv.x = texture2D(yTexture, vTextCoord).g;\n"
        "    yuv.y = texture2D(uTexture, vTextCoord).g - 0.5;\n"
        "    yuv.z = texture2D(vTexture, vTextCoord).g - 0.5;\n"
        "    rgb = mat3(\n"
        "            1.0, 1.0, 1.0,\n"
        "            0.0, -0.39465, 2.03211,\n"
        "            1.13983, -0.5806, 0.0\n"
        "    ) * yuv;\n"
        "    gl_FragColor = vec4(rgb, 1.0);\n"
        "}\n"
        ;

class GLCoder {
public:
    GLCoder(const char *vsSource,const char *fsSource);
    void printGLString(const char *name, EGLenum s);
    GLint initShader(const char *source, GLint type);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

    GLuint ID;
private:
    int checkCompileErrors(GLint shader, std::string type);


};


#endif //MFFMPEG_GLCODER_H
