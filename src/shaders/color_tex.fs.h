fs=R"GLSL(
uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
    if (uHalftone != 0 && mod(mod(gl_FragCoord.x, 2.0)+mod(gl_FragCoord.y, 2.0)+0.5, 2.0) < 1.0)
        discard;
    gl_FragColor = texture2D( uTex0, vTex ) * vColor;
    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)GLSL";
