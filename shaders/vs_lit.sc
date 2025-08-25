\
$input a_position, a_normal, a_texcoord0
$output v_worldNormal

#include "common.sh"

uniform mat4 u_mvp;

void main()
{
    gl_Position = mul(u_mvp, vec4(a_position, 1.0));
    v_worldNormal = a_normal;
}
