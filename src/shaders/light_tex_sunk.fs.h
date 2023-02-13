fs=R"GLSL(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;
uniform sampler2D uTex1;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vTan;
varying vec3 vBitan;
varying vec3 vEyePos;
varying mat3 vTBN;

void main()
{
if (uHalftone != 0 && mod(gl_FragCoord.x+gl_FragCoord.y+0.5, 2.0) < 1.0)
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
vec4 texColor = vec4(1.0, 0.0, 1.0, 1.0); // debug magenta
vec4 colorSunk = texture2D(uTex1, vTex);
if (colorSunk.a != 0.0)
{
    vec3 eyeDir = normalize(vEyePos);
    vec3 tbnTrans = vTBN * eyeDir;
    vec2 offset = tbnTrans.xy / tbnTrans.z * (colorSunk.a * 0.04);
    vec2 texc = vTex - offset;
    float sunk = texture2D(uTex1, texc).a;
    if (sunk != 0.0)
    {
        texColor = texture2D(uTex0, texc);
    }
    else
    {
        texColor = colorSunk;
    }
}
else
{
    texColor = texture2D(uTex0, vTex);
}
gl_FragColor = texColor * color;
if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
    discard;
}
)GLSL";
