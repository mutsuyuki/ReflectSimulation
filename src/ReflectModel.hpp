#pragma once
#include "ofMain.h"
#include "ofxRay.h"

// ライト、ミラーの位置や向きを格納
struct NodePoint {
    ofVec3f position;
    ofVec3f direction;
};

// 光線の断面上の各点の位置や向き
struct LightPoint {
    bool isAlive; //有効な光点か
    ofVec3f position;
    ofVec3f direction;
    float distance;
};

class ReflectModel {
public:

    float mirrorRadius; // 光の半径
    float lightRadius; // 光の半径
    float diffuseRate; // 1mあたりの光の拡散の割合
    int gridTotalNum;
    int gridSideNum;
    float gridDistance;

    int nodeNum;
    vector<NodePoint> nodePoints;
    vector<vector<LightPoint>> lightPoints;
    vector<LightPoint> tempLightPoints;

private:
    ofPlane reflectPlane;
    ofRay lightRay;
    ofVec3f reflectPosition;


public:
    void setup(float __lightRadius = 0.04, float __mirrorRadius = 0.06, float __diffuseRate = 1.001, int __lineNum = 80) {
        lightRadius = __lightRadius;
        mirrorRadius = __mirrorRadius;
        diffuseRate = __diffuseRate;

        gridTotalNum = __lineNum * (1.0 / (pow(0.5, 2.0) * PI));
        gridSideNum = sqrt(gridTotalNum);
        gridDistance = (__lightRadius * 2.0) / (float)gridSideNum;

        // 9反射分メモリを確保しておく
        nodePoints.clear();
        lightPoints.clear();
        allocPoints(10);

        // 作業用エリアを確保しておく
        tempLightPoints.clear();
        for (int j = 0; j < gridTotalNum; ++j) {
            tempLightPoints.push_back(LightPoint());
        }
    }

    void update(const vector<ofVec3f>& __positions, const vector<ofVec3f>& __directions) {
        nodeNum = __positions.size();

        for (int i = 0; i < nodeNum; ++i) {
            if (i >= nodePoints.size())
                addPoints();

            nodePoints[i].position.set(__positions[i]);
            nodePoints[i].direction.set(__directions[i].getNormalized());
        }

        updateFirstPositions();
        for (int i = 1; i < nodeNum; ++i) {
            updateReflectPositions(i);
        }

    }

private:
    void allocPoints(int __max) {
        int currentNodeSize = nodePoints.size();
        for (int i = currentNodeSize; i < __max; ++i) {
            nodePoints.push_back(NodePoint());
        }

        int currentLightsSize = lightPoints.size();
        for (int i = currentLightsSize; i < __max; ++i) {
            lightPoints.push_back(vector<LightPoint>());
            for (int j = 0; j < gridTotalNum; ++j) {
                lightPoints[i].push_back(LightPoint());
            }
        }
    }

    void addPoints() {
        allocPoints(nodePoints.size() + 1);
    }

