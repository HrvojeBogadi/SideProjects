import PySimpleGUI as sg
import cv2 as cv
import numpy as np
from functions import userDataHandler
from functions import folderHandler
from functions import faceRecognition as fr
from functions import cameraHandler

def welcomeScreen():
    sg.theme('DarkGrey6')
    mainInterface = [
        [
            sg.Button(button_text="Create New User Profile", key="newProfile", size=(20, 10)),
            sg.Button(button_text="Start The Program", key="start", size=(20, 10)),
            sg.Button(button_text="Unlock All Folders", key="unlock", size=(20, 10))
        ]
    ]
    window = sg.Window("Access Control", mainInterface)

    while (True):
        event, values = window.read(timeout=30)

        if event == "newProfile":
            window.close()
            registerUserGUI()
        if event == "start":
            window.close()
            workingScreen()
        if event == "unlock":
            folderHandler.unlockAllFolders()
            window.close()
        if event == sg.WIN_CLOSED:
            return


def registerUserGUI():
    sg.theme('DarkGrey6')

    mainInterface = [
        [
            sg.Text("Select folder to Lock"),
            sg.In(default_text="Path to Folder", size=(50, 1), enable_events=True, key="folder"),
            sg.FolderBrowse()
        ],

        [
            sg.Text("Username "),
            sg.In(size=(25, 1))
        ],

        [
            sg.Image(filename='', key='video')
        ],

        [
            sg.Button(button_text="Take Picture", key="takePic", size=(20,1)),
            sg.Ok(button_text="Save data", key="save"),
            sg.Cancel()
        ]
    ]

    window = sg.Window("Create User", mainInterface)
    cap = -1

    sg.popup_ok("Please take at least 10 images of your face. Try to take frontal images from various"
                " angles. \n\n"
                "Select the folder you wish to lock and select your username.")

    while(True):
        event, values = window.read(timeout=30)

        if(cap == None or cap == -1):
            cap = cameraHandler.initializeCamera()

        if cap == -1:
            popupEvent = sg.popup("Cannot open camera. Please check if camera is connected and try again", title="Camera not found", button_type=4)
            if popupEvent == "Cancel" or popupEvent == None:
                return
            continue

        frame = cameraHandler.getCameraFrame(cap)
        frameCpy = np.copy(frame)

        faceRect = fr.findFaces(frame)

        for face in faceRect:
            cv.rectangle(img=frameCpy, pt1=face[0], pt2=face[1], color=(0, 255, 0), thickness=2)

        image = cameraHandler.convertFrameToImg(frameCpy)
        window['video'].update(data=image)

        if event == 'Cancel' or event == sg.WIN_CLOSED:
            cap.release()
            return

        elif event == 'takePic':
            if values[0] == "":
                sg.popup_ok("Please input your username")
                continue
            elif values['folder'] == "Path to Folder" or values['folder'] == "":
                sg.popup_ok("Please select a folder you wish to lock")
                continue
            elif not folderHandler.folderExists(values['folder']):
                sg.popup_ok("Folder to lock can't be found. Please check the path to folder")
                continue
            elif faceRect == []:
                sg.popup_ok("No face recognized, please try again")
                continue
            elif len(faceRect) > 1:
                sg.popup_ok("Multiple people detected in the image, please take a picture of only one person")
                continue

            userDataHandler.saveImage(frame)

        elif event == 'save':
            if not userDataHandler.isEnoughImages():
                sg.popup_ok("Not enoguh images. Please take at least 10 images")
                continue

            try:
                userDataHandler.saveDataToFile(username=values[0], folder=values['folder'])
                folderHandler.lockAllFolders()
            except FileExistsError:
                sg.popup_ok("Folder you selected is already locked by another user. Please select other folder")
                continue
            break
    window.close()
    welcomeScreen()
    return


def workingScreen():
    sg.theme('DarkGrey6')

    mainInterface = [
        [
            sg.Text("The program is now working. You can minimise this window. If you wish to"
                    " exit, please click the button.")
        ],
        [
            sg.Button(button_text="Hide Folders And Quit", key="quit", size=(10, 5))
        ]
    ]
    window = sg.Window("Working...", mainInterface)

    cap = cameraHandler.initializeCamera()

    while (True):
        event, values = window.read(timeout=10)

        if event == "quit" or event == sg.WIN_CLOSED:
            window.close()
            folderHandler.lockAllFolders()
            return

        frame = cameraHandler.getCameraFrame(cap)
        data = userDataHandler.getOldFeatures()
        foldersInfo = userDataHandler.getFolderLockData()

        users = []

        if data is None or foldersInfo is None:
            sg.popup_ok("No data found, please register at least one user before you start using the program.")
            window.close()
            welcomeScreen()
            return

        usernames = foldersInfo["user"]
        folders = foldersInfo["folder"]

        faceRect = fr.findFaces(frame)
        if not (faceRect == []):
            if(len(faceRect) > 1):
                continue
            else:
                users = fr.getCurrentUser(frame, data)


        if users == []:
            folderHandler.lockAllFolders()
            continue
        else:
            for user in users:
                for i, username in enumerate(usernames):
                    if user == username:
                        folderHandler.unlockFolder(folders[i])