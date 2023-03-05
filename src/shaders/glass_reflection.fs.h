fs=R"GLSL(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uHalftone;

uniform sampler2D uTex0;
uniform samplerCube uCube;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
#insertHalftoneTest
    vec4 texColor = texture2D(uTex0, vTex);
    if (texColor.a >= 0.5)
        discard;
    gl_FragColor = textureCube(uCube, vCubeRay);
}
)GLSL";
