//
// Created by Hrvoje Bogadi on 14/01/2021.
//
#include <vector>
#include <math.h>

#include <iostream>
#include <cstdio>


#include <opencv2/opencv.hpp>

#define ESC_KEY 27

using namespace std;
using namespace cv;

void onClick(int event, int x, int y, int flags, void* userData);

struct userCallbackData{
    vector<Point> points;
};


int main(){
    Mat cameraMatrixLoaded, distCoeffsLoaded;
    FileStorage fsL("/mnt/4DA028260B3CDAA2/Development/RGiRV/LV3/src/CameraCalib/cameraparams.xml", FileStorage::READ);
    fsL["camera_matrix"] >> cameraMatrixLoaded;
    fsL["distortion_coefficients"] >> distCoeffsLoaded;
    fsL.release();

    VideoCapture cap(0);


    if (cap.isOpened())
    {
        int c = 0;

        cout << "\nPress c to capture your image and start Hough Transform" << endl;

        for (;;)
        {
            c = waitKey(15);

            Mat frame, imgclone, imgUndistort;

            cap >> frame;

            imgclone = frame.clone();

            //Undistort
            undistort(imgclone, imgUndistort, cameraMatrixLoaded, distCoeffsLoaded);

            imshow("Undistorted view", imgUndistort);

            if (c == ESC_KEY) break;
            if (c == 'c'){
                userCallbackData uData;
                vector<Point> points;
                int c;

                imshow("Undistorted view", imgUndistort);

                setMouseCallback("Undistorted view", onClick, &uData);

                cout << "\nSelect 4 points for region of interest" << endl;


                while(1) {
                    c = waitKey(15);

                    if (uData.points.size() == 4) {
                        setMouseCallback("Undistorted view", NULL, NULL);
                        break;
                    }
                }

                int width, height;
                width = uData.points[3].x - uData.points[0].x;
                height = uData.points[2].y - uData.points[0].y;
                int u0, v0;
                u0 = uData.points[0].x;
                v0 = uData.points[0].y;

                Mat image = imgUndistort(Rect(Point(u0, v0), Point(u0 + width, v0 + height)));

                imshow("ROI", image);

                cvtColor(image, image, COLOR_BGR2GRAY);

                Mat cannyImg;

                Canny(image, cannyImg, 50, 255, 3);

                vector<Vec2f> lines;
                HoughLines(cannyImg, lines, 1, CV_PI/180, 10, 0, 0, 0, CV_PI);
                if(lines.empty()){
                    cout << "Hough can't find any lines, please try again" << endl;
                    continue;
                }
                cvtColor(cannyImg, cannyImg, COLOR_GRAY2BGR);

                float a, b;
                float rhoHough, thetaHough;
                float x0, y0;

                rhoHough = lines[0][0];
                thetaHough = lines[0][1];

                /**** DRAW THE MOST DOMINANT HOUGH LINE ****/
                Point pt1, pt2;
                a = cos(thetaHough);
                b = sin(thetaHough);
                x0 = a*rhoHough;
                y0 = b*rhoHough;
                pt1.x = cvRound(x0 + 1000*(-b));
                pt1.y = cvRound(y0 + 1000*(a));
                pt2.x = cvRound(x0 - 1000*(-b));
                pt2.y = cvRound(y0 - 1000*(a));
                line( cannyImg, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);


                Mat rvec, tvec;
                vector<Point3f> objPoints;
                vector<Point2f> imgPoints;

                double x1, y1;

                cout << "Please input object size: " << endl;
                cout << "\n \t x = ";
                cin >> x1;
                cout << "\n \t y = ";
                cin >> y1;


                objPoints.push_back(Point3f(0, 0, 0));
                objPoints.push_back(Point3f(0, y1, 0));
                objPoints.push_back(Point3f(x1, y1, 0));
                objPoints.push_back(Point3f(x1, 0, 0));

                Size imageSize = image.size();
                int rows, cols;
                rows = imageSize.height;
                cols = imageSize.width;

                imgPoints.push_back(Point2f(0, 0));
                imgPoints.push_back(Point2f(0, rows));
                imgPoints.push_back(Point2f(cols, rows));
                imgPoints.push_back(Point2f(cols, 0));

                solvePnP(objPoints, imgPoints, cameraMatrixLoaded, distCoeffsLoaded, rvec, tvec);

                Mat rodRvec;
                Rodrigues(rvec, rodRvec);

                Mat AMat, bMat;
                gemm(cameraMatrixLoaded, rodRvec, 1, 0, 0, AMat);
                gemm(cameraMatrixLoaded, tvec, 1, 0, 0, bMat);

                float Lx, Ly, Lp;

                Lx = AMat.at<double>(0, 0) *  a + AMat.at<double>(1, 0) * b - rhoHough*AMat.at<double>(2, 0);
                Ly = AMat.at<double>(0, 1) *  a + AMat.at<double>(1, 1) * b - rhoHough*AMat.at<double>(2, 1);
                Lp = bMat.at<double>(2) * rhoHough - bMat.at<double>(0) * a - bMat.at<double>(1) * b;

                float thetaCam, rhoCam;
                thetaCam = atan2(Ly, Lx);
                rhoCam = Lp / (sqrt((Lx*Lx) + (Ly*Ly)));

                cout << "\nRho Image = " << rhoHough << endl;
                thetaHough = thetaHough * (180.00 / CV_PI);
                cout << "Theta Image = " << thetaHough << endl;

                thetaCam = thetaCam * (180.00 / CV_PI);
                cout << "ThetaCam = " << thetaCam << endl;
                cout << "RhoCam = " << rhoCam << endl;

                imshow("Hough Line", cannyImg);

            }
        }
    }

    destroyAllWindows();

    return 0;
}

void onClick(int event, int x, int y, int flags, void* userData){
    userCallbackData *uData = (userCallbackData*)userData;

    if (event == EVENT_LBUTTONDOWN) {
        uData->points.push_back(Point(x, y));
        cout << "Added point: " << Point(x, y) << endl;
    }
}