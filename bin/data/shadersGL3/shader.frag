#version 150

uniform sampler2DRect _texture;
uniform float radius;
uniform float scale;
uniform vec2 direction;

in vec2 texCoordToFlag;
out vec4 outputColor;

const float halfPI = 1.570796326794896;

void main()
{
    float weightTotal = 1.0;
    for(float i = 1.0; i < radius; i++){
      weightTotal += cos( 0.8 * halfPI * i / radius) * 2;
    }

  	float x = texCoordToFlag.x;
  	float y = texCoordToFlag.y;

    vec4 result = texture(_texture, vec2(x, y)) * (1.0 / weightTotal);
    for(float i = 1.0; i < radius; i++){
      float weight = cos( 0.8 * halfPI * i / radius) / weightTotal;
      vec2 offset = i * scale * direction;

      result += texture(_texture, vec2(x, y) + offset) * weight;
      result += texture(_texture, vec2(x, y) - offset) * weight;
    }

    outputColor = result;

}
