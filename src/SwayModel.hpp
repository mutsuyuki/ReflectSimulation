#pragma once

#include "ofMain.h"

class SwayModel {
public:
    float speed;
    float range;

private:
    ofVec3f offsetDirection;
    ofVec3f offsetCycle;
    ofVec3f moveAmount;
    ofVec3f offset;
    ofVec3f current;
    ofVec3f swayed;


public:

    SwayModel(float __speed = 0, float __range = 0) {
        speed = __speed;
        range = __range;
        offsetDirection.set(ofRandom(2.0) - 1.0, ofRandom(2.0) - 1.0, ofRandom(2.0) - 1.0);
    }

    void update() {
        update(current);
    }

    void update(const ofVec3f& __current ) {
        current.set(__current);

        offsetCycle += offsetDirection * speed;
        offset.set(sin(offsetCycle.x), sin(offsetCycle.y), sin(offsetCycle.z));
        offset *= range;
        swayed.set(current + offset);
    }

    ofVec3f getSwayed() {
        return swayed;
    }

};
