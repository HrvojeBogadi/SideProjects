//
// Created by Hrvoje on 21.1.2021..
//

#include "functions.h"

void ReadKinectPic(string pathRGB, Mat* depthImage, vector<RV3DPOINT>* point3DArray, unsigned long& n3DPoints)
{

    int *DepthMap = new int[depthImage->cols * depthImage->rows];
    memset(DepthMap, 0, depthImage->cols * depthImage->rows*sizeof(int));


    int u, v, d;
    int dmin = 2047;
    int dmax = 0;

    n3DPoints = 0;

    //Get DepthImage file path
    string pathDepth = pathRGB.substr(0, pathRGB.length() - 4);
    pathDepth.append("-D.txt");

    FILE *fp;

    fopen_s(&fp, pathDepth.c_str(), "r");

    if (fp)
    {
        bool bOK = true;

        //Determine max and min depth values and get Depth map
        for (v = 0; v<depthImage->rows; v++)
        {
            for (u = 0; u<depthImage->cols; u++)
            {
                if (!(bOK = (fscanf(fp, "%d ", &d) == 1)))
                    break;


                if (d == 2047)
                {
                    d = -1;
                }
                else
                {


                    //determine min and max d
                    if (d<dmin)
                        dmin = d;

                    if (d>dmax)
                        dmax = d;
                }

                DepthMap[v*depthImage->cols + u] = d;


                if (d != -1)
                {
                    RV3DPOINT pt3;
                    pt3.u = u;
                    pt3.v = v;
                    pt3.d = d;

                    point3DArray->push_back(pt3);

                }


            }
        }

        fclose(fp);
    }


    //get  number of valid 3D points
    n3DPoints = point3DArray->size();

    //Form grayscale pic -> Scale from 1 to 255 (reserve 0 for undefined regions)
    for (v = 0; v<depthImage->rows; v++)
    {
        for (u = 0; u<depthImage->cols; u++)
        {
            d = DepthMap[v*depthImage->cols + u];

            if (d != -1)
                d = ((d - dmin) * 254 / (dmax - dmin)) + 1;
            else
                d = 0;

            ((uchar *)(depthImage->data + v*depthImage->step))[u] = d;

        }
    }

    delete[] DepthMap;
}

long getRandPoistion(unsigned long upperBound){
    return rand() % upperBound;
}

vector<vector<RV3DPOINT>> getAllPlanes(vector<RV3DPOINT>& R, unsigned long& n3DPoints, unsigned long nIterations, float error){
    //Working with a copy of 3D points because of wish to save original points array

    srand (12345);

    int c = 0;
    int i = 0;
    int j = 0;
    RV3DPOINT m1, m2, m3;
    Mat points = Mat(3, 3, CV_32F);
    Mat distance = Mat(3, 1, CV_32F);
    Mat planeParams;
    Mat dominantPlaneParams;
    vector<RV3DPOINT> tempPoints;
    vector<RV3DPOINT> goodPoints;
    float condition;
    vector<vector<RV3DPOINT>> planesPoints;

    while(R.size() > 2){

        c = 0;

        for(i = 0; i < nIterations; i++){
            m1 = R.at(getRandPoistion(R.size()));
            m2 = R.at(getRandPoistion(R.size()));
            m3 = R.at(getRandPoistion(R.size()));

            points.at<float>(0, 0) = m1.u;
            points.at<float>(0, 1) = m1.v;
            points.at<float>(0, 2) = 1;
            points.at<float>(1, 0) = m2.u;
            points.at<float>(1, 1) = m2.v;
            points.at<float>(1, 2) = 1;
            points.at<float>(2, 0) = m3.u;
            points.at<float>(2, 1) = m3.v;
            points.at<float>(2, 2) = 1;

            distance.at<float>(0, 0) = m1.d;
            distance.at<float>(1, 0) = m2.d;
            distance.at<float>(2, 0) = m3.d;

            solve(points, distance, planeParams);

            tempPoints.clear();

            for(j = 0; j < R.size(); j++){
                condition = planeParams.at<float>(0, 0) * R.at(j).u + planeParams.at<float>(1, 0) * R.at(j).v +
                            planeParams.at<float>(2, 0) - R.at(j).d;
                if(abs(condition) <= error){
                    tempPoints.push_back(R.at(j));
                }
            }
            if(tempPoints.size() > c){
                c = tempPoints.size();
                goodPoints = tempPoints;
            }

        }
        planesPoints.push_back(goodPoints);

        vector<RV3DPOINT>::iterator it;
        for(it = goodPoints.begin(); it < goodPoints.end(); it++){
            R.erase(remove(R.begin(), R.end(), *it), R.end());
        }
    }


    return planesPoints;
}

bool operator==(const RV3DPOINT op1, const RV3DPOINT op2){
    if(op1.u == op2.u && op1.v == op2.v){
        return true;
    }
    return false;
}