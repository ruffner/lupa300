#version 330 core

uniform sampler2D qt_texture;   // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
in vec2 qt_coordinate;

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    qt_fragColor = texture(qt_texture, qt_coordinate, 0).rrra;
    qt_fragColor.rgb = 64.0 * qt_fragColor.rgb;
}
