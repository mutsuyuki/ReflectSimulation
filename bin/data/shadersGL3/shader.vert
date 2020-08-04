#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;

out vec2 texCoordToFlag;

void main()
{
    texCoordToFlag = texcoord;
	  gl_Position = modelViewProjectionMatrix * position;
}
