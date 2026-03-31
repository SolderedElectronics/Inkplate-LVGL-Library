# Soldered Inkplate LVGL library

[![Compile Examples](https://github.com/SolderedElectronics/Inkplate-LVGL-Library/actions/workflows/compile.yml/badge.svg)](https://github.com/SolderedElectronics/Inkplate-LVGL-Library/actions/workflows/compile.yml)
[![Arduino Library Manager Compatibility](https://github.com/SolderedElectronics/Inkplate-LVGL-Library/actions/workflows/arduinoCompatibility.yml/badge.svg)](https://github.com/SolderedElectronics/Inkplate-LVGL-Library/actions/workflows/arduinoCompatibility.yml)

<p align="center">
  <img src="https://raw.githubusercontent.com/SolderedElectronics/Inkplate-Arduino-library/master/extras/InkplateImage.jpg">
</p>

The Soldered Inkplate LVGL Library brings full LVGL support to the Soldered Inkplate family of e-paper displays.
It provides ready-to-use integrations for display control, touch input (where available), and file system (FS) access, so you can focus entirely on designing and implementing your user interface.
All necessary LVGL initialization is handled internally through the library’s begin() function — meaning you can start building your UI right away without worrying about setup details.
Below are the color formats used by each Inkplate model:

| Board              | Color Format |
| ------------------ | ------------ |
| Inkplate 2         | RGB565       |
| Inkplate 4TEMPERA  | RGB565       |
| Inkplate 5V2       | L8           |
| Inkplate 6         | L8           |
| Inkplate 6FLICK    | L8           |
| Inkplate 6COLOR    | RGB565       |
| Inkplate 10        | L8           |
| Inkplate 13SPECTRA | RGB565       |


### Source code file structure

```
Inkplate-LVGL-Library/
   |
   +--- src/
   |      |
   |      +--- Inkplate-LVGL.h          <-- Main library header, defines the Inkplate class
   |      +--- Inkplate.cpp             <-- Core class implementation (LVGL init, display control)
   |      +--- boardSelect.h            <-- Selects the correct board driver at compile time
   |      |
   |      +--- boards/                  <-- Per-board EPD driver code
   |      |      +--- Inkplate2/
   |      |      |      +--- Inkplate2BoardFile.h      <-- Board wrapper / class alias
   |      |      |      +--- Inkplate2Driver.cpp       <-- EPD driver implementation
   |      |      |      +--- Inkplate2Driver.h         <-- EPD driver class definition
   |      |      |      `--- pins.h                    <-- Pin definitions
   |      |      |
   |      |      +--- Inkplate4TEMPERA/
   |      |      |      +--- Inkplate4TEMPERABoardFile.h
   |      |      |      +--- Inkplate4TEMPERADriver.cpp
   |      |      |      +--- Inkplate4TEMPERADriver.h
   |      |      |      +--- pins.h
   |      |      |      `--- waveforms.h               <-- EPD waveform lookup tables
   |      |      |
   |      |      +--- Inkplate5V2/
   |      |      |      +--- Inkplate5V2BoardFile.h
   |      |      |      +--- Inkplate5V2Driver.cpp
   |      |      |      +--- Inkplate5V2Driver.h
   |      |      |      +--- pins.h
   |      |      |      `--- waveforms.h
   |      |      |
   |      |      +--- Inkplate6/
   |      |      |      +--- Inkplate6BoardFile.h
   |      |      |      +--- Inkplate6Driver.cpp
   |      |      |      +--- Inkplate6Driver.h
   |      |      |      +--- pins.h
   |      |      |      `--- waveforms.h
   |      |      |
   |      |      +--- Inkplate6COLOR/
   |      |      |      +--- Inkplate6COLORBoardFile.h
   |      |      |      +--- Inkplate6COLORDriver.cpp
   |      |      |      +--- Inkplate6COLORDriver.h
   |      |      |      `--- pins.h
   |      |      |
   |      |      +--- Inkplate6FLICK/
   |      |      |      +--- Inkplate6FLICKBoardFile.h
   |      |      |      +--- Inkplate6FLICKDriver.cpp
   |      |      |      +--- Inkplate6FLICKDriver.h
   |      |      |      +--- pins.h
   |      |      |      `--- waveforms.h
   |      |      |
   |      |      +--- Inkplate10/
   |      |      |      +--- Inkplate10BoardFile.h
   |      |      |      +--- Inkplate10Driver.cpp
   |      |      |      +--- Inkplate10Driver.h
   |      |      |      +--- pins.h
   |      |      |      `--- waveforms.h
   |      |      |
   |      |      `--- Inkplate13SPECTRA/
   |      |             +--- Inkplate13SPECTRABoardFile.h
   |      |             +--- Inkplate13SPECTRADriver.cpp
   |      |             +--- Inkplate13SPECTRADriver.h
   |      |             `--- pins.h
   |      |
   |      +--- features/               <-- Optional on-board peripherals
   |      |      +--- featureSelect.h  <-- Compile-time switches to enable/disable features
   |      |      +--- APDS9960/        <-- Gesture & proximity sensor (Inkplate 4TEMPERA)
   |      |      +--- BME680/          <-- Environmental sensor (Inkplate 4TEMPERA)
   |      |      +--- BQ27441/         <-- Battery fuel gauge
   |      |      +--- Buzzer/          <-- Buzzer driver (Inkplate 4TEMPERA)
   |      |      +--- LSM6DS3/         <-- IMU / accelerometer (Inkplate 4TEMPERA)
   |      |      +--- MCP4018/         <-- Digital potentiometer (frontlight control)
   |      |      +--- SdFat/           <-- Vendored SdFat library (SD card access)
   |      |      +--- frontlight/      <-- Frontlight PWM control
   |      |      +--- rtc/             <-- Onboard real-time clock
   |      |      +--- touchpad/        <-- Capacitive touchpad (legacy Inkplate models)
   |      |      `--- touchscreen/     <-- Touchscreen drivers (Cypress / Elan controllers)
   |      |
   |      +--- graphics/               <-- Pixel-level rendering helpers
   |      |      +--- GraphicsDefs.h   <-- Shared graphics type definitions
   |      |      +--- ditheringColor/  <-- Floyd-Steinberg dithering for color EPD panels
   |      |      `--- ditheringGrayscale/ <-- Floyd-Steinberg dithering for grayscale EPD panels
   |      |
   |      +--- lvgl/                   <-- LVGL integration layer
   |      |      +--- FS_driver_implementation.cpp  <-- LVGL SD filesystem driver
   |      |      +--- FS_driver_implementation.h
   |      |      +--- custom_allocation_algorithm.h <-- Custom LVGL memory allocator
   |      |      `--- src/             <-- Vendored LVGL v9 source
   |      |
   |      `--- system/                 <-- Low-level ESP32 system utilities
   |             +--- InkplateBoards.h         <-- Board detection and class selection
   |             +--- defines.h                <-- Global macros and constants
   |             +--- NetworkController/       <-- Wi-Fi connection helpers
   |             +--- UtilI2S/                 <-- I2S audio utility (Inkplate 4TEMPERA)
   |             +--- esp32/                   <-- ESP32-specific system functions
   |             `--- pcalExpander/            <-- PCAL6416A GPIO expander driver
   |
   +--- examples/                      <-- Arduino sketch examples, one folder per board
   |
   `--- README.md                      <-- This file
```

### Setting up Inkplate in Arduino IDE

In order to get a head start with Inkplate (any), follow these steps:

1. [Install Inkplate board definition](https://github.com/SolderedElectronics/Inkplate-Board-Definitions-for-Arduino-IDE) - add Inkplate 2/5V2/6/6FLICK/6COLOR/10 as a board into your Arduino IDE. 
2. If you aren't using macOS install CH340 drivers (in case you don't have them yet) - instructions [here](https://soldered.com/learn/ch340-driver-installation-croduino-basic3-nova2/)
3. Install Inkplate LVGL library - Install the library from the Arduino Library Manager (just type "Inkplate LVGL" and click "Install") or install it with this repo. If you don't know how, check our [tutorial](https://soldered.com/learn/arduino-library/#Kako%20instaliraty%20library?).
4. You are ready to get started! Select Tools -> Board -> Inkplate Boards and then pick correct one, as well as correct COM port and upload!

### Code examples & LVGL Documentation

As this is currently a work in progress, all examples except the 'project' Inkplate examples have been ported! The projects are coming soon.

To see the entire LVGL documentation, please check out the [official LVGL documentation](https://docs.lvgl.io/master/).

### Documentation

Find Inkplate documentation [here](https://soldered.com/documentation/inkplate/). 

### Battery power

Inkplate boards has two options for powering it. First one is obvious - USB port at side of the board. Just plug any micro USB cable and you are good to go. Second option is battery. Supported batteries are standard Li-Ion/Li-Poly batteries with 3.7V nominal voltage. Connector for the battery is standard 2.00mm pitch JST connector (except on Inkplate 2, it uses SMD solder pads for battery terminals). The onboard charger will charge the battery with 500mA when USB is plugged at the same time. You can use battery of any size or capacity if you don't have a enclosure. If you are using our enclosure, battery size shouldn't exceed 90mm x 40mm (3.5 x 1.57 inch) and 5mm (0.19 inch) in height (excluding Inkplate 2, it uses [this battery](https://soldered.com/product/li-ion-baterija-600mah-3-7v/). [This battery](https://soldered.com/product/li-ion-battery-1200mah-3-7v/) is good fit for the Inkplate. Also, Inkplate's hardware is specially optimized for low power consumption in deep sleep mode, making it extremely suitable for battery applications.

#### WARNING
Please check the polarity on the battery JST connector! Some batteries that can be purchased from the web have reversed polarity that can damage Inkplate board! You are safe if you are using the pouch battery from [soldered.com](https://soldered.com/categories/power-sources-batteries/batteries/lithium-batteries/) or Inkplate with the built-in battery . 

#### NOTE
CR2032 battery is only for RTC backup. Inkplate cannot be powered with it.

### About Soldered

<img src="https://raw.githubusercontent.com/SolderedElectronics/Soldered-IIS2DULPXTR-Accelerometer-Arduino-Library/dev/extras/Soldered-logo-color.png" alt="soldered-logo" width="500"/>

At Soldered, we design and manufacture a wide selection of electronic products to help you turn your ideas into acts and bring you one step closer to your final project. Our products are intented for makers and crafted in-house by our experienced team in Osijek, Croatia. We believe that sharing is a crucial element for improvement and innovation, and we work hard to stay connected with all our makers regardless of their skill or experience level. Therefore, all our products are open-source. Finally, we always have your back. If you face any problem concerning either your shopping experience or your electronics project, our team will help you deal with it, offering efficient customer service and cost-free technical support anytime.

### Where to buy & other

Inkplate boards are available for purchase via:

- [soldered.com](https://soldered.com/categories/inkplate/)
- [Crowd Supply](https://www.crowdsupply.com/soldered)
- [Mouser](https://hr.mouser.com/Search/Refine?Keyword=inkplate)

For all questions and issues please reach us via [e-mail](mailto:hello@soldered.com) or our [contact form](https://soldered.com/contact/).
