import imutils
import cv2 as cv
from sys import platform

def initializeCamera():
    if platform == "win32":
        capture = cv.VideoCapture(0, cv.CAP_DSHOW)
    else:
        capture = cv.VideoCapture(0)

    if not capture.isOpened():
        capture = -1
    return capture


def getCameraFrame(cap):
    ret, frame = cap.read()
    resize = imutils.resize(frame, width=640)
    return resize


def convertFrameToImg(frame):
    imgbytes = cv.imencode('.png', frame)[1].tobytes()
    return imgbytes