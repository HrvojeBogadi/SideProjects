//
// Created by Hrvoje on 21.1.2021..
// Header file containing all function and structure definitions needed to run LV6 code
//

#ifndef LV6_FUNCTIONS_H
#define LV6_FUNCTIONS_H

#include "opencv2/opencv.hpp"
#include <stdlib.h>
using namespace cv;
using namespace std;

struct RV3DPOINT
{
    int u, v, d;
};

void ReadKinectPic(string pathRGB, Mat* depthImage, vector<RV3DPOINT>* point3DArray, unsigned long& n3DPoints);
vector<vector<RV3DPOINT>> getAllPlanes(vector<RV3DPOINT>& R, unsigned long& n3DPoints, unsigned long nIterations, float error);
long getRandPoistion(unsigned long upperBound);
bool operator==(const RV3DPOINT point1, const RV3DPOINT point2);

#endif //LV6_FUNCTIONS_H
