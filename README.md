# WiFi Motor Controller
## It's a work in progress ...

So what is it?  I had this idea that a motor controller could communicate over wifi which would mean only power is required.

By providing a basic API on the controller itself it can not only be incorporated into a project with basic wifi commands, but 
could also me managed by a central management tool which could potentially offer more advanced features (more on that later).

The project is `wmc` and needs to be installed onto the Motor Controller's ESP8266.  The first time this is done over a USB
connection, however once installed the unit becomes fully wireless and can be managed and updated over WiFi.

Use the Aruino IDE for both the initial installation over USB and subsequent updates uning the OTA (Over The Air) connection.

The default IP Address after the initial flashing is `192.168.4.1` and is accessed after pointing your device at the `WMC-Config` network.  No password required, just connect to this network and browse to the default IP Address.

This will present a basic form asking for SSID and password for your local WiFi network.  Complete the details and press the `SAVE` button. The device will restart and should pick up an IP Address from your router/dhcp server.  I would suggest making this address permanent as you don't want your project to fail because the Motor's IP address has changed!

Point you browser at the allocated IP address - e.g. `http://<your-controller-ip>/status` - and you'll see something like this: 
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

Calibration is required to allow the Motor Controller to determine the limits for your motor.

```
`http://<your-controller-ip>/calibrate`
```
Before running the calibration routine:
* ensure your motor is free to turn
* ensure the power source is at optimum level (battery charged)

STEP 1: The calibration routine first determines the slowest speed the motor can run at.  DC motors are not like BLDC or Stepper Motors and there's going to be a minimum speed that the motor will turn at.  The motor controller will step up the PWM duty cycle until the motor moves. if this is 5rpm for example, you won't be able to instruct the motor to turn at 3rpm, it just won't work. In practice you probably want to double this lower number as a safe starting rpm.

STEP 2: The calibration routine now attempts to determine the highest speed by running at 10%, 20%, 30%, 40% and 50% then predicting the speed curve from the results and estimating the highest speed.  This is done so as not to stress your motor by running it at max speed.

STEP 3: when both values have been calculated they're stored on the controller and the device is flagged as calibrated.  Following this this Motor Controller is available to be used.

Running `http://<your-controller-ip>/status` again now will reveal the min and max speeds that can be used with this motor/controller/power source combination.

The Motor Controller employs a PID control loop to ensure smooth acceleration up to the desired speed.  The PID values can be changed to match your requirements using the `http://<your-controller-ip>/config` page.  Fine tuning a PID is a black-art and you should only tweak these values if you know what you're doing.


