# ADSB-Telemetry-Decoder ✈️📡

A C++ based ADS-B telemetry decoding engine using RTL-SDR, featuring a custom-calculated RF antenna and live LCD coordinate tracking.

## 📖 Project Overview
This project captures live ADS-B (Automatic Dependent Surveillance-Broadcast) signals from aircraft at 1090 MHz. We process the raw RF static into a clean digital stream using C++ and display real-time global coordinates on a dedicated LCD screen. 

## ✨ Key Features
* **Custom Antenna Design:** Specifically calculated and physically constructed for optimal 1090 MHz reception.
* **C++ Decoding Engine:** Custom DSP (Digital Signal Processing) pipeline to extract and decode 112-bit telemetry packets.
* **Live LCD Tracking:** Real-time display of aircraft coordinates (Latitude/Longitude) and flight data.
* **Hardware Integration:** Seamless connection between the RTL-SDR dongle, processing unit, and I2C LCD.

## 🛠️ Hardware & Setup
* RTL-SDR Dongle
* Custom-built 1090 MHz Antenna
* I2C LCD Display 
* Processing PC

## 👥 Contributors
* Sasiru Yasith Rasanjana - Development & RF Engineering
* Isuri - Hardware Integration & System Architecture
