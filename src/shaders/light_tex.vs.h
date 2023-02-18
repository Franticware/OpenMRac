vs=R"GLSL(
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
)GLSL";
