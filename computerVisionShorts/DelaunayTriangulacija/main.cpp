#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <time.h>

using namespace cv;
using namespace std;

Point2f randPoint(int winSize, int mapSize);

struct MouseCallbackUserData {
    Mat win;
    Subdiv2D *subdiv;
};

void onClick(int event, int x, int y, int flags, void* userData);

int main() {
    srand(time(NULL));

    /****Create a GUI Window****/
    int winSize = 440;
    Mat win;
    win.create(winSize, winSize, CV_8UC3); /* Create matrix (with 8-bit unsigned
                                                 * 3-channel array elements) that is the same
                                                 * size as the window so that each
                                                 * matrix element represents the belonging
                                                 * pixel
                                                 */
    win.setTo(255); /* Set window background colour to white */

    /****Randomly scater points inside the map (window)****/
    int nPoints = 20;
    int mapSize = 400;

    Rect rect((winSize-mapSize)/2-1, (winSize-mapSize)/2-1, mapSize+2, mapSize+2);
    /*Rectangle is 1px larger on each side in order to fit all the points inside of it and not on its
     * edges (subdiv requires all values to be inside of the rectangle)
     */
    Subdiv2D subdiv(rect);

    vector<Point2f> points;

    /*Push first 4 points into points vector*/
    points.push_back(Point2f (rect.x+1, rect.y+1));
    points.push_back(Point2f (rect.x + rect.width-1, rect.y+1));
    points.push_back(Point2f (rect.x+1, rect.y + rect.height-1));
    points.push_back(Point2f (rect.x + rect.width-1, rect.y + rect.height-1));

    for(int i = 4 ; i < nPoints; i++){
        points.push_back(randPoint(winSize, mapSize));
    }

    /****Calculate Delaunay Triangulation for given points****/

    subdiv.insert(points);

    int nEdges = 0;
    int iEdge, iEdgeNext;

    Point pt1, pt2;

    /****Determine number of edges created by Delaunay triangulation****/
    for(int i = 4; i < nPoints+4; i++){
        subdiv.getVertex(i, &iEdge);
        iEdgeNext = iEdge;

        do {
            if (iEdgeNext > nEdges)
                nEdges = iEdgeNext;

            iEdgeNext = subdiv.getEdge(iEdgeNext, subdiv.NEXT_AROUND_ORG);
            pt1 = subdiv.getVertex(subdiv.edgeOrg(iEdgeNext));
            pt2 = subdiv.getVertex(subdiv.edgeDst(iEdgeNext));
        }while(iEdgeNext != iEdge);
    }

    /****Draw edges calculated by Delaunay Triangulation****/

    int ip1, ip2;
    Point p1, p2;
    for(iEdge = 0; iEdge <= nEdges; iEdge++){
        ip1 = subdiv.edgeOrg(iEdge);
        ip2 = subdiv.edgeDst(iEdge);

        if(ip1 >= 4 && ip2 > ip1){
            p1 = subdiv.getVertex(ip1);
            p2 = subdiv.getVertex(ip2);
        }

        line(win, p1, p2, Scalar(0, 0, 0), 1);
    }

    imshow("Delaunay Triangulacija", win);

    /****Enable and track left mouse click and pointer position****/

    MouseCallbackUserData userCallbackData;
    userCallbackData.win = win;
    userCallbackData.subdiv = &subdiv;


    setMouseCallback("Delaunay Triangulacija", onClick, &userCallbackData);

    /****Close all windows and release mouse callback****/

    waitKey();
    setMouseCallback("Delaunay Triangulacija", NULL, NULL);
    destroyAllWindows();

    return 0;

}

void onClick(int event, int x, int y, int flags, void* userData){
    MouseCallbackUserData *uData = (MouseCallbackUserData*)userData;
    int flag = 0;

    /****Find a selected triangle****/

    if(event == EVENT_LBUTTONUP){
        imshow("Delaunay Triangulacija", uData->win);
        int nTriangles, iTriangle;
        vector<Vec6f> Triangles;
        Point a, b, c;
        vector<int> isInside;

        uData->subdiv->getTriangleList(Triangles);
        nTriangles = Triangles.size();

        for (int i = 0; i < nTriangles; i++) {
            a.x = Triangles[i][0];
            a.y = Triangles[i][1];
            b.x = Triangles[i][2];
            b.y = Triangles[i][3];
            c.x = Triangles[i][4];
            c.y = Triangles[i][5];

            vector<vector<Point>> contours = {{{a.x, a.y}, {b.x, b.y}, {c.x, c.y}}};

            isInside.push_back(pointPolygonTest(contours[0], Point2f((float) x, (float) y), false));
            /* The selected triangle is found by checking inside of which polygon
             * the mouse pointer can be found.
             */
        }
        for (int j = 0; j < nTriangles; j++) {
            if (isInside[j] == 1) {
                iTriangle = j;
                flag = 1;
            }
        }

        if(flag == 0)
            return;

        a.x = Triangles[iTriangle][0];
        a.y = Triangles[iTriangle][1];
        b.x = Triangles[iTriangle][2];
        b.y = Triangles[iTriangle][3];
        c.x = Triangles[iTriangle][4];
        c.y = Triangles[iTriangle][5];

        vector<Point> centralTriangle;
        centralTriangle.push_back(a);
        centralTriangle.push_back(b);
        centralTriangle.push_back(c);

        Mat win2 = uData->win.clone();

        fillConvexPoly(win2, centralTriangle, Scalar(0, 0, 255));

        /****Get all triangles sharing an edge with the selected triangle****/

        vector<Vec6f> wantedTriangles;

        for (int i = 0; i < nTriangles; i++) {
            a.x = Triangles[i][0];
            a.y = Triangles[i][1];
            b.x = Triangles[i][2];
            b.y = Triangles[i][3];
            c.x = Triangles[i][4];
            c.y = Triangles[i][5];

            flag = 0;
            for (int j = 0; j < 3; j++) {
                if ((a.x == centralTriangle[j].x && a.y == centralTriangle[j].y) ||
                    (b.x == centralTriangle[j].x && b.y == centralTriangle[j].y) ||
                    (c.x == centralTriangle[j].x && c.y == centralTriangle[j].y))
                    flag++;
            }
            if (flag == 2)
                wantedTriangles.push_back(Triangles[i]);

        }

        for (int i = 0; i < wantedTriangles.size(); i++) {
            a.x = wantedTriangles[i][0];
            a.y = wantedTriangles[i][1];
            b.x = wantedTriangles[i][2];
            b.y = wantedTriangles[i][3];
            c.x = wantedTriangles[i][4];
            c.y = wantedTriangles[i][5];

            vector<Point> wantedTriangle;
            wantedTriangle.push_back(a);
            wantedTriangle.push_back(b);
            wantedTriangle.push_back(c);

            fillConvexPoly(win2, wantedTriangle, Scalar(255, 0, 0));
        }

        imshow("Delaunay Triangulacija", win2);
    }
}

Point2f randPoint(int winSize, int mapSize){
    return Point2f (( rand()%mapSize + ( winSize-mapSize )/2 ), ( rand()%mapSize + ( winSize-mapSize )/2 ));
}