# Dual-Core FreeRTOS Sensor Telemetry Hub

A real-time sensor monitoring project built using the ESP32, FreeRTOS, and Embedded C/C++. This project demonstrates multitasking, dual-core task execution, sensor data acquisition, and telemetry monitoring for embedded systems.

## 🚀 Live Simulation

**Run the project on Wokwi:** [Dual-Core FreeRTOS Sensor Telemetry Hub](https://wokwi.com/projects/478585811328244737)

## 📌 Project Overview

The Dual-Core FreeRTOS Sensor Telemetry Hub demonstrates how an ESP32-based embedded system can organize sensor monitoring and telemetry operations using FreeRTOS tasks.

The project focuses on real-time task scheduling, sensor data processing, and modular embedded software development.

## 🎯 Objectives

- Understand FreeRTOS task creation and scheduling.
- Explore dual-core execution on the ESP32.
- Implement sensor data acquisition and monitoring.
- Display sensor telemetry through serial output.
- Learn real-time embedded system design principles.

## 🛠️ Technologies Used

- **Microcontroller:** ESP32
- **Programming Language:** Embedded C/C++
- **RTOS:** FreeRTOS
- **Simulation:** Wokwi
- **Communication:** UART Serial Monitor

## ⚙️ Key Features

- FreeRTOS-based multitasking.
- ESP32 dual-core architecture.
- Periodic sensor data acquisition.
- Sensor telemetry monitoring.
- Modular embedded software design.
- Virtual hardware testing using Wokwi.

## 🏗️ System Architecture

```text
             ESP32
               |
        FreeRTOS Scheduler
               |
       +-------+-------+
       |               |
  Sensor Task      Telemetry Task
       |               |
  Read Sensors     Process Data
       |               |
       +-------+-------+
               |
         Serial Monitor
```

*Note: Confirm the actual task assignments and data flow against your source code.*

## 🔄 Working Principle

1. Initialize the ESP32 and required peripherals.
2. Create the necessary FreeRTOS tasks.
3. Read sensor data at configured intervals.
4. Process the acquired readings.
5. Display telemetry information through the serial monitor.
6. Use task scheduling to manage concurrent operations.

## ▶️ How to Run

1. Open the [Wokwi Live Simulation](https://wokwi.com/projects/478585811328244737).
2. Click the **Start Simulation** button.
3. Open the serial monitor if required.
4. Observe the sensor readings and task output produced by the firmware.

## 🧠 Concepts Demonstrated

- Embedded C/C++ programming
- Real-Time Operating Systems
- FreeRTOS task scheduling
- ESP32 architecture
- Sensor interfacing
- Serial communication
- Embedded system simulation

## 🔮 Future Enhancements

- MQTT-based IoT telemetry.
- Sensor data transfer using FreeRTOS queues.
- Sensor fault detection and alerts.
- Performance monitoring and task timing analysis.
- Real-time web dashboard integration.

## 📚 References

- [Wokwi ESP32 Simulator](https://wokwi.com/)
- [ESP-IDF FreeRTOS Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html)
- [Arduino ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)

## 👨‍💻 Author

**Suru Vishal**

Electronics and Communication Engineering (ECE)

**Project Simulation:** https://wokwi.com/projects/478585811328244737

---

⭐ If you find this project useful, consider starring the repository.
