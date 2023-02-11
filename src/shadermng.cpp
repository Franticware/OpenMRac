#include "shadermng.h"

#include <vector>
#include <cstdio>
#include <string>

ShaderMng::ShaderMng()
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
    const char* vShaderTexSrc =
R"SRC(
uniform mat4 uProjModelViewMat;

attribute vec3 aPos;
attribute vec2 aTex;

varying vec2 vTex;

void main()
{
    vTex = aTex;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)SRC";

    const char* fShaderTexSrc =
R"SRC(
uniform sampler2D uTex0;

varying vec2 vTex;

void main()
{
    gl_FragColor = texture2D( uTex0, vTex );
}
)SRC";

    // ///////////////////

    const char* vShaderTexColorSrc =
R"SRC(
uniform mat4 uProjModelViewMat;

attribute vec3 aPos;
attribute vec4 aColor;
attribute vec2 aTex;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
    vTex = aTex;
    vColor = aColor;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)SRC";

    const char* fShaderTexColorSrc =
R"SRC(
uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
    //uniform int uHalftone;
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    gl_FragColor = texture2D( uTex0, vTex ) * vColor;
    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)SRC";

    // ///////////////////


    const char* vShaderColorSrc =
R"SRC(
uniform mat4 uProjModelViewMat;

attribute vec3 aPos;
attribute vec4 aColor;

varying vec4 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)SRC";

    const char* fShaderColorSrc =
R"SRC(
varying vec4 vColor;

void main()
{
    gl_FragColor = vColor;
}
)SRC";

    // ///////////////////

    const char* vShaderTexLightSrc =
R"SRC(
uniform mat4 uModelViewMat;
uniform mat4 uProjModelViewMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTex;

varying vec2 vTex;
varying vec3 vNormal;

void main()
{
    vTex = aTex;
    vNormal = uNormMat * aNormal;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)SRC";

    const char* fShaderTexLightSrc =
R"SRC(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec3 vNormal;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;

    vec3 normal = vNormal;
    if (!gl_FrontFacing)
    {
        normal = -normal;
    }

    float intensity = max(0.0, dot(normal, uLightPos.xyz));

    vec4 color = vec4(vec3(intensity), 1.0) * uLightDiffuse + uLightAmbient;
    color.r = clamp(color.r, 0.0, 1.0);
    color.g = clamp(color.g, 0.0, 1.0);
    color.b = clamp(color.b, 0.0, 1.0);
    color.a = 1.0;

    gl_FragColor = texture2D( uTex0, vTex ) * color;

    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)SRC";

    // ///////////////////

    const char* vShaderCarTopSrc =
R"SRC(
uniform mat4 uModelViewMat;
uniform mat4 uProjModelViewMat;
uniform mat4 uTexMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTex;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    vTex = aTex;
    vNormal = uNormMat * aNormal;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
    vec3 eyePos = vec3(uModelViewMat * vec4(aPos, 1.0));
    vCubeRay = normalize(mat3(uTexMat) * reflect(eyePos, vNormal));
}
)SRC";

    const char* fShaderCarTopSrc =
R"SRC(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uHalftone;

uniform sampler2D uTex0;
uniform samplerCube uCube1;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    vec4 texColor = texture2D( uTex0, vTex );
    if (texColor.a < 0.5)
        discard;
    if (!gl_FrontFacing)
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, texColor.a);
    }
    else
    {
        vec4 envColor = textureCube( uCube1, vCubeRay );
        vec3 normal = vNormal;
        float intensity = max(0.0, dot(normal, uLightPos.xyz));
        vec4 color = vec4(vec3(intensity), 1.0) * uLightDiffuse + uLightAmbient;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        color.a = 1.0;
        gl_FragColor = texColor * color + vec4(envColor.rgb, 0.0);
    }
}
)SRC";

    // ///////////////////

    const char* vShaderCarSrc =
R"SRC(
uniform mat4 uModelViewMat;
uniform mat4 uProjModelViewMat;
uniform mat4 uTexMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTex;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    vTex = aTex;
    vNormal = uNormMat * aNormal;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
    vec3 eyePos = vec3(uModelViewMat * vec4(aPos, 1.0));
    vCubeRay = normalize(mat3(uTexMat) * reflect(eyePos, vNormal));
}
)SRC";

    const char* fShaderCarSrc =
R"SRC(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uHalftone;

uniform sampler2D uTex0;
uniform samplerCube uCube1;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    vec4 texColor = texture2D( uTex0, vTex );
    vec4 envColor = textureCube( uCube1, vCubeRay );
    vec3 normal = vNormal;
    float intensity = max(0.0, dot(normal, uLightPos.xyz));
    vec4 color = vec4(vec3(intensity), 1.0) * uLightDiffuse + uLightAmbient;
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    color.a = 1.0;
    gl_FragColor = texColor * color + vec4(envColor.rgb, 0.0);
}
)SRC";

    // ///////////////////

    const char* vShaderGlassTintSrc =
R"SRC(
uniform mat4 uProjModelViewMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec2 aTex;

varying vec2 vTex;

void main()
{
    vTex = aTex;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)SRC";

    const char* fShaderGlassTintSrc =
R"SRC(
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    vec4 texColor = texture2D( uTex0, vTex );
    if (texColor.a >= 0.5)
        discard;
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.6);
}
)SRC";

    // ///////////////////


    const char* vShaderGlassReflectionSrc =
R"SRC(
uniform mat4 uModelViewMat;
uniform mat4 uProjModelViewMat;
uniform mat4 uTexMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTex;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    vTex = aTex;
    vNormal = uNormMat * aNormal;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
    vec3 eyePos = vec3(uModelViewMat * vec4(aPos, 1.0));
    vCubeRay = normalize(mat3(uTexMat) * reflect(eyePos, vNormal));
}
)SRC";

    const char* fShaderGlassReflectionSrc =
R"SRC(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uHalftone;

uniform sampler2D uTex0;
uniform samplerCube uCube1;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    vec4 texColor = texture2D( uTex0, vTex );
    if (texColor.a >= 0.5)
        discard;
    gl_FragColor = textureCube( uCube1, vCubeRay );
}
)SRC";

    // ///////////////////



    for (int i = 0; i != (int)ShaderId::Count; ++i)
    {
        std::string vSrc;
        std::string fSrc;

        shaders[i].program = 0;

        ShaderId id = (ShaderId)i;
        switch (id)
        {
        case ShaderId::ColorTex:
            vSrc = vShaderTexColorSrc;
            fSrc = fShaderTexColorSrc;
            break;
        case ShaderId::Color:
            vSrc = vShaderColorSrc;
            fSrc = fShaderColorSrc;
            break;
        case ShaderId::LightTex:
            vSrc = vShaderTexLightSrc;
            fSrc = fShaderTexLightSrc;
            break;
        case ShaderId::Tex:
            vSrc = vShaderTexSrc;
            fSrc = fShaderTexSrc;
            break;
        case ShaderId::CarTop:
            vSrc = vShaderCarTopSrc;
            fSrc = fShaderCarTopSrc;
            break;
        case ShaderId::Car:
            vSrc = vShaderCarSrc;
            fSrc = fShaderCarSrc;
            break;
        case ShaderId::GlassTint:
            vSrc = vShaderGlassTintSrc;
            fSrc = fShaderGlassTintSrc;
            break;
        case ShaderId::GlassReflection:
            vSrc = vShaderGlassReflectionSrc;
            fSrc = fShaderGlassReflectionSrc;
            break;
        default:
            break;
        }

        if (!vSrc.empty() && !fSrc.empty())
        {
#if USE_GL_ES2
            fSrc = "precision mediump float;\n"+fSrc;
#else
            static const char* version120line = "#version 120\n";
            vSrc = version120line+vSrc;
            fSrc = version120line+fSrc;
#endif
            GLuint programObject = glCreateProgram();

            glBindAttribLocation(programObject, (GLuint)ShaderAttrib::Pos, "aPos");
            glBindAttribLocation(programObject, (GLuint)ShaderAttrib::Color, "aColor");
            glBindAttribLocation(programObject, (GLuint)ShaderAttrib::Tex, "aTex");
            glBindAttribLocation(programObject, (GLuint)ShaderAttrib::Normal, "aNormal");

            GLuint vId = loadShader(GL_VERTEX_SHADER, vSrc.c_str());
            GLuint fId = loadShader(GL_FRAGMENT_SHADER, fSrc.c_str());

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

                shaders[i].mat4locs[(int)ShaderUniMat4::ModelViewMat] = glGetUniformLocation(programObject, "uModelViewMat");
                shaders[i].mat4locs[(int)ShaderUniMat4::ProjMat] = glGetUniformLocation(programObject, "uProjMat");
                shaders[i].mat4locs[(int)ShaderUniMat4::TexMat] = glGetUniformLocation(programObject, "uTexMat");

                shaders[i].pmvloc = glGetUniformLocation(programObject, "uProjModelViewMat");
                shaders[i].normloc = glGetUniformLocation(programObject, "uNormMat");

                shaders[i].vec4locs[(int)ShaderUniVec4::LightPos] = glGetUniformLocation(programObject, "uLightPos");
                shaders[i].vec4locs[(int)ShaderUniVec4::LightAmbient] = glGetUniformLocation(programObject, "uLightAmbient");
                shaders[i].vec4locs[(int)ShaderUniVec4::LightDiffuse] = glGetUniformLocation(programObject, "uLightDiffuse");

                shaders[i].intlocs[(int)ShaderUniInt::AlphaDiscard] = glGetUniformLocation(programObject, "uAlphaDiscard");
                shaders[i].intlocs[(int)ShaderUniInt::Halftone] = glGetUniformLocation(programObject, "uHalftone");

                glUseProgram(programObject);

                glUniform1i(glGetUniformLocation(programObject, "uTex0"), 0);
                glUniform1i(glGetUniformLocation(programObject, "uCube1"), 1);

                shaders[i].program = programObject;
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
