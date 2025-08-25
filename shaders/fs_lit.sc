\
$input v_worldNormal

#include "common.sh"

uniform vec4 u_lightDir;     // xyz dir (world), w unused
uniform vec4 u_lightColInt;  // rgb color, a intensity

void main()
{
    vec3 N = normalize(v_worldNormal);
    vec3 L = normalize(-u_lightDir.xyz);
    float NdotL = max(dot(N, L), 0.0);
    vec3 col = u_lightColInt.rgb * u_lightColInt.a * (0.1 + 0.9 * NdotL);
    gl_FragColor = vec4(col, 1.0);
}
