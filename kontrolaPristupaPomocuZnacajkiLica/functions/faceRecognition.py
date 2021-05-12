import cv2 as cv
import dlib
import numpy as np
import imutils
import keras.models as km

faceDetector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(".\\data\\shape_predictor_68_face_landmarks.dat")
pathToModel = ".\\model\\facenet_keras.h5"
model = km.load_model(pathToModel)


def cnnEmbeddings(images):
    embeddings = model.predict(images)

    return embeddings
    #Returns all embeddings of all images given to the cnn


def alignFace(image, face):
    pointsLeft = []
    pointsRight = []

    def findEyeCentres():
        xCoordinatesLeft, yCoordinatesLeft = zip(*pointsLeft)

        xCoordinatesRight, yCoordinatesRight = zip(*pointsRight)

        rectangleCentreLeft = int((min(xCoordinatesLeft) + max(xCoordinatesLeft)) / 2), int(
            (min(yCoordinatesLeft) + max(yCoordinatesLeft)) / 2)
        rectangleCentreRight = int((min(xCoordinatesRight) + max(xCoordinatesRight)) / 2), int(
            (min(yCoordinatesRight) + max(yCoordinatesRight)) / 2)

        return rectangleCentreLeft, rectangleCentreRight


    def rotateImage(leftEyeCentre, rightEyeCentre, image):
        if leftEyeCentre[1] > rightEyeCentre[1]:
            width = rightEyeCentre[0] - leftEyeCentre[0]
            height = leftEyeCentre[1] - rightEyeCentre[1]
            direction = -1
        else:
            width = rightEyeCentre[0] - leftEyeCentre[0]
            height = rightEyeCentre[1] - leftEyeCentre[1]
            direction = 1

        angle = direction * np.arctan2(height, width)
        angle = angle * 180 / np.pi
        image = imutils.rotate(image, angle)

        return image

    for i in range(36, 42):
        pointsLeft.append([face.part(i).x, face.part(i).y])

    for i in range(42, 48):
        pointsRight.append([face.part(i).x, face.part(i).y])

    leftEye, rightEye = findEyeCentres()

    image = rotateImage(leftEye, rightEye, image)

    return image


def findFaces(imageOrig):
    image = np.copy(imageOrig)
    image = cv.cvtColor(image, cv.COLOR_RGB2GRAY)
    faces = faceDetector(image)

    fcs = [[(face.left(), face.top()), (face.right(), face.bottom())] for face in faces]

    return fcs


def findFaceLandmarks(imageOrig):
    image = np.copy(imageOrig)
    image = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
    faces = faceDetector(image)
    landmarks = []

    for face in faces:
        landmarks = predictor(image, face)

    return landmarks


def cropFace(image):
    imgCpy = np.copy(image)
    faces = findFaces(image)

    if(len(faces) > 1):
        return None

    for face in faces:
        imgCpy = imgCpy[face[0][1]:face[1][1], face[0][0]:face[1][0]]
        return imgCpy

    return None


def calculateDistance(knownEncoding, currentEncoding):
    dist = np.linalg.norm(knownEncoding - currentEncoding)
    return dist


def compareFaces(knownEncodings, currentEncoding):
    threshold = 3
    distances = []

    for encoding in knownEncodings:
        distance = calculateDistance(encoding, currentEncoding) <= threshold
        distances.append(distance)
    return distances


def getCurrentUser(frame, data):
    users = data["users"]
    features = data["encodings"]

    encoding = findEncodings([frame])

    if encoding is None:
        return []

    names = []

    matches = compareFaces(features, encoding)
    if True in matches:
        matchedIdxs = [i for (i, b) in enumerate(matches) if b]
        hits = {}
        for i in matchedIdxs:
            username = users[i]
            hits[username] = hits.get(username, 0) + 1
        username = max(hits, key=hits.get)
        names.append(username)

    return names


def findEncodings(images):
    cnnInput = []

    for image in images:
        image = cv.cvtColor(image, cv.COLOR_BGR2RGB)
        landmarks = findFaceLandmarks(image)
        image = alignFace(image, landmarks)
        image = cropFace(image)
        if image is None:
            return None
        else:
            image = cv.resize(image, (160, 160), cv.INTER_AREA)

            cnnInput.append(image)

    cnnInput = np.asarray(cnnInput) #Expected CNN input
    embeddings = cnnEmbeddings(cnnInput)

    return embeddings

