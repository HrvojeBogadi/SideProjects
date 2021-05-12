#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define ESC_KEY 27

Mat loadImage(String pathToFile);
void selectROI(int event, int x, int y, int flags, void* userData);

bool roiSelected = false;
Rect roi;

int main() {
    String pathToLearn = "/mnt/4DA028260B3CDAA2/Development/RGiRV/LV4/src/ImgLearn.jpeg";
    String pathToTest = "/mnt/4DA028260B3CDAA2/Development/RGiRV/LV4/src/ImgTestRotated.jpg";;
    Mat imgLearn, imgTest, imgRoi;
    char c;

    try {
        imgLearn = loadImage(pathToLearn);
        imgTest = loadImage(pathToTest);
    } catch (const char* msg) {
        cerr << msg << endl;
    }

    /* Downscale huge input image */
    int scale_amount_percentage = 35;   //Percentage of the original image size
    int widthL = imgLearn.size[1] * scale_amount_percentage / 100 ;
    int heightL = imgLearn.size[0] * scale_amount_percentage / 100 ;
    int widthT = imgLearn.size[1] * scale_amount_percentage / 100 ;
    int heightT = imgLearn.size[0] * scale_amount_percentage / 100 ;

    resize(imgLearn, imgLearn, Size (widthL, heightL), INTER_AREA);
    resize(imgTest, imgTest, Size (widthT, heightT), INTER_AREA);

    imshow("Image", imgLearn);

    setMouseCallback("Image", selectROI, &imgLearn);

    cout << "Select rectangular region of interest.\nPress 'c' to confirm your selection" << endl;

    /**** DETERMINE THE REGION OF INTEREST ****/

    Mat imgCpy = imgLearn.clone();

    while(1){
        c = waitKey(15);
        imshow("Image", imgCpy);

        if(roiSelected){
            imgCpy = imgLearn.clone();   /* Re-copy the original
                                          * image to see only the last drawn rectangle, not all of them
                                          */
            rectangle(imgCpy, roi, Scalar(255, 0, 0), 3);
        }

        if(c == 'c'){
            setMouseCallback("Image", NULL, NULL);
            cout << "Region of interest successfully selected!" << endl;
            imgRoi = imgLearn(roi);
            destroyWindow("Image");
            break;
        }
    }

    /**** FIND OBJECT FEATURES USING SIFT ****/

    cvtColor(imgRoi, imgRoi, COLOR_BGR2GRAY);
    cvtColor(imgTest, imgTest, COLOR_BGR2GRAY);

    Ptr<Feature2D> siftDetector = SIFT::create();
    vector<KeyPoint> keypointsLearn, keypointsTest;
    Mat descriptorsLearn, descriptorsTest;
    siftDetector->detectAndCompute(imgRoi, noArray(), keypointsLearn, descriptorsLearn);
    siftDetector->detectAndCompute(imgTest, noArray(), keypointsTest, descriptorsTest);

    Mat siftOutLearn, siftOutTest;

    drawKeypoints(imgRoi, keypointsLearn, siftOutLearn);
    drawKeypoints(imgTest, keypointsTest, siftOutTest);

    imshow("ROI", siftOutLearn);
    imshow("Scene", siftOutTest);

    /**** FIND FEATURE MATCHES ****/
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector<vector<DMatch>> matches;
    matcher->knnMatch(descriptorsLearn, descriptorsTest, matches, 2);
    vector<DMatch> goodMatches;

    const float ratio_tresh = 0.65f; // Value empiricaly determined
    for(size_t i = 0; i < matches.size(); i++){
        if(matches[i][0].distance < ratio_tresh * matches[i][1].distance){
            goodMatches.push_back(matches[i][0]);
        }
    }

    Mat siftMatches;

    drawMatches(imgRoi, keypointsLearn, imgTest, keypointsTest, goodMatches, siftMatches);

    vector<Point2f> object, scene;

    for(size_t i = 0; i < goodMatches.size(); i++){
        object.push_back((Point2f)keypointsLearn[goodMatches[i].queryIdx].pt);
        scene.push_back((Point2f)keypointsTest[goodMatches[i].trainIdx].pt);
    }

    /**** DRAW RECOGNIZED FEATURES ****/
    Mat homography = findHomography(object, scene, noArray(), RANSAC);

    vector<Point2f> objectCorners(4);

    objectCorners[0] = Point2f(0, 0);
    objectCorners[1] = Point2f(0, (float)imgRoi.rows);
    objectCorners[2] = Point2f((float)imgRoi.cols, 0);
    objectCorners[3] = Point2f((float)imgRoi.cols, (float)imgRoi.rows);
    vector<Point2f> sceneCorners(4);
    perspectiveTransform(objectCorners, sceneCorners, homography);

    line(siftMatches, sceneCorners[0] + Point2f((float)imgRoi.cols, 0), sceneCorners[1] + Point2f((float)imgRoi.cols, 0), Scalar(0, 255, 0), 3);
    line(siftMatches, sceneCorners[1] + Point2f((float)imgRoi.cols, 0), sceneCorners[3] + Point2f((float)imgRoi.cols, 0), Scalar(0, 255, 0), 3);
    line(siftMatches, sceneCorners[2] + Point2f((float)imgRoi.cols, 0), sceneCorners[0] + Point2f((float)imgRoi.cols, 0), Scalar(0, 255, 0), 3);
    line(siftMatches, sceneCorners[3] + Point2f((float)imgRoi.cols, 0), sceneCorners[2] + Point2f((float)imgRoi.cols, 0), Scalar(0, 255, 0), 3);

    imshow("Found Matches", siftMatches);

    waitKey(0);
    destroyAllWindows();
    return 0;
}

Mat loadImage(String pathToFile){
    Mat img = imread(pathToFile, IMREAD_COLOR);

    if(img.empty()){
        throw "Image could not be loaded. Please check your path";
    }
    return img;
}

void selectROI(int event, int x, int y, int flags, void* userData){
    Mat* img = (Mat*)userData;

    if(event == EVENT_LBUTTONDOWN){
        roiSelected = true;
        roi = Rect(Point(x, y), Point(x, y));
    }

    if(event == EVENT_MOUSEMOVE){
        if(roiSelected == true){
            roi.height = y - roi.y;
            roi.width = x - roi.x;
        }
    }

    if(event == EVENT_LBUTTONUP){
        if(roiSelected){
            roiSelected = false;
        }
    }
}