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
    vec4 envColor = textureCube(uCube, vCubeRay);
    vec3 normal = vNormal;
    float intensity = max(0.0, dot(normal, uLightPos.xyz));
    vec4 color = vec4(vec3(intensity), 1.0) * uLightDiffuse + uLightAmbient;
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    color.a = 1.0;
    gl_FragColor = texColor * color + vec4(envColor.rgb, 0.0);
}
}
)GLSL";
