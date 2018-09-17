#version 330 core

uniform sampler2D qt_texture;         // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES
uniform     float qt_maximum = 1.0f;  // USED TO RESCALE INCOMING VIDEO TO RANGE UP TO 1.0

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    qt_fragColor = vec4(texelFetch(qt_texture, ivec2(gl_FragCoord.xy), 0).rrr / qt_maximum, 1.0);
}
