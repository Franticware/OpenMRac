fs=R"GLSL(
uniform sampler2D uTex0;

varying vec2 vTex;

void main()
{
    gl_FragColor = texture2D( uTex0, vTex );
}
)GLSL";
