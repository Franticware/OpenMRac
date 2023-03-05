fs=R"GLSL(
uniform int uAlphaDiscard;
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
#insertHalftoneTest
    gl_FragColor = texture2D( uTex0, vTex ) * vColor;
    if (uAlphaDiscard != 0 && gl_FragColor.a < 0.5)
        discard;
}
)GLSL";
