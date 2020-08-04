#version 150

uniform float decayRate;

in float vertex_lightDistance;
out vec4 outputColor;

void main()
{
    float bright = pow(decayRate, vertex_lightDistance);
    outputColor = vec4(bright, bright, bright, 1.0);
}
