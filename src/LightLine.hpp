#pragma once
#include "ofMain.h"
#include "ReflectModel.hpp"

class LightLine {
public:
    float decayRate;

private:
    ofShader shader;
    ofVbo vbo;

    int indicesNum;

public:

    void setup(float __decayRate = 0.995) {
        shader.load("shadersGL3/lightDecay");
        decayRate = __decayRate;
    }

    void update(const int __nodeNum,  const vector<vector<LightPoint>>& __lightPoints, bool __plusRandom = false) {
        if (__nodeNum < 2)
            return;

        indicesNum = (__nodeNum - 1) * 2 * __lightPoints[0].size();
        ofVec3f* vertices = new ofVec3f[indicesNum];
        float* distances = new float[indicesNum];
        ofIndexType* indices = new ofIndexType[indicesNum];

        for (int i = 0; i < __nodeNum - 1; ++i) {
            int lineNum = __lightPoints[i].size();
            float currentRandomRange = __lightPoints[i][0].position.distance(__lightPoints[i][1].position);
            float nextRandomRange = __lightPoints[i+1][0].position.distance(__lightPoints[i+1][1].position);
            for (int j = 0; j < lineNum; ++j) {
                int index = (i * lineNum + j) * 2;


                vertices[index].set(__lightPoints[i][j].position);
                vertices[index + 1].set(__lightPoints[i + 1][j].position);
                if (__plusRandom) {
                    vertices[index] += ofVec3f(ofRandom(currentRandomRange) * 2.0 - currentRandomRange, ofRandom(currentRandomRange) * 2.0 - currentRandomRange, ofRandom(currentRandomRange) * 2.0 - currentRandomRange);
                    vertices[index + 1] += ofVec3f(ofRandom(nextRandomRange) * 2.0 - nextRandomRange, ofRandom(nextRandomRange) * 2.0 - nextRandomRange, ofRandom(nextRandomRange) * 2.0 - nextRandomRange);
                }
            
                indices[index] = index;
                indices[index + 1] = index + 1;
                distances[index] = __lightPoints[i][j].distance;
                distances[index + 1] = __lightPoints[i + 1][j].distance;
            }
        }

        if (indicesNum != vbo.getNumVertices()) {
            vbo.clear();
            vbo.setVertexData(vertices, indicesNum, GL_DYNAMIC_DRAW);
            vbo.setIndexData(indices, indicesNum, GL_DYNAMIC_DRAW);
            shader.begin();
            int attributeLocation = shader.getAttributeLocation("lightDistance");
            vbo.setAttributeData(attributeLocation, distances, 1, indicesNum, GL_DYNAMIC_DRAW);
            shader.end();
        } else {
            vbo.updateVertexData(vertices, indicesNum);
            shader.begin();
            int attributeLocation = shader.getAttributeLocation("lightDistance");
            vbo.updateAttributeData(attributeLocation, distances, indicesNum);
            shader.end();
        }

        delete[] vertices;
        delete[] distances;
        delete[] indices;
    }


    void draw() {
        shader.begin();
		shader.setUniform1f("decayRate", decayRate);
        vbo.drawElements(GL_LINES, indicesNum);
        shader.end();

    }
};
