#pragma once
#include "ofxDatGui.h"

class BlurEffect {
public:
	int radius = 5;
	float scale = 2.5;

private :
	ofShader shader;
	ofFbo tmpFbo;
	ofFbo finalFbo;

	ofxDatGui* gui;


public:

	BlurEffect() {
		shader.load("shadersGL3/shader");

		gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
		gui->addSlider("radius", 1, 50)->onSliderEvent([&](ofxDatGuiSliderEvent e) { radius = e.value; });
		gui->addSlider("scale", 1, 20)->onSliderEvent([&](ofxDatGuiSliderEvent e) { scale = e.value; });
	}

	void update(const ofFbo& __fbo) {

		if (__fbo.getWidth() != tmpFbo.getWidth() || __fbo.getHeight() != tmpFbo.getHeight()) {
			tmpFbo.allocate(__fbo.getWidth(), __fbo.getHeight());
			finalFbo.allocate(__fbo.getWidth(), __fbo.getHeight());
		}

		ofColor(0, 0, 0, 255);

		// 横方向
		tmpFbo.begin();
		{
			shader.begin();
			{
				shader.setUniformTexture("_texture", __fbo.getTextureReference(), 1);
				shader.setUniform1f("radius", radius);
				shader.setUniform1f("scale", scale);
				shader.setUniform2f("direction", 1, 0);
				__fbo.draw(0, 0);
			}
			shader.end();
		}
		tmpFbo.end();

		// 全方位
		finalFbo.begin();
		{
			shader.begin();
			{
				shader.setUniformTexture("texture", tmpFbo.getTextureReference(), 1);
				shader.setUniform1f("radius", radius);
				shader.setUniform1f("scale", scale);
				shader.setUniform2f("direction", 0, 1);
				tmpFbo.draw(0, 0);
			}
			shader.end();
		}
		finalFbo.end();

	}

	void draw() {
		finalFbo.draw(0, 0);
	}
};
