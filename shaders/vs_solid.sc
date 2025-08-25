\
// Simple solid vertex shader
$input a_position, a_normal, a_texcoord0
$output v_normal, v_uv

#include "common.sh"

uniform mat4 u_mvp;

void main()
{
    gl_Position = mul(u_mvp, vec4(a_position, 1.0));
    v_normal = a_normal;
    v_uv = a_texcoord0;
}
