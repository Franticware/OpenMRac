vs=R"GLSL(
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
)GLSL";
