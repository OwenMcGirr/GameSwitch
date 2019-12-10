# GameSwitch
An Arduino project. Allows disabled users to play Xbox One games with three switches.

# Hardware required
• 2x Adafruit Feather 32u4

• An iOS device

• Xbox Adaptive Controller

• 3D printer (only required for printing enclosure)

# Software required
• Xcode

• Visual Studio Code with PlatformIO extension

# Arduino frameworks required 
• ArduinoJoystickLibrary - https://github.com/MHeironimus/ArduinoJoystickLibrary

• Adafruit BluefruitLE nRF51

# How it works
Each microcontroller plugs into either side of the Xbox Adaptive Controller. It is important that the left microcontroller is running the GameSwitchLeftUSB firmware and the right microcontroller is running the GameSwitchRightUSB firmware. When they are both plugged in launch the XboxGS app on your iOS device. Each microcontroller should connect automatically to the app. 

# Button mapping 
In order to play the Xbox you will have to remap a few buttons using the Xbox Accessories app. 

#### Remap the following 
1. Left stick (press) - left trigger 

2. Right stick (press) - right trigger 

3. X1 - Xbox button action 
