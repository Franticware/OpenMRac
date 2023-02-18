vs=R"GLSL(
uniform mat4 uModelViewMat;
uniform mat4 uProjModelViewMat;
uniform mat3 uNormMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec3 aTan;
attribute vec3 aBitan;
attribute vec2 aTex;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vTan;
varying vec3 vBitan;
varying vec3 vEyePos;
varying mat3 vTBN;

void main()
{
    vTex = aTex;
    vNormal = uNormMat * aNormal;
    vTan = uNormMat * aTan;
    vBitan = uNormMat * aBitan;
    vTBN[0][0] = vTan.x;    vTBN[1][0] = vTan.y;    vTBN[2][0] = vTan.z;
    vTBN[0][1] = vBitan.x;  vTBN[1][1] = vBitan.y;  vTBN[2][1] = vBitan.z;
    vTBN[0][2] = vNormal.x; vTBN[1][2] = vNormal.y; vTBN[2][2] = vNormal.z;
    vEyePos = vec3(uModelViewMat * vec4(aPos, 1.0));
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)GLSL";
