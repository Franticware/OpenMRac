fs=R"GLSL(
uniform int uHalftone;

uniform sampler2D uTex0;

varying vec2 vTex;
varying vec3 vNormal;
varying vec3 vCubeRay;

void main()
{
    if (uHalftone != 0 && mod(gl_FragCoord.x + gl_FragCoord.y + 0.5, 2.0) < 1.0)
        discard;
    vec4 texColor = texture2D( uTex0, vTex );
    if (texColor.a >= 0.5)
        discard;
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.6);
}
)GLSL";
