vs=R"GLSL(
uniform mat4 uProjModelViewMat;

attribute vec3 aPos;
attribute vec2 aTex;

varying vec2 vTex;

void main()
{
    vTex = aTex;
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)GLSL";
