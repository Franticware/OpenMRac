fs=R"GLSL(
uniform vec4 uLightPos;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec3 vNormal;

void main()
{
    if (uHalftone != 0 && mod(mod(gl_FragCoord.x, 2.0)+mod(gl_FragCoord.y, 2.0)+0.5, 2.0) < 1.0)
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
    gl_FragColor = texture2D( uTex0, vTex ) * color;
    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)GLSL";
