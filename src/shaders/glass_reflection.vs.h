vs=R"GLSL(
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
)GLSL";
