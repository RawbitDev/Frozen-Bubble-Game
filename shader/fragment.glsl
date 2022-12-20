#version 300 es
precision mediump float;
uniform vec4 Color;
uniform sampler2D tex;
uniform sampler2D tex2;
in vec2 TexCoord;
out vec4 color;
void main(void) {
    //gl_FragColor = vec4(TexCoord,0.0f,1.0f);
    //gl_FragColor = texture2D(tex,TexCoord);
    //vec4 texel0 = texture2D(tex, TexCoord);
    //vec4 texel1 = texture2D(tex2, TexCoord);

    //color = texel0 * texel1;
    color = texture2D(tex, TexCoord);
}