import evdev
import sys
import time
import util
import RPi.GPIO as gpio
import serial
import os
from rpi_hardware_pwm import HardwarePWM
import pigpio

lport = 18
rport = 19

relay1 = 15
relay2 = 14
horn = 23
turntable = 26 #26
winch = 13 #13

controller_name = "Xbox Wireless Controller"

found_path = None
controller = None

def find_device():
    global found_path
    devices = [evdev.InputDevice(path) for path in evdev.list_devices()]

    for device in devices:
        print(device.name)
        if device.name == controller_name:
            found_path = device.path

def connect_controller():
    global found_path
    global controller
    controller = evdev.InputDevice(found_path)
    print(controller.capabilities(verbose=True, absinfo=True))


        
while found_path == None:
    find_device()
    print("Controller not found, check name")
    time.sleep(1)
    
#read from pico

pi = pigpio.pi()

#i.set_mode(turntable, pigpio.OUTPUT)

#gpio init stuff
gpio.setmode(gpio.BCM)
#gpio.setup(lport, gpio.OUT)
gpio.setup(winch, gpio.OUT)
gpio.setup(turntable, gpio.OUT)
#gpio.setup(rport, gpio.OUT)
gpio.setup(relay1, gpio.OUT)
gpio.setup(relay2, gpio.OUT)
gpio.setup(horn, gpio.OUT)

#50hz standard
#lpwm = gpio.PWM(lport ,50)
lpwm = HardwarePWM(pwm_channel=0, hz=50, chip=0)
rpwm = HardwarePWM(pwm_channel=1, hz=50, chip=0)
#rpwm = gpio.PWM(rport, 50)
lft = gpio.PWM(winch, 50)
trn = gpio.PWM(turntable, 50)
#neutral position
lpwm.start(7.5)
rpwm.start(7.5)
lft.start(0)
trn.start(0)
#pi.set_PWM_frequency(turntable, 50)
#pi.set_PWM_dutycycle(turntable, 0)
gpio.output(relay1, 1)
gpio.output(relay2, 1)
gpio.output(horn, 1)

    

#damping constant for feed forward filter
k = 0.1

#this is in the pi pico ADC terms. not psi
pressure1 = 0
pressure2 = 0

cruise = False

#read events

def event_loop():
    global x_val
    global y_val
    global cruise
    global left
    global right

    
    x_val = -util.clean_input_127(controller.absinfo(evdev.ecodes.ABS_RX))
    y_val = -util.clean_input_127(controller.absinfo(evdev.ecodes.ABS_Y))
    
    if util.is_button_pressed('Rpress', controller):
        lft.ChangeDutyCycle(8)
        #cruise = False
    elif util.is_button_pressed('Lpress', controller):
        #cruise = True
        lft.ChangeDutyCycle(6)
    else:
        lft.ChangeDutyCycle(0)
    
    if not cruise:
        left = y_val - x_val
        right = y_val + x_val
    else:
        print(left, right)
    #print(util.pwm_ify(right))
    rpwm.change_duty_cycle(util.pwm_ify(right))
    lpwm.change_duty_cycle(util.pwm_ify(left))
    #print(util.pwm_ify(left))
    #lpwm.ChangeDutyCycle(11.65);
    #print(util.is_button_pressed('A', controller))
    gpio.output(relay1, not util.is_button_pressed('A', controller))
    gpio.output(horn, not util.is_button_pressed('Lshoulder', controller))
    gpio.output(relay2, not util.is_button_pressed('X', controller))
    
    if (util.is_button_pressed('B', controller)):
        trn.ChangeDutyCycle(8)
        #pi.set_PWM_dutycycle(turntable, 7.9)
    elif util.is_button_pressed('Y', controller):
        trn.ChangeDutyCycle(6.5)
        #pi.set_PWM_dutycycle(turntable, 6.1)
    else:
        #pi.set_PWM_dutycycle(turntable, 0)
        trn.ChangeDutyCycle(0)
    #gpio.output(relay1, False)
    time.sleep(0.0001)

while True:
     try:
          event_loop()
     except:
          found_path = None
          controller = None
          while found_path is None:
               find_device()
               print("controller dc, finding")
               time.sleep(1)
          connect_controller()
