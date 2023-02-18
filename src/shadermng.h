#ifndef SHADERMNG_H
#define SHADERMNG_H

#include "gl1.h"
#include "glm1.h"

enum class ShaderId {
    None = -1,
    Color = 0,
    Tex,
    ColorTex, // uses AlphaDiscard
    LightTex, // uses AlphaDiscard
    LightTexSunk, // uses AlphaDiscard
    Car, // never alpha discard
    CarTop, // always alpha discard
    GlassTint,
    GlassReflection,
    Count
};

enum class ShaderAttrib {
    Pos,
    Color,
    Tex,
    Normal,
    Tan,
    Bitan,
    Count
};

enum class ShaderUniMat4 {
    ProjMat,
    ModelViewMat,
    TexMat,
    Count
};

enum class ShaderUniVec4 {
    LightPos,
    LightAmbient,
    LightDiffuse,
    Count
};

enum class ShaderUniInt {
    AlphaDiscard,
    Halftone,
    Count
};

enum class ShaderUniTex {
    Tex0,
    Tex1,
    Cube,
    Count
};

class ShaderWrap
{
public:
    GLuint program;
    GLint mat4locs[(int)ShaderUniMat4::Count];
    GLint pmvloc;
    GLint normloc;
    GLint vec4locs[(int)ShaderUniVec4::Count];
    GLint intlocs[(int)ShaderUniInt::Count];
};

class ShaderMng
{
public:
    ShaderMng();

    void init();
    void use(ShaderId id);
    void set(ShaderUniMat4 id, glm::mat4 m);
    void set(ShaderUniVec4 id, glm::vec4 v);
    void set(ShaderUniInt id, GLint i);

    ShaderId currentShader;

    ShaderWrap shaders[(int)ShaderId::Count];
    glm::mat4 mat4s[(int)ShaderUniMat4::Count];
    glm::mat4 mat4pmv;
    glm::mat3 mat3norm;
    glm::vec4 vec4s[(int)ShaderUniVec4::Count];
    GLint ints[(int)ShaderUniInt::Count];
};

#endif // SHADERMNG_H
