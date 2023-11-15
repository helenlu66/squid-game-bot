
# main.py -- put your code here!
from machine import Pin
from machine import I2C
from lib.vl53l0x import VL53L0X
from lib import mpu6050
from lib.ble import BLE

import math
import time
import board
import busio
import adafruit_vl53l0x

L2 = 0.05 # length of arm joint to the in meters

# LiDAR setup
i2c = busio.I2C(board.SCL, board.SDA)  # Create I2C bus
vl53 = adafruit_vl53l0x.VL53L0X(i2c)  # Create VL53L0X sensor object


# return lidar read in meters
def read_lidar():
    return vl53.range * 1000


# ensure arm_angle is in radians
def calculate_laser_coordinates(arm_angle, L=L2):
    arm_angle_rad = math.radians(arm_angle)
    relative_laser_x = L * math.cos(arm_angle_rad)
    relative_laser_y = L * math.sin(arm_angle_rad)
    return relative_laser_x, relative_laser_y


# TODO: from the image, find the center of the green pixels: depends on format of image
def find_coordinates_from_image(image):
    x, y = 0, 0
    return x, y


# for the x, y from image and distance from image to scene return the X,Y,Z coordinates of the scene point
def find_scene_coordinates(image_x, image_y, Z, f):
    X = (image_x * Z) / f
    Y = (image_y * Z) / f
    return X, Y


def calculate_inverse_kinematics(player_x, player_y, shoulder_x, shoulder_y, laser_x, laser_y, Z):
    absolute_laser_x = shoulder_x + laser_x
    absolute_laser_y = shoulder_y + laser_y

    # Calculate the angles in radians
    angle_base = math.atan2(player_x - absolute_laser_x, Z)
    angle_laser = math.atan2(player_y - absolute_laser_y, Z)

    # Convert angles to degrees if needed
    base_angle_deg = math.degrees(angle_base)
    laser_angle_deg = math.degrees(angle_laser)

    return base_angle_deg, laser_angle_deg


# with the X,Y,Z coordinates of the scene point, use inverse kinematics to find base rotation angle
def find_base_angle(X, Y, Z):
    return base_angle


# given angle rotate base
def rotate_base(current_angle, target_angle):
    rotation_angle = current_angle - target_angle
    return target_angle


# given angle rotate arm from the current angle to the target angle and return the new angle
def rotate_arm(current_angle, target_angle):
    rotation_angle = current_angle - target_angle
    return target_angle


if __name__ == '__main__':
    f = 0.01  # focal length of camera in meters
    arm_angle = 0
    base_angle = 0
    shoulder_x = 0  # in meters (likely will actually be 0)
    shoulder_y = 0  # in meters
    image = []  # TODO: unclear the format
    Z = read_lidar()  # ensure output is in meters
    laser_x, laser_y = calculate_laser_coordinates(arm_angle)
    image_x, image_y = find_coordinates_from_image(image)
    X, Y = find_scene_coordinates(image_x, image_y, Z, f)
    angle_base, angle_laser = calculate_inverse_kinematics(X, Y, shoulder_x, shoulder_y, laser_x, laser_y, Z)

    base_angle = rotate_base(base_angle)
    arm_angle = rotate_arm(arm_angle)
>>>>>>> Stashed changes
