\
// Simple solid fragment shader
$input v_normal, v_uv

#include "common.sh"

void main()
{
    // simple lambert-ish based on normal.z just for some variation
    float shade = 0.4 + 0.6 * abs(normalize(v_normal).z);
    gl_FragColor = vec4(shade, 0.6 * shade, 0.9 * shade, 1.0);
}
