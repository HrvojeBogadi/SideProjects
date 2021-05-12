import os
import pickle
from functions import faceRecognition as fr

images = []
numberOfImages = 10

pathToDB = os.path.abspath('.')
pathToDB = os.path.join(pathToDB, "bin\\database")

pathToInfo = os.path.abspath('.')
pathToInfo = os.path.join(pathToInfo, "bin\\userInfo")


def isEnoughImages():
    if len(images) < numberOfImages:
        return False
    return True


def saveImage(frame):
    images.append(frame)


def getFolderLockData():
    oldDataInfo = {}

    if os.path.isfile(pathToInfo):
        infoRead = open(pathToInfo, "rb")

        while True:
            try:
                oldDataInfo = pickle.load(infoRead)
            except EOFError:
                break
        infoRead.close()
        return oldDataInfo
    return None


def getOldFeatures():
    oldDataFeatures = {}

    if os.path.isfile(pathToDB):
        featuresRead = open(pathToDB, "rb")

        while True:
            try:
                oldDataFeatures = pickle.load(featuresRead)
            except EOFError:
                break
        featuresRead.close()
        return oldDataFeatures
    return None
    return None


def saveDataToFile(username, folder):
    users = [username]
    user = [username]
    folders = [folder]

    embeddings = fr.findEncodings(images)

    encodings = [embedding for embedding in embeddings]
    users = [username for i, embedding in enumerate(embeddings)]

    images.clear()

    oldDataInfo = getFolderLockData()
    oldDataFeatures = getOldFeatures()

    if oldDataInfo is not None:
        oldUser = oldDataInfo["user"]
        oldFolder = oldDataInfo["folder"]
        for folderPath in oldFolder:
            if folderPath == folder:
                raise FileExistsError()
        user.extend(oldUser)
        folders.extend(oldFolder)

    if oldDataFeatures is not None:
        oldEncodings = oldDataFeatures["encodings"]
        oldUsers = oldDataFeatures["users"]
        users.extend(oldUsers)
        encodings.extend(oldEncodings)

    featuresToSave = {"users": users, "encodings": encodings}
    userInfo = {"user": user, "folder": folders}

    file = open(pathToDB, "wb")
    pickle.dump(featuresToSave, file)
    file.close()

    file = open(pathToInfo, "wb")
    pickle.dump(userInfo, file)
    file.close()

    return
