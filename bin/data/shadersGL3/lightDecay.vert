#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in float lightDistance;
out float vertex_lightDistance;

void main()
{
    vertex_lightDistance = lightDistance;
	gl_Position = modelViewProjectionMatrix * position;
}
