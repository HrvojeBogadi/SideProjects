import os
from functions import userDataHandler as udh
from subprocess import call


def folderExists(path):
    if os.path.isdir(path):
        return True
    return False

def lockAllFolders():
    data = udh.getFolderLockData()
    folders = data["folder"]

    for folder in folders:
        call(["attrib", "+H", folder])
    return

def unlockAllFolders():
    data = udh.getFolderLockData()
    if data is not None:
        folders = data["folder"]

        for folder in folders:
            call(["attrib", "-H", folder])

        pathToDB = os.path.abspath('.')
        pathToDB = os.path.join(pathToDB, "bin\\database")

        pathToInfo = os.path.abspath('.')
        pathToInfo = os.path.join(pathToInfo, "bin\\userInfo")

        os.remove(pathToInfo)
        os.remove(pathToDB)

    return

def unlockFolder(folder):
    call(["attrib", "-H", folder])
    return
