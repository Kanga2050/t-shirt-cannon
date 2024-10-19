import evdev
import sys
import time
import util
import RPi.GPIO as gpio
import serial
import os

lport = 18
rport = 12

relay1 = 14
relay2 = 15
horn = 23

#gpio init stuff
gpio.setmode(gpio.BCM)
gpio.setup(lport, gpio.OUT)
gpio.setup(13, gpio.OUT)
gpio.setup(rport, gpio.OUT)
gpio.setup(relay1, gpio.OUT)
gpio.setup(relay2, gpio.OUT)
gpio.setup(horn, gpio.OUT)

#50hz standard
lpwm = gpio.PWM(lport ,50)
rpwm = gpio.PWM(rport, 50)
lft = gpio.PWM(13, 50)

#neutral position
lpwm.start(7.5)
rpwm.start(7.5)
lft.start(7.5)
gpio.output(relay1, 0)

controller_name = "Xbox Wireless Controller"
devices = [evdev.InputDevice(path) for path in evdev.list_devices()]

found_path = None

def find_device():
    global found_path
    for device in devices:
        print(device.name)
        if device.name == controller_name:
            found_path = device.path
        
while found_path == None:
    find_device()
    print("Controller not found, check name")
    time.sleep(1)
    
#read from pico


    
controller = evdev.InputDevice(found_path)
print(controller.capabilities(verbose=True, absinfo=True))

#damping constant for feed forward filter
k = 0.1

#this is in the pi pico ADC terms. not psi
pressure1 = 0
pressure2 = 0

cruise = False

#read events
while True:

    
    x_val = util.clean_input_127(controller.absinfo(evdev.ecodes.ABS_RX))
    y_val = util.clean_input_127(controller.absinfo(evdev.ecodes.ABS_Y))
    
    if util.is_button_pressed('Rpress', controller):
        cruise = False
    elif util.is_button_pressed('Lpress', controller):
        cruise = True
    
    if not cruise:
        left = y_val - x_val
        right = y_val + x_val
    else:
        print(left, right)
    rpwm.ChangeDutyCycle(util.pwm_ify(right))
    lpwm.ChangeDutyCycle(util.pwm_ify(left))
    #print(util.pwm_ify(left))
    #lpwm.ChangeDutyCycle(11.65);
    #print(util.is_button_pressed('A', controller))
    gpio.output(relay1, util.is_button_pressed('A', controller))
    gpio.output(horn, util.is_button_pressed('Lshoulder', controller))
    gpio.output(relay2, util.is_button_pressed('X', controller))
    
    if (util.is_button_pressed('B', controller)):
        lft.ChangeDutyCycle(9)
    elif util.is_button_pressed('Y', controller):
        lft.ChangeDutyCycle(6)
    else: lft.ChangeDutyCycle(0)
    #gpio.output(relay1, False)
    time.sleep(0.0001)
