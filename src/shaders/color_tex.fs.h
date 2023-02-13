fs=R"GLSL(
uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
    //uniform int uHalftone;
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    gl_FragColor = texture2D( uTex0, vTex ) * vColor;
    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)GLSL";
