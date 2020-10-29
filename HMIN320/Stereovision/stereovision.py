from enum import Enum

import cv2

class Camera(Enum):
    LEFT = 0,
    RIGHT = 1

def OnClick(event, x, y, flags, params):
    if event == cv2.EVENT_LBUTTONDOWN:
        if (params == Camera.LEFT):
            print ("DDD")
        elif (params == Camera.RIGHT):
            print ("MDR")

leftImage = cv2.imread("images/TurtleG.tif")
rightImage = cv2.imread("images/TurtleD.tif")

cv2.namedWindow("Left Image", cv2.WINDOW_NORMAL)
cv2.namedWindow("Right Image", cv2.WINDOW_NORMAL)
cv2.setMouseCallback("Left Image", OnClick, Camera.LEFT) # Left
cv2.setMouseCallback("Right Image", OnClick, Camera.RIGHT) # Right

cv2.imshow("Left Image", leftImage)
cv2.imshow("Right Image", rightImage)

while True:
    key = cv2.waitKey(0)
    if key == 27:
        cv2.destroyAllWindows()
        break

