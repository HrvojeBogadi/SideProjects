#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;


int main(int argc, char *argv[]) {

    /**************************************************************************/
    /* A MODIFIED VERSION OF THE EXAMPLE PROVIDED IN :                        */
    /* https://docs.opencv.org/3.4.3/d4/d94/tutorial_camera_calibration.html  */
    /**************************************************************************/

    int m_MaxNImages;    //max number of images taken  (5)
    int m_BoardWidth;    //board width (ie the max number of corners that can be found along the chessboard length) (8)
    int m_BoardHeight;    //board height(ie the max number of corners that can be found along the chessboard height) (6)
    float m_SquareLength;    //the length of one of the squares on the chessboard (in mm)  (36)


    cout << "Max number of images to take:";
    cin >> m_MaxNImages;
    cout << "Board width (ie the max number of corners that can be found along the chessboard width):";
    cin >> m_BoardWidth;
    cout << "Board height(ie the max number of corners that can be found along the chessboard height):";
    cin >> m_BoardHeight;
    cout << "Length of one of the squares on the chessboard (in mm):";
    cin >> m_SquareLength;

    Size imageSize;

    vector<Point2f> corners;
    vector<vector<Point2f> > imagePoints;

    Mat cameraMatrix, distCoeffs;

    Size board_sz = Size(m_BoardWidth, m_BoardHeight);


    int successes = 0;
    const int ESC_KEY = 27;


    //Start video capture
    VideoCapture cap(0); // cap(0) -> open the default/inbuilt camera

    if (cap.isOpened())  // check if we succeeded
    {

        int c = 0;
        bool found;

        namedWindow("Current view", 1);
        for (;;) {
            c = waitKey(15);

            // get a new frame from camera
            Mat frame, imgclone, imggray;

            cap >> frame;

            //display the frame
            imshow("Current view", frame);

            imageSize = frame.size();
            //Use frame to determine chessboard corners:
            if (c == 'p') {
                if (successes < m_MaxNImages) {
                    imgclone = frame.clone();
                    imggray.create(imgclone.size(), CV_8UC1);

                    //Find chessboard corners:
                    found = findChessboardCorners(imgclone, board_sz, corners,
                                                  CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

                    if (found) {
                        // improve the found corners' coordinate accuracy for chessboard
                        Mat viewGray;
                        cvtColor(imgclone, imggray, COLOR_BGR2GRAY);
                        cornerSubPix(imggray, corners, Size(11, 11),
                                     Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));

                        // Draw the corners.
                        drawChessboardCorners(imgclone, board_sz, Mat(corners), found);

                        imagePoints.push_back(corners);

                        successes++;
                    }
                    imshow("Calibration", imgclone);
                } else
                    break;
            }

            if (c == ESC_KEY) break;
        }
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    //END COLLECTION WHILE LOOP.

    //close all windows
    destroyAllWindows();


    if (successes == m_MaxNImages) {
        //PERFORM CALIBRATION
        vector<Mat> rvecs, tvecs;
        vector<float> reprojErrs;
        double totalAvgErr = 0;

        cameraMatrix = Mat::eye(3, 3, CV_64F);
        distCoeffs = Mat::zeros(8, 1, CV_64F);

        vector<vector<Point3f> > objectPoints(1);

        //Compute board_corners
        for (int i = 0; i < board_sz.height; ++i)
            for (int j = 0; j < board_sz.width; ++j)
                objectPoints[0].push_back(Point3f(j * m_SquareLength, i * m_SquareLength, 0));


        objectPoints.resize(imagePoints.size(), objectPoints[0]);

        //Find intrinsic and extrinsic camera parameters
        double rms;
        rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, 0);

        cout << "Re-projection error reported by calibrateCamera: " << rms << endl;

        bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

        //Calculate average reprojection error
        //totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

        cout << (ok ? "Calibration succeeded" : "Calibration failed");



        //SAVE PARAMS
        FileStorage fs("/mnt/4DA028260B3CDAA2/Development/RGiRV/LV3/src/CameraCalib/cameraparams.xml", FileStorage::WRITE);
        fs << "camera_matrix" << cameraMatrix;
        fs << "distortion_coefficients" << distCoeffs;
        fs.release();
    }

    destroyAllWindows();

    return 0;
}