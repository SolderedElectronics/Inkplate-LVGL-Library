# Soldered Inkplate LVGL library

[![Compile Sketches](https://github.com/e-radionicacom/Inkplate-Arduino-library/actions/workflows/compile.yml/badge.svg?branch=master)](https://github.com/e-radionicacom/Inkplate-Arduino-library/actions/workflows/compile.yml)

<p align="center">
  <img src="https://raw.githubusercontent.com/SolderedElectronics/Inkplate-Arduino-library/master/extras/InkplateImage.jpg">
</p>

The Soldered Inkplate LVGL Library brings full LVGL support to the Soldered Inkplate family of e-paper displays.
It provides ready-to-use integrations for display control, touch input (where available), and file system (FS) access, so you can focus entirely on designing and implementing your user interface.
All necessary LVGL initialization is handled internally through the library’s begin() function — meaning you can start building your UI right away without worrying about setup details.
Below are the color formats used by each Inkplate model:

| Board           | Color Format |
| --------------- | ------------ |
| Inkplate 2      | RGB565       |
| Inkplate 5V2    | L8           |
| Inkplate 6      | L8           |
| Inkplate 6Flick | L8           |
| Inkplate 6Color | RGB565       |
| Inkplate 10     | L8           |


### Setting up Inkplate in Arduino IDE

In order to get a head start with Inkplate (any), follow these steps:

1. [Install Inkplate board definition](https://github.com/SolderedElectronics/Inkplate-Board-Definitions-for-Arduino-IDE) - add Inkplate 2/5V2/6/6FLICK/6COLOR/10 as a board into your Arduino IDE. 
2. If you aren't using macOS install CH340 drivers (in case you don't have them yet) - instructions [here](https://soldered.com/learn/ch340-driver-installation-croduino-basic3-nova2/)
3. Install Inkplate Arduino library - Install the library from the Arduino Library Manager (just type "Inkplate" and click "Install") or install it with this repo. If you don't know how, check our [tutorial](https://soldered.com/learn/arduino-library/#Kako%20instaliraty%20library?).
4. You are ready to get started! Select Tools -> Board -> Inkplate Boards and then pick correct one, as well as correct COM port and upload!

### Code examples & LVGL Documentation

As this is currently a work in progress, all examples except the 'project' Inkplate examples have been ported! The projects are coming soon.

To see the entire LVGL documentation, please check out the [official LVGL documentation](https://docs.lvgl.io/master/).

### Documentation

Find Inkplate documentation [here](https://soldered.com/documentation/inkplate/). 

### Battery power

Inkplate boards has two options for powering it. First one is obvious - USB port at side of the board. Just plug any micro USB cable and you are good to go. Second option is battery. Supported batteries are standard Li-Ion/Li-Poly batteries with 3.7V nominal voltage. Connector for the battery is standard 2.00mm pitch JST connector (except on Inkplate 2, it uses SMD solder pads for battery terminals). The onboard charger will charge the battery with 500mA when USB is plugged at the same time. You can use battery of any size or capacity if you don't have a enclosure. If you are using our enclosure, battery size shouldn't exceed 90mm x 40mm (3.5 x 1.57 inch) and 5mm (0.19 inch) in height (excluding Inkplate 2, it uses [this battery](https://soldered.com/product/li-ion-baterija-600mah-3-7v/). [This battery](https://soldered.com/product/li-ion-battery-1200mah-3-7v/) is good fit for the Inkplate. Also, Inkplate's hardware is specially optimized for low power consumption in deep sleep mode, making it extremely suitable for battery applications.

#### ⚠️ WARNING
Please check the polarity on the battery JST connector! Some batteries that can be purchased from the web have reversed polarity that can damage Inkplate board! You are safe if you are using the pouch battery from [soldered.com](https://soldered.com/categories/power-sources-batteries/batteries/lithium-batteries/) or Inkplate with the built-in battery . 

#### ℹ NOTE
CR2032 battery is only for RTC backup. Inkplate cannot be powered with it.

### Open-source

All of Inkplate-related development is open-sourced:

- [Arduino library](https://github.com/SolderedElectronics/Inkplate-Arduino-library)
- Hardware design:
  - [Soldered Inkplate 2](https://github.com/SolderedElectronics/Soldered-Inkplate-2-hardware-design)
  - Soldered Inkplate 6 (comming soon!)
  - Soldered Inkplate 6PLUS (comming soon!)
  - [Soldered Inkplate 10](https://github.com/SolderedElectronics/Soldered-Inkplate-10-hardware-design)
  - Soldered Inkplate 6COLOR (comming soon!)
  - [e-radionica.com Inkplate 6](https://github.com/SolderedElectronics/Inkplate-6-hardware)
  - [e-radionica.com Inkplate 10](https://github.com/SolderedElectronics/Inkplate-10-hardware)
  - [e-radionica.com Inkplate 6PLUS](https://github.com/SolderedElectronics/Inkplate-6PLUS-Hardware)
  - e-radionica.com Inkplate 6COLOR (comming soon!)
- [micropython Inkplate](https://github.com/SolderedElectronics/Inkplate-micropython)
- [OSHWA cerfiticates](https://certification.oshwa.org/list.html?q=inkplate)

### Where to buy & other

Inkplate boards are available for purchase via:

- [soldered.com](https://soldered.com/categories/inkplate/)
- [Crowd Supply](https://www.crowdsupply.com/soldered)
- [Mouser](https://hr.mouser.com/Search/Refine?Keyword=inkplate)

For all questions and issues please reach us via [e-mail](mailto:hello@soldered.com) or our [contact form](https://soldered.com/contact/).
