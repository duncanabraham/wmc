# WiFi Motor Controller

<img width="1007" alt="image" src="https://github.com/duncanabraham/wmc/assets/5994927/a5c3c90a-4fcc-4c8e-834c-9338acafe754">

## It's a work in progress ...

## Introduction
The WiFi Motor Controller is an innovative project that enables the control of a DC motor via WiFi, simplifying setups by eliminating the need for complex wiring beyond a power supply. This project is perfect for hobbyists and professionals looking to integrate motor control into their wireless systems.

I don't know anyone else doing this and initial testing is going well. I think this could be a game changer for hobby robotics projects. With so many microcontroller boards and SBCs having WiFi, why not take advantage of the high bandwidth and wire-less operation! 

![image](https://github.com/duncanabraham/wmc/assets/5994927/b1716846-3414-4d9a-984a-ab04700f69b3)

## Features
- Remote control over WiFi
- Customizable motor speed and direction
- Onboard calibration for optimal performance
- Easy web interface for setup and configuration
- Unique serial number allocation for easy management


## Hardware
The hardware consists of:

* DC Motor                   - Amazon: EsportsMJJ 775 Motor DC 12V-36V 3500-9000RPM
* BTS7960                    - 43A Peak, 30A continuous, Motor Controller 
* AS5600                     - Magnetic 12bit rotary encoder
* ESP8266                    - Microcontroller Dev Module
* AHT21                      - Temperature sensor
* Custom Chassis             - 3d printed housing
* Custom 2-way power socket  - 2x XT30 Sockets
* Assorted fixings           - Nuts and bolts
 
The chassis is my design and both CAD and STL files will be included in the project when everything has been tested and fine tuned.

An AS5600 12bit encoder is used to sense the motor position and this is fed back to the microcontroller.  The microcontroller then controls the speed and direction of the motor by providing the appropriate PWM signals.

For detailed information on the pin connections, please see the [Pin Connections](./pins.md) document.

## Web Interface and Configuration

The WiFi Motor Controller features a simple API, allowing for straightforward configuration and management directly over WiFi. This interface is key to setting up your controller and customising it for your specific needs.

### Accessing the Web Interface
Upon the initial setup, connect your device (like a laptop or smartphone) to the `WMC-Config` network. No password is needed for this initial connection. Once connected, navigate to `192.168.4.1` in your web browser. You'll be presented with a simple form to configure the WiFi settings of your motor controller.

### Setting WiFi Credentials
The web interface provides a basic form to enter the SSID and password of your WiFi network. Fill in these details and hit the `SAVE` button. The motor controller will restart and connect to your specified WiFi network. It's recommended to assign a static IP address to the motor controller in your router settings to avoid IP changes.

### Device Configuration and Management
Once connected to your WiFi network, the controller can be accessed at its new IP address. Here, you can:

- **Check the Status:** Direct your browser to `http://<your-controller-ip>/status` to view the current configuration and status of your motor controller.
- **Calibrate Your Motor:** Access `http://<your-controller-ip>/calibrate` to start the motor calibration process. This step is crucial for the optimal operation of your motor.
- **Update PID Values:** Fine-tune the PID control loop parameters through `http://<your-controller-ip>/config`. This advanced feature is for users who are familiar with PID tuning and wish to customize the motor behavior further.

### Calibration Process
Before running the calibration, ensure that your motor is free to turn and that the power source is at its optimal level. The calibration routine involves several steps:

1. **Determining Minimum Speed:** The controller gradually increases the PWM duty cycle to find the slowest speed at which the motor can operate reliably.
2. **Estimating Maximum Speed:** The controller tests various speeds to predict the maximum operational speed without stressing the motor.
3. **Storing Calibration Data:** Once both values are determined, they are saved on the controller, marking it as calibrated.

After calibration, visiting `http://<your-controller-ip>/status` will show the min and max speed limits for your motor.

### Advanced Features
The motor controller uses a PID control loop for smooth operation. While default PID values are set, you may need to adjust them based on your motor and application. Caution is advised as PID tuning requires a good understanding of control systems.

## Available commands are:
/status             - to show the current motor status
/calibrate          - to determine motor min and max rpm values
/config             - to configure some basic parameters
/speed?value=[n|-n] - set the desired speed in RPM.  A negative number denotes CCW and a positive number CW rotation.
/hold               - attempt to keep the motor in the current position - if this draws too much power it may be removed
/free               - allow the motor to turn freely without power.
/factory_reset      - clear the EEPROM to remove all stored settings.
/brake              - Stop and hold the motor by enabling both sides of the H-bridge.
/release            - release the brake.


### /status
Direct your browser to the allocated IP address - e.g. `http://<your-controller-ip>/status` - and you'll see something like this: 
```
{
  "firmwareVersion": "0.0.2",
  "serialNumber": "00000000-AFC6-F2C2-4275-09F500000000",
  "calibrated": false,
  "pid": {
    "kp": 2.00,
    "ki": 0.50,
    "kd": 0.10
  },
  "direction": "",
  "minSpeed": 0,
  "maxSpeed": 0,
  "actualSpeed": 0.00,
  "targetSpeed": 0.00,
  "actualSpeedRPM": 0.00,
  "targetSpeedRPM": 0.00
}
```

This indicates the motor controller is configured and ready to play.  Each Motor Controller will be allocated a unique serial number to allow it to be managed and identified easily.

### /calibrate: `http://<your-controller-ip>/calibrate` 
This causes the motor controller to perform a test to see how slow and how fast the motor can turn.  This then stores the min and max values to be used later. 

### /config: `http://<your-controller-ip>/confg`
A simple form is presented to allow values to be updated and stored to EEPROM.  This allows the PID controller to be tweaked and also the Name of the motor to be added. Having a name helps with later management.

### /speed: `http://<your-controller-ip>/speed?value=[n|-n]`
To make your configured motor turn you will need to call the speed command and pass a desired speed in RPM.  Providing a positive number causes the motor to turn in one direction and a negative number the other.  If you provide a value that is outside of the calibrated min and max values it will be ignored.  Use the `/free` command to stop your motor, don't set the RPM to 0

### /hold: `http://<your-controller-ip>/hold`
This will take a note of the current position and attempt to hold the motor in that position.  I have concerns about this mode causing too much current to be pulled whilst the motor is not turning and moving air to cool down.  If this mode proves to be problematic it will be removed.

### /free: `http://<your-controller-ip>/free`
Set the motor free!! Stop sending PWM signals and allow the motor to turn freely without power.

### /factory_reset
As the name suggests, this will wipe all stored data from the device and return it to its initial state.

This clears:
* SSID and Password
* Serial number
* Name
* Motor parameters