    // 光源の位置を保持する
    void updateFirstPositions() {
        const ofVec3f& nodeDirection = nodePoints[0].direction;

        // 光線の向きを法線とする面の基底ベクトルを求める
        float absDirectionX = abs(nodeDirection.x);
        float absDirectionY = abs(nodeDirection.y);
        float absDirectionZ = abs(nodeDirection.z);
        float maxLength = max(max(absDirectionX, absDirectionY), absDirectionZ);
        float x = (maxLength != absDirectionX) ? 1 : -(nodeDirection.y + nodeDirection.z) / nodeDirection.x;
        float y = (maxLength != absDirectionY) ? 1 : -(nodeDirection.x + nodeDirection.z) / nodeDirection.y;
        float z = (maxLength != absDirectionZ) ? 1 : -(nodeDirection.x + nodeDirection.y) / nodeDirection.z;
        ofVec3f grid1Vec = (ofVec3f(x, y, z)).normalize() * gridDistance;
        ofVec3f grid2Vec = (ofVec3f(grid1Vec).cross(nodeDirection)).normalize() * gridDistance;

        // 光線の発射位置を求める
        const ofVec3f& nodePosition = nodePoints[0].position;
        ofVec3f gridPoint;
        for (int i = 0; i < gridSideNum; ++i) {
            for (int j = 0; j < gridSideNum; ++j) {
                gridPoint.set(nodePosition + (grid1Vec * i) + (grid2Vec * j) - (grid1Vec + grid2Vec) * ((gridSideNum - 1.0) / 2.0));
                int index = i * gridSideNum + j;
                lightPoints[0][index].isAlive = (nodePosition.distance(gridPoint) < lightRadius);
                lightPoints[0][index].position.set(gridPoint);
            }
        }

        // 光線の向きを求める
        const ofVec3f baseDirectionPosition = nodePoints[0].position + nodePoints[0].direction;
        for (int i = 0; i < lightPoints[0].size(); ++i) {
            if (!lightPoints[0][i].isAlive)
                continue;

            // 拡散を考慮した各光点の向きを計算
            tempLightPoints[i].position.set(lightPoints[0][i].position);
            tempLightPoints[i].position += nodePoints[0].direction;
            tempLightPoints[i].position += ((tempLightPoints[i].position - baseDirectionPosition) * (diffuseRate - 1.0));

            // 光源の光点→ミラー周りの光点を方向にする
            lightPoints[0][i].direction.set(tempLightPoints[i].position - lightPoints[0][i].position);
            lightPoints[0][i].direction.normalize();

            // 光の距離を初期化
            lightPoints[0][i].distance = 0.0;
        }

    }

    // 光源の位置を設定
    void updateReflectPositions(int __index) {
        int prevIndex = __index - 1;
        int currentIndex = __index;

        reflectPlane.setCenter(nodePoints[currentIndex].position);
        reflectPlane.setNormal(nodePoints[currentIndex].direction);
        reflectPlane.setInfinite(true);

        // 反射点を求める
        const NodePoint& currentNode = nodePoints[currentIndex];
        LightPoint& prevLightPoint = LightPoint();
        ofVec3f reflectDirection;
        for (int i = 0; i < lightPoints[prevIndex].size(); ++i) {

            prevLightPoint = lightPoints[prevIndex][i];

            // ひとつ前のノードで反射したもののみ対象
            if (prevLightPoint.isAlive) {
                // 反射点を求める
                lightRay.setStart(prevLightPoint.position);
                lightRay.setEnd(prevLightPoint.position + prevLightPoint.direction);
                reflectPlane.intersect(lightRay, reflectPosition);

                // ミラーの領域に交点があれば反射
                bool isFace = currentNode.direction.dot(prevLightPoint.direction) < 0;
                bool isReflect = isFace && (currentNode.position.distance(reflectPosition) < mirrorRadius);

                if (isReflect) {
                    // 反射点と反射の向きを設定
                    lightPoints[currentIndex][i].isAlive = true;
                    lightPoints[currentIndex][i].position.set(reflectPosition);
                    reflectDirection.set(2.0 * currentNode.direction * currentNode.direction.dot(-prevLightPoint.direction) + prevLightPoint.direction);
                    lightPoints[currentIndex][i].direction.set(reflectDirection);
                    lightPoints[currentIndex][i].direction.normalize();

                } else {
                    // 反射しない場合は遠くまで飛ばす
                    lightPoints[currentIndex][i].isAlive = false;
                    lightPoints[currentIndex][i].position.set(prevLightPoint.position + (prevLightPoint.direction * 5000.0));
                }

                // 光の距離を設定
                float sectionDistance = lightPoints[currentIndex][i].position.distance(lightPoints[prevIndex][i].position);
                lightPoints[currentIndex][i].distance = prevLightPoint.distance + sectionDistance;
            } else {
                lightPoints[currentIndex][i].isAlive = false;
                lightPoints[currentIndex][i].position.set(prevLightPoint.position);
            }

        }
    }


};
