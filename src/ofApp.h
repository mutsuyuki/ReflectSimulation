#pragma once

#include "ofMain.h"
#include "BlurEffect.hpp"
#include "MyHoverCamera.hpp"
#include "LightLine.hpp"
#include "ReflectModel.hpp"
#include "SwayModel.hpp"

class ofApp : public ofBaseApp {
private:

    ofFbo lineFbo;
    BlurEffect blurEffect;
    MyHoverCamera camera;
    ofRectangle cameraViewPort;

    LightLine lightLine;
    ReflectModel reflectModel;
    ofSpherePrimitive sphere;

    vector<SwayModel> positionSways;
    vector<SwayModel> directionSways;

    ofxDatGui* gui;
    float lightRadius = 0.5;
    float mirrorRadius = 3.0;
    float diffuseRate = 1.06;
    float decayRate = 0.995;
    float lightNum = 80;
    float lightMoveDebug = 0.0;

    void setup() {
        ofSetFrameRate(60);
        lineFbo.allocate(ofGetWidth(), ofGetHeight());
        cameraViewPort = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());
        blurEffect.radius = 0;

        init();

        gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
        gui->addSlider("light radius", 0.01, 10)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), lightRadius = e.value, init(), camera.setTangible(true); });
        gui->addSlider("mirror radius", 0.01, 10)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), mirrorRadius = e.value, init(), camera.setTangible(true); });
        gui->addSlider("diffuse rate", 1, 1.2)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), diffuseRate = e.value, init(), camera.setTangible(true); });
        gui->addSlider("decay rate", 0.0, 1.00)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), decayRate = e.value / 50.0 + 0.98, init(), camera.setTangible(true); });
        gui->addToggle("blur", false)->onToggleEvent([&](ofxDatGuiToggleEvent e) { blurEffect.radius = e.checked ? 4.0 : 0.0; });
        gui->addSlider("light num", 10, 1000)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), lightNum = e.value, init(), camera.setTangible(true); });
        gui->addSlider("light move", -0.2, 0.2)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), lightMoveDebug = e.value, camera.setTangible(true); });
        gui->addSlider("1 pos range", 0.0, 3.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), positionSways[0].range = e.value, camera.setTangible(true); });
        gui->addSlider("1 pos speed", 0.0, 1.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), positionSways[0].speed = e.value, camera.setTangible(true); });
        gui->addSlider("1 angle range", 0.0, 0.1)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), directionSways[0].range = e.value, camera.setTangible(true); });
        gui->addSlider("1 angle speed", 0.0, 1.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), directionSways[0].speed = e.value, camera.setTangible(true); });
        gui->addSlider("2 pos range", 0.0, 3.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), positionSways[1].range = e.value, camera.setTangible(true); });
        gui->addSlider("2 pos speed", 0.0, 1.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), positionSways[1].speed = e.value, camera.setTangible(true); });
        gui->addSlider("2 angle range", 0.0, 0.1)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), directionSways[1].range = e.value, camera.setTangible(true); });
        gui->addSlider("2 angle speed", 0.0, 1.0)->onSliderEvent([&](ofxDatGuiSliderEvent e) { camera.setTangible(false), directionSways[1].speed = e.value, camera.setTangible(true); });
    }

    void init() {
        for (int i = 0; i < 3; ++i) {
            positionSways.push_back(SwayModel(0.1, 0.0));
            directionSways.push_back(SwayModel(0.1,0.0));
        }
        reflectModel.setup(lightRadius, mirrorRadius, diffuseRate, lightNum);
        lightLine.setup(decayRate);
    }

    void update() {
        cout << "decay rate " << lightLine.decayRate << endl;
        ofSetWindowTitle(ofToString(ofGetFrameRate()));

        ofVec3f emitterPos = ofVec3f(100, 0, 0);
        ofVec3f emitterDirection = ofVec3f(-1, 0 + lightMoveDebug, 0);

        positionSways[0].update(ofVec3f(0, 0, 0));
        positionSways[1].update(ofVec3f(-1.0 / 3.0 * 100, 2.0 / 3.0 * 100, 2.0 / 3.0 * 100));
        positionSways[2].update(ofVec3f(0, 100, 0));

        directionSways[0].update(ofVec3f(1, 1 , 1));
        directionSways[1].update(ofVec3f(0, -1, 0));
        directionSways[2].update(ofVec3f(1, 1, 0));

        vector<ofVec3f> positions;
        vector<ofVec3f> directions;

        positions.push_back(emitterPos);
        positions.push_back(positionSways[0].getSwayed());
        positions.push_back(positionSways[1].getSwayed());
        positions.push_back(positionSways[2].getSwayed());

        directions.push_back(emitterDirection);
        directions.push_back(directionSways[0].getSwayed());
        directions.push_back(directionSways[1].getSwayed());
        directions.push_back(directionSways[2].getSwayed());

        reflectModel.update(positions, directions);
        lightLine.update(reflectModel.nodeNum, reflectModel.lightPoints, blurEffect.radius > 0);
    };

    void draw() {
        ofBackground(0);

        lineFbo.begin();
        {
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            ofBackground(0);
            camera.begin(ofRectangle(cameraViewPort));
            {
                ofSetColor(255, 255, 255, 80);
                ofSetDepthTest(true);
                lightLine.draw();
                ofSetDepthTest(false);
            }
            camera.end();
        }
        lineFbo.end();

        blurEffect.update(lineFbo);
        ofSetColor(255, 255, 255, 255);
        blurEffect.draw();

        camera.begin(ofRectangle(cameraViewPort));
        {
            ofDrawAxis(100);
            ofDrawBitmapString("x", 50, 0, 0);
            ofDrawBitmapString("y", 0, 50, 0);
            ofDrawBitmapString("z", 0, 0, 50);

            ofSetColor(0, 255, 255, 80);
            for (int i = 0; i < reflectModel.nodeNum; ++i) {
                float radius = i == 0 ? lightRadius : mirrorRadius;
                sphere.set(radius, 10);
                sphere.setPosition(ofVec3f::zero());
                sphere.setScale(1, 1, 0.0001);
                sphere.lookAt(reflectModel.nodePoints[i].direction);
                sphere.setPosition(reflectModel.nodePoints[i].position);
                sphere.draw();
            }

//            ofSetColor(255, 255, 255, 80);
//            lightLine.draw();
        }
        camera.end();

        gui->draw();
    };

    void keyPressed(int key) {
        camera.setTangible(!camera.tangible);
    }

};
