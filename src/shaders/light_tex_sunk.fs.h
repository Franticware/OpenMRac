fs=R"GLSL(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

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
    vec4 texColorTop = texture2D(uTex0, vTex);
    vec4 colorSunk = texture2D(uTex1, vTex);
    vec3 eyeDir = normalize(vEyePos);
    vec3 tbnTrans = vTBN * eyeDir;
    vec2 offset = tbnTrans.xy / tbnTrans.z * (colorSunk.a * 0.04);
    vec2 texc = vTex - offset;
    float hiA = colorSunk.a;
    if (hiA == 0.0) hiA = 1.0;
    float frameMid = texture2D(uTex1, vTex - offset * 0.5).a;
    vec4 texColorFrame = texture2D(uTex1, texc);
    float frameA = min(texColorFrame.a, frameMid);
    float hiFrameA = frameA;
    if (hiFrameA == 0.0) hiFrameA = 1.0;
    vec4 texColorBtm = mix(colorSunk, texture2D(uTex0, texc), frameA/hiFrameA);
    gl_FragColor = mix(texColorTop, texColorBtm, colorSunk.a/hiA) * color;
}
)GLSL";
