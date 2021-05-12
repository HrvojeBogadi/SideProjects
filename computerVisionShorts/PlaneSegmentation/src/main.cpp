//
// Created by Hrvoje Bogadi on 21.1.2021..
//

#include "functions.h"

int main(){
    unsigned long nIterations = 500;
    int error = 1;

    vector<RV3DPOINT> points;
    vector<vector<RV3DPOINT>> allPlanes;
    Mat image;
    unsigned long nPoints;

    String pathToImage = "D:\\Development\\RGiRV\\LV6\\KinectPics\\sl-00242.bmp";

    RNG rng(12345);

    image = imread(pathToImage, IMREAD_COLOR);
    imshow("Original image", image);
    waitKey(15);
    Mat imgCopy = image.clone();

    ReadKinectPic(pathToImage, &imgCopy, &points, nPoints);

    allPlanes = getAllPlanes(points, nPoints, nIterations, error);

    Scalar color = Scalar(rng.uniform(0, 255),rng.uniform(0, 255), rng.uniform(0, 255));

    /* SHOW ALL RECOGNISED PLANES
    
    for(int i = 0; i < allPlanes.size(); i++){
        Scalar color = Scalar(rng.uniform(0, 255),rng.uniform(0, 255), rng.uniform(0, 255));
        for(int j = 0; j < allPlanes.at(i).size(); j++){
            circle(image, Point(allPlanes.at(i).at(j).u, allPlanes.at(i).at(j).v), 0, color, 1);
        }
    }
    
    */

    for(int j = 0; j < allPlanes.at(0).size(); j++) {
        circle(image, Point(allPlanes.at(0).at(j).u, allPlanes.at(0).at(j).v), 0, color, 1);
    }


    imshow("Found Planes", image);
    waitKey(0);

    destroyAllWindows();
    return 0;
}


