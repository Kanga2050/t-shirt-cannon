import evdev

button_maps = {
    307: "X",
    308: "Y",
    304: "A",
    305: "B",
    318: "Lpress",
    317: "Rpress",
    310: "Lshoulder"
    }

def pwm_ify(val: int):
    val = min(val, 127)
    val = max(val, -127)
    val = 64 *(val/127)*(val/127)*(val/127);
    val += 64
    val *= 8.3/128
    val += 3.35
    
    if (val > 7.2 and val < 7.9): val = 0
    
    
    return val

def pico_to_psi(val: int):
    r2 = 220
    r1 = 100
    volts = val*3.3*(r1+r2) / (4096*r2)
    volts = max(0.5, volts)
    volts = min(4.5, volts)
    
    volts -= 0.5
    
    psi = volts*300/4
    
    return psi
    

#clean the reading and scale to -127 to +127
def clean_input_127(device_reading: evdev.device.AbsInfo):
    if abs(device_reading.value) < device_reading.flat:
        return 0
    
    val = max(device_reading.value, device_reading.min)
    val = min(val, device_reading.max)
    
    #assume uniform scale
    val *= 127/abs(device_reading.min)
    
    if (abs(val) < 18): return 0
    
    return int(val) if (val < 0) else int(val+0.5)

def is_button_pressed(name: str, device: evdev.InputDevice):
    pressed = [button_maps.get(i) for i in device.active_keys()]

    return name in pressed
    
    