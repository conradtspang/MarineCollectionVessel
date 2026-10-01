<div align="center">
  <img src="MarineCollectionVessel.png" alt="Marine Collection Vessel" width="300" />
  <h1>Marine Collection Vessel</h1>
  <p><strong>A Catamaran-Style Robotic Vessel for Cleaning Rivers, Harbors & Inland Waterways</strong>
    </br><strong>U.S. Utility Patent Application No. 19/754,961</strong>
</br><strong>U.S. Design Patent Application No. 30/073,887</strong></p>
</div>

## 📖 Overview
<p>The <b>Marine Collection Vessel</b> is a remote-controlled, and in later phases autonomous, surface vessel designed to collect floating debris from calm waterways.</p>
<p>An estimated 95% of plastic pollution in the world's seas arrives via major rivers. Cleaning these upstream corridors stops waste before it ever reaches the open ocean. Rivers and harbours also offer reduced wave action, predictable currents and defined boundaries, which makes them ideal for small, quiet, electric robotic vessels. The Marine Collection Vessel is sized to reach places conventional cleaning boats cannot: narrow channels, alongside dock pilings and near shoreline infrastructure.</p>
<p>The vessel drives <b>into and over</b> floating debris, funnelling it through a rigid collection mouth on the underside of the hull and into an interchangeable collection net at the rear.</p>

## ✨ Key Features
- 🚤 **Catamaran-Style Hull:** A central structure flanked by two buoyant, tear-drop-shaped outriggers for stability in calm water.
- 🌀 **Counter-Rotating Differential Thrust:** Two independently controlled, reversible 3-blade thrusters (one clockwise, one counterclockwise) give precise steering and turning on the spot.
- 🗑 **Underside Collection Mouth:** A rigid, triangulated lattice "scoop" with a D-ring cross-section that guides debris into the net without snagging.
- 🎣 **Interchangeable Collection Nets:** Rigid, flexible nylon/polypropylene or lattice-box nets, with optional floats and weights, attached via a primary/secondary D-ring connection.
- 📡 **Real-Time Encrypted Wireless Control:** Router-free, low-latency ESP-NOW link between the Remote Controller and the vessel, with a range of up to ~200 m.
- 🎮 **Joystick & Button Control:** Precise manual control of speed and heading from a compact handheld remote with a color display.
- 🔋 **Battery Monitoring:** Live battery voltage monitoring of both the vessel and the remote.
- 🦵 **Integrated Land Stand:** Legs and feet mounted to the wings keep the vessel upright on land and protect the collection mouth.

## 🏗 The Electronics & Control Architecture
All electronics and sensors are sealed inside the central structure's internal cavity, with motor wiring running through the hollow, waterproof propulsion legs to each thruster.
</br>The system separates time-critical control loops from high-level decision-making, so no single processor is overloaded:
1.	Remote Controller (ESP32-S3): Reads the joysticks and buttons, drives the TFT display, and sends commands to the vessel. Powered by a 3.7 V battery, charged and programmed over USB.
2.	Primary Processor (ESP32): Receives commands, runs real-time motor control and polls sensors.
3.	Motor Controller & Propulsion: A dual DC motor driver powers two 3-blade thruster 7.4 V motors, setting each motor's speed and direction independently with minimal power loss.
4.	Power Supply Unit: Distributes power from the 12 V lithium battery to the motor controller, processor and transceiver.
5.	Sensors (Phase 2): A LiDAR and a MEMS IMU provide obstacle detection, orientation, heading and speed data to the primary processor.
6.	Machine-Vision Processor (Raspberry Pi, Phase 2): Handles camera processing, object detection and path planning, and sends recommended routes to the primary processor.

The connections between components are summarised below:

Core System

- Remote Controller (ESP32-S3) → Primary Processor (ESP32): Encrypted ESP-NOW wireless
- Primary Processor → Dual DC Motor Driver: PWM speed & direction
- Dual DC Motor Driver → Thruster A (CW) / Thruster B (CCW): Motor power
- 12 V Li Battery → PSU → Processor & Motor Driver: Power distribution

Phase 2 Additions

- LiDAR → Primary Processor: Obstacle
- MEMS IMU → Primary Processor: Orientation, heading, speed
- Camera → Raspberry Pi (YOLO vision): Image stream
- Raspberry Pi → Primary Processor: Navigation data

<b>Pin mapping table</b>
<center>
<table>
  <tr>
    <th>ESP32 pin</th>
    <th>Connects to</th>
    <th>Signal</th>
    <th>Purpose</th>
  </tr>
  <tr>
    <td>GPIO32</td>
    <td>DRV8841 ENA</td>
    <td>PWM output</td>
    <td>Left thruster speed</td>
  </tr>
    <tr>
    <td>GPIO33</td>
    <td>DRV8841 IN1</td>
    <td>Digital output</td>
    <td>Left thruster direction</td>
  </tr>
    <tr>
    <td>GPIO25</td>
    <td>DRV8841 IN2</td>
    <td>Digital output</td>
    <td>Left thruster direction</td>
  </tr>
    <tr>
    <td>GPIO26</td>
    <td>DRV8841 ENB</td>
    <td>PWM output</td>
    <td>Right thruster speed</td>
  </tr>
    <tr>
    <td>GPIO27</td>
    <td>DRV8841 IN3</td>
    <td>Digital output</td>
    <td>Right thruster direction</td>
  </tr>
    <tr>
    <td>GPIO23</td>
    <td>DRV8841 IN4</td>
    <td>Digital output</td>
    <td>Right thruster direction</td>
  </tr>
      <tr>
    <td>GPIO34</td>
    <td>68k/10k voltage divider</td>
    <td>Analog input (ADC)</td>
    <td>Battery voltage monitoring</td>
  </tr>
        <tr>
    <td>GPIO2</td>
    <td>2N7000 MOSFET</td>
    <td>Digital output</td>
    <td>Switch Lidar Power rail</td>
  </tr>
          <tr>
    <td>GPIO4</td>
    <td>2N7000 MOSFET</td>
    <td>Digital output</td>
    <td>Switch Raspberry Pi Power rail</td>
  </tr>
            <tr>
    <td colspan="4"><b>Phase 2 (planned)</b></td>
  </tr>
           <tr>
    <td>GPIO19</td>
    <td>MPU-9250 SCL</td>
    <td>I²C clock</td>
    <td>IMU</td>
  </tr> 
            <tr>
    <td>GPIO18</td>
    <td>MPU-9250 SDA</td>
    <td>I²C data</td>
    <td>IMU</td>
  </tr> 
              <tr>
    <td>GPIO21</td>
    <td>TFmini-S TX</td>
    <td>UART receive</td>
    <td>LiDAR</td>
  </tr> 
                <tr>
    <td>GPIO22</td>
    <td>TFmini-S RX</td>
    <td>UART transmit</td>
    <td>LiDAR</td>
  </tr> 
             <tr>
    <td>GPIO17</td>
    <td>Raspberry Pi TX</td>
    <td>UART receive</td>
    <td>Navigation data from Pi</td>
  </tr> 
                <tr>
    <td>GPIO16</td>
    <td>Raspberry Pi RX</td>
    <td>UART transmit</td>
    <td>Navigation data from Pi</td>
  </tr>
</table>
</center>
  
## 🗺 Development Roadmap
The vessel hardware is shared across all phases; each phase adds capability to the electronics and control system.
-	**Phase 1: Remote-Controlled Vessel** (✅ This repository)
ESP32 primary processor, dual DC motor driver, PSU, battery and ESP-NOW remote controller.
-	**Phase 2: Autonomous Guidance** (🚧 Planned)
LiDAR obstacle avoidance, camera + Raspberry Pi running a YOLO detection model, and MEMS orientation/heading sensing. See details below.
-	**Phase 3: Swarm Operation** (🚧 Planned)
Multiple vessels share data and images over long-range transceivers to coordinate cleanup and avoid collisions, with no internet required.
-	**Phase 4: Advanced Autonomy** (🚧 Planned)
GPS geofencing, return-to-base, automatic charging and net-emptying decisions, and weather/boat-traffic awareness via modem.

**🔭 Phase 2 Guidance System (Planned)**
-	**LiDAR (e.g. TFmini-S)**: Connected directly to the ESP32 over TTL UART so collision avoidance never depends on the vision pipeline.
-	**Camera + Raspberry Pi (64-bit Raspberry Pi OS)**: Runs a YOLO object-detection model with PyTorch/torchvision, OpenCV and NumPy to classify floating debris vs. obstacles, estimate distance and heading, and plan an optimal collection path.
-	**MEMS Sensor**: Reports orientation, heading, acceleration and speed so the ESP32 can correct for currents, uneven loads or mismatched motors.
-	**Switchable Power Rails**: Separate PSUs for sensors and a dedicated 40 W PSU for the Raspberry Pi, each switched off by the ESP32 when not in use to save battery.

Behind a wrap-around transparent resin window at the bow, the LiDAR and camera see the front of the vessel.

## 📂 Project Structure & Key Files
- 🚤 **Robot.ino:** Firmware for the Marine Collection Vessel (ESP32). Receives encrypted ESP-NOW commands, drives the dual DC motor driver via PWM for differential thrust, and reports battery voltage. <i>The file must be placed in a folder with the same name</i>. 
- 🎮 **Controller.ino:** Firmware for the Remote Controller (ESP32-S3). Reads the joysticks and buttons, renders status and battery information on the TFT display, and transmits encrypted commands to the vessel. <i>The file must be placed in a folder with the same name</i>.

## IDE and Libraries
- Install the Arduino IDE 2.3.8 https://www.arduino.cc/en/software/
- From the boards manager, install the esp32 by Espressif Systems v3.1.3
- Install Adafruit GFX Library by Adafruit v1.12.6
- Install Adafruit ST7735 and ST7789 Library by Adafruit v1.11.0

## 🚀 Getting Started 

**Marine Collection Vessel (Robot.ino)**

1. Open 04_Code/Robot/Robot.ino in the Arduino IDE
2. From the board manager, select DOIT ESP32 DEVKIT V1
3. Update peerAddress[] with the Remote's MAC address
4. Update the encryption keys pmk_key[16] and lmk_key[16] with random numbers
5. Compile and upload

**Remote Controller (Controller.ino)**

1. Open 04_Code/Controller/Controller.ino in the Arduino IDE
2. From the board manager, select the ESP32S3 Dev Module
3. Enable USB CDC On Boot
4. Update peerAddress[] with the Vessel's MAC address
5. Update the encryption keys pmk_key[16] and lmk_key[16] with the same values used on the vessel
6. Compile and upload

##
<div align="center">
Use the <b>Marine Collection Vessel</b> to stop waste before it reaches the ocean.
</div>

