import numpy as np
import time
import requests
import random
from PIL import Image
from io import BytesIO
 
#change the IP address below according to the
#IP shown in the Serial monitor of Arduino code
esp32_cam_url='http://10.247.137.14/picture'
esp32_url = ''
 
hue_threshold = 25
saturation_lower_threshold = 100
saturation_upper_threshold = 255
value_lower_threshold = 100
value_upper_threshold = 255

def getGameState():
    """Get the game state from the ESP32.
    Result might be 'game not started', 'game started'
    """
    requests.get(esp32_url)

def turnOnLight(light='green'):
    """Send a POST request to ESP32 to turn on the green or red light
    """
    data = {
        'light':light
    }
    requests.post(esp32_url, data=data)

def getImage():
    """Gets one image from esp32 camera
    """
    response = requests.get(esp32_cam_url)

    # Check if the request was successful (status code 200)
    if response.status_code == 200:
        # Convert the image data from bytes to a NumPy array
        image_data = BytesIO(response.content)
        image = Image.open(image_data)
        return image
    else:
        print("Failed to retrieve image. Status code:", response.status_code)
        return

def filter_green(img):
    """return the green mask of tne input image

    Args:
        img_np (_type_): a PIL image.
    """
    # Convert the RGB image to HSV using colorsys.rgb_to_hsv
    hsv_array = np.array(img.convert("HSV"))
    # Extract the individual channels
    hue_channel = hsv_array[:, :, 0]
    saturation_channel = hsv_array[:, :, 1]
    value_channel = hsv_array[:, :, 2]

    green_mask = (hue_channel >= (120 - hue_threshold)) & (hue_channel <= (120 + hue_threshold))
    return green_mask

def motionDetect(interval=3, threshold_percentage = 0.3):
    """Get two images from the esp32 camera that are taken `interval` seconds apart.
    Detect whether there is motion based on these two images.

    Args:
        interval (int, optional): delay between the two images. Defaults to 3.
        threshold_percentage (float, optional): the percent change in green pixels used for determining whether there's motion. Defaults to 0.3.
    """
    img1_pil = getImage()
    # for debug purpose save the image to view
    img1_pil.save('captured_img.jpg')
    green_mask1  = filter_green(img=img1_pil)
    boolean_img1 = Image.fromarray(green_mask1)
    boolean_img1.save('green_mask.jpg')

    # wait for a while before taking the second picture
    time.sleep(interval)
    img2_pil = getImage()
    # for debug purpose save the image to view
    img2_pil.save('captured_img2.jpg')
    green_mask2  = filter_green(img=img2_pil)
    boolean_img2 = Image.fromarray(green_mask2)
    boolean_img2.save('green_mask2.jpg')

    green_mask_difference = np.logical_xor(green_mask1, green_mask2).astype(np.uint8)
    difference_pil = Image.fromarray(green_mask_difference)
    difference_pil.save('difference.jpg')

    diff = np.sum(green_mask_difference) / np.sum(green_mask2)
    player_moved = diff >= threshold_percentage

    return player_moved

def calculateTarget():
    """Find the (x, y) location of the target based on green_mask"""
    # get a green mask of the player at this moment
    player_img = getImage()
    green_mask = filter_green(player_img)
    # find the pixel coordinates of a green pixel
    xs, ys = np.where(green_mask)
    if len(xs) > 0:
        x = xs[len(xs)//2]
        y = ys[len(xs)//2]
        return (x, y)
    return None

def eliminatePlayer(target):
    """Send a POST request to ESP32 to eliminate player at target location.
    Args:
        target: (x, y) pixel coordinates. The indices of a green pixel on a 160x120 pixels image.
    """
    data = {
        'servos':'shootingMode',
        'target':target
    }
    requests.post(esp32_url, data=data)

def turnOffLights():
    """Send a POST request to ESP32 to turn off both lights
    """
    data = {
        'light':'off'
    }
    requests.post(esp32_url, data=data)

def resetServos():
    """Send a POST request to ESP32 to reset servos
    """
    data = {
        'servos':'reset'
    }
    requests.post(esp32_url, data=data)

def gameLoop():
    gameState = getGameState()
    while (gameState == 'game started'):
        turnOnLight(light='green')
        time.sleep(random.randint(5, 15))
        turnOnLight(light='red')
        player_moved = motionDetect()
        if player_moved:
            player_target = calculateTarget()
            eliminatePlayer(player_target)
            time.sleep(5) # pause a while for servos to get into shooting position
            resetServos() #return servos to initial positions
        time.sleep(5)
        
        gameState = getGameState()
    # game ended or is being reset, turn off lights
    turnOffLights()

if __name__=="__main__":
    player_moved, mask = motionDetect()
    calculateTarget(mask)