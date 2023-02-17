#include "shadermng.h"

#include <vector>
#include <cstdio>
#include <string>

#define STR_SWCASE(prefix, ec, id) case ec::id: return prefix #id

inline const char* strShaderAttrib(int a)
{
    switch ((ShaderAttrib)a)
    {
    STR_SWCASE("a", ShaderAttrib, Pos);
    STR_SWCASE("a", ShaderAttrib, Color);
    STR_SWCASE("a", ShaderAttrib, Tex);
    STR_SWCASE("a", ShaderAttrib, Normal);
    STR_SWCASE("a", ShaderAttrib, Tan);
    STR_SWCASE("a", ShaderAttrib, Bitan);
    default: assert(false && "unexpected id"); return nullptr;
    }
}

inline const char* strShaderUniMat4(int a)
{
    switch ((ShaderUniMat4)a)
    {
    STR_SWCASE("u", ShaderUniMat4, ProjMat);
    STR_SWCASE("u", ShaderUniMat4, ModelViewMat);
    STR_SWCASE("u", ShaderUniMat4, TexMat);
    default: assert(false && "unexpected id"); return nullptr;
    }
}

inline const char* strShaderUniVec4(int a)
{
    switch ((ShaderUniVec4)a)
    {
    STR_SWCASE("u", ShaderUniVec4, LightPos);
    STR_SWCASE("u", ShaderUniVec4, LightAmbient);
    STR_SWCASE("u", ShaderUniVec4, LightDiffuse);
    default: assert(false && "unexpected id"); return nullptr;
    }
}

inline const char* strShaderUniInt(int a)
{
    switch ((ShaderUniInt)a)
    {
    STR_SWCASE("u", ShaderUniInt, AlphaDiscard);
    STR_SWCASE("u", ShaderUniInt, Halftone);
    default: assert(false && "unexpected id"); return nullptr;
    }
}

inline const char* strShaderUniTex(int a)
{
    switch ((ShaderUniTex)a)
    {
    STR_SWCASE("u", ShaderUniTex, Tex0);
    STR_SWCASE("u", ShaderUniTex, Tex1);
    STR_SWCASE("u", ShaderUniTex, Cube);
    default: assert(false && "unexpected id"); return nullptr;
    }
}

#undef STR_SWCASE

ShaderMng::ShaderMng() : ints()
{
    currentShader = ShaderId::None;
}

static GLuint loadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
// Create the shader object
    shader = glCreateShader(type);
    if(shader == 0)
    {
        return 0;
    }
// Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);
// Compile the shader
    glCompileShader(shader);
// Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1)
        {
            std::vector<char> infoLog(infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog.data());
            fprintf(stderr, "%s: %s\n", (type == GL_VERTEX_SHADER ? "VS" : "FS"), infoLog.data());
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void ShaderMng::init()
{
#if 0 // test stringify completeness
#define TEST_STR_EC_COMPLETE(ec) do { for (int i = 0; i != (int)ec::Count; ++i) str##ec(i); } while (false)
TEST_STR_EC_COMPLETE(ShaderAttrib);
TEST_STR_EC_COMPLETE(ShaderUniMat4);
TEST_STR_EC_COMPLETE(ShaderUniVec4);
TEST_STR_EC_COMPLETE(ShaderUniInt);
TEST_STR_EC_COMPLETE(ShaderUniTex);
#undef TEST_STR_EC_COMPLETE
#endif

    for (int i = 0; i != (int)ShaderId::Count; ++i)
    {
        ShaderWrap& sh = shaders[i];
        sh.program = 0;
        std::string vs;
        std::string fs;
        ShaderId id = (ShaderId)i;
        switch (id)
        {
        case ShaderId::ColorTex:
            #include "shaders/color_tex.vs.h"
            #include "shaders/color_tex.fs.h"
            break;
        case ShaderId::Color:
            #include "shaders/color.vs.h"
            #include "shaders/color.fs.h"
            break;
        case ShaderId::LightTex:
            #include "shaders/light_tex.vs.h"
            #include "shaders/light_tex.fs.h"
            break;
        case ShaderId::LightTexSunk:
            #include "shaders/light_tex_sunk.vs.h"
            #include "shaders/light_tex_sunk.fs.h"
            break;
        case ShaderId::Tex:
            #include "shaders/tex.vs.h"
            #include "shaders/tex.fs.h"
            break;
        case ShaderId::CarTop:
            #include "shaders/car_top.vs.h"
            #include "shaders/car_top.fs.h"
            break;
        case ShaderId::Car:
            #include "shaders/car.vs.h"
            #include "shaders/car.fs.h"
            break;
        case ShaderId::GlassTint:
            #include "shaders/glass_tint.vs.h"
            #include "shaders/glass_tint.fs.h"
            break;
        case ShaderId::GlassReflection:
            #include "shaders/glass_reflection.vs.h"
            #include "shaders/glass_reflection.fs.h"
            break;
        default:
            break;
        }

        if (!vs.empty() && !fs.empty())
        {

            if (g_opengl_profile == PROFILE_ES2)
            {
                fs = "precision mediump float;"+fs;
            }
            else
            {
                static const char* version120line = "#version 120";
                vs = version120line+vs;
                fs = version120line+fs;
            }
            GLuint programObject = glCreateProgram();
            for (int i = 0; i != (GLuint)ShaderAttrib::Count; ++i)
            {
                glBindAttribLocation(programObject, i, strShaderAttrib(i));
            }
            GLuint vId = loadShader(GL_VERTEX_SHADER, vs.c_str());
            GLuint fId = loadShader(GL_FRAGMENT_SHADER, fs.c_str());

            if (programObject != 0)
            {
                glAttachShader(programObject, vId);
                glAttachShader(programObject, fId);

                // Link the program
                glLinkProgram(programObject);
                // Check the link status
                GLint linked;
                glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
                if(!linked)
                {
                    GLint infoLen = 0;
                    glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
                    if(infoLen > 1)
                    {
                        std::vector<char> infoLog(infoLen);
                        glGetProgramInfoLog(programObject, infoLen, NULL, infoLog.data());
                        fprintf(stderr, "LINK:%s\n", infoLog.data());
                    }
                    glDeleteProgram(programObject);
                }
                sh.pmvloc = glGetUniformLocation(programObject, "uProjModelViewMat");
                sh.normloc = glGetUniformLocation(programObject, "uNormMat");
                for (int j = 0; j != (int)ShaderUniMat4::Count; ++j)
                {
                    sh.mat4locs[j] = glGetUniformLocation(programObject, strShaderUniMat4(j));
                }
                for (int j = 0; j != (int)ShaderUniVec4::Count; ++j)
                {
                    sh.vec4locs[j] = glGetUniformLocation(programObject, strShaderUniVec4(j));
                }
                for (int j = 0; j != (int)ShaderUniInt::Count; ++j)
                {
                    sh.intlocs[j] = glGetUniformLocation(programObject, strShaderUniInt(j));
                }

                glUseProgram(programObject);
                for (int j = 0; j != (int)ShaderUniTex::Count; ++j)
                {
                    GLint loc = glGetUniformLocation(programObject, strShaderUniTex(j));
                    if (loc != -1)
                    {
                        glUniform1i(loc, j);
                    }
                }
                sh.program = programObject;
            }
        }
    }
    glUseProgram(0);
}

void ShaderMng::use(ShaderId id)
{
    glUseProgram(0);
    currentShader = id;
    GLint loc;
    if (id != ShaderId::None)
    {
        glUseProgram(shaders[(int)id].program);
        for (int i = 0; i != (int)ShaderUniMat4::Count; ++i)
        {
            GLint loc = shaders[(int)currentShader].mat4locs[i];
            if (loc != -1)
            {
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat4s[i]));
            }
        }
        if ((loc = shaders[(int)currentShader].pmvloc) != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat4pmv));
        }
        if ((loc = shaders[(int)currentShader].normloc) != -1)
        {
            glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat3norm));
        }
        for (int i = 0; i != (int)ShaderUniVec4::Count; ++i)
        {
            GLint loc = shaders[(int)currentShader].vec4locs[i];
            if (loc != -1)
            {
                glUniform4fv(loc, 1, glm::value_ptr(vec4s[i]));
            }
        }
        for (int i = 0; i != (int)ShaderUniInt::Count; ++i)
        {
            GLint loc = shaders[(int)currentShader].intlocs[i];
            if (loc != -1)
            {
                glUniform1i(loc, ints[i]);
            }
        }
    }
}

void ShaderMng::set(ShaderUniMat4 id, glm::mat4 m)
{
    mat4s[(int)id] = m;
    bool updatedPmv = false;
    bool updatedNorm = false;
    if (id == ShaderUniMat4::ModelViewMat)
    {
        mat4pmv = mat4s[(int)ShaderUniMat4::ProjMat] * m;
        updatedPmv = true;
        mat3norm = glm::transpose(glm::inverse(m));
        updatedNorm = true;
    }
    // if the following condition is commented out, then the ProjMat must be set before corresponding ModelViewMat
    /*if (id == ShaderUniMat4::ProjMat)
    {
        mat4pmv = m * mat4s[(int)ShaderUniMat4::ModelViewMat];
        updatedPmv = true;
    }*/
    if (currentShader != ShaderId::None)
    {
        glUseProgram(shaders[(int)currentShader].program);
        GLint loc = shaders[(int)currentShader].mat4locs[(int)id];
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m)); checkGL();
        }
        if (updatedPmv)
        {
            GLint loc = shaders[(int)currentShader].pmvloc;
            if (loc != -1)
            {
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat4pmv)); checkGL();
            }
        }
        if (updatedNorm)
        {
            GLint loc = shaders[(int)currentShader].normloc;
            if (loc != -1)
            {
                glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat3norm)); checkGL();
            }
        }
    }
}

void ShaderMng::set(ShaderUniVec4 id, glm::vec4 v)
{
    vec4s[(int)id] = v;
    if (currentShader != ShaderId::None)
    {
        GLint loc = shaders[(int)currentShader].vec4locs[(int)id];
        if (loc != -1)
        {
            glUniform4fv(loc, 1, glm::value_ptr(v)); checkGL();
        }
    }
}

void ShaderMng::set(ShaderUniInt id, GLint i)
{
    ints[(int)id] = i;
    if (currentShader != ShaderId::None)
    {
        GLint loc = shaders[(int)currentShader].intlocs[(int)id];
        if (loc != -1)
        {
            glUniform1i(loc, i); checkGL();
        }
    }
}
