# Rapidomize IoT Edge
Embedded C/C++ SDK to build your own IoT controller solutions for ESP32 based devices. 
Use it with Rapidomize IoT platform or with any MQTT server.


## H/W Models supported
- Generic ESP32 devices
- [IoT Edge Controller: rpz-d2x2t2ux-we](https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/)


# Features
### System Info
![System Info](/img/home.png)

### Wifi Setup
Refer to above guideline to setup WiFi.
![Wifi Setup](/img/wifi.png)

### MQTT Setup
Configure your MQTT broker (server) by setting the required topic, server IP addresses, client ID, server credentials and preferred publishing topic. The device publishes sensor messages to this topic.
![MQTT Setup](/img/mqtt.png)

### Peripheral H/W setup
You can first configure pins and values, then setup MQTT configurations to receive MQTT messages from the device, then you can enable the device by clicking the checkbox and clicking save.
![Peripherals](/img/peripherals.png)

### Firmware update - OTA & File
To update firmware using OTA (over the air) get the URL of the release binary, then enter it in the URL field in the OTA update section, click the update button and it will update the firmware and show you the progress, and once it is fully updated it will automatically reboot. 
To update firmware using a local file, first download the latest release from [the firmware updates page](https://github.com/rapidomize/rapidomize-sdk-embedded/), and upload the file using the file tab. Click update. Once it is fully updated it will automatically reboot and start with the new firmware.
![Firmware update](/img/fw.png)

### Local logs
Device events will be shows in event logs, including peripheral messages.
![Firmware update](/img/logs.png)

### Reset
You can reset the devices to its factory state. It will remove all the configurations & WiFi settings you have saved, aka it will clear the preferences flash.
![Factory Reset](/img/reset.png)


# TODO
- [ ] Additional peripherals (e.g. Ethernet, Relays, GSM, IR, ...etc)
- [ ] Secure Login
- [ ] Option to disable local AP.
- [ ] Command control
- [ ] Automatic OTA updates
- [ ] Makefile


## Firmware Installation (Flashing)
Firmware can be first time installed (Flashed) using following tools. Once the firmware is installed into your device, further updates can be installed using the [Firmware update - OTA & File feature](#firmware-update---ota--file). It allows you to install firmware from the release page using the firmware release URL or download a particular release to your local computer and install using the downloaded local firmware binary file.

### First Time installations
Before you start, download the latest release of the [rapidomize-sdk-embedded firmware](https://github.com/rapidomize/rapidomize-sdk-embedded/releases).
- Connect your device using a USB-C cable to your computer. 
- In case of, [IoT Edge Controller: rpz-d2x2t2ux-we](https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/), it needs to be powered up using the 12V power adopter. 
- You may need USB serial adapter drivers if it's not already installed in your computer: Install CH341SER USB driver. 
- Download one of the Flash Tool mentioned below.

If you are customizing the firmware, download the source release tarball and follow the developer documentation to customize the firmware as you need. 

#### Identify the Serial Port the device is connected to.

- Windows: Either use following command or use Device Manager > **Ports (COM & LPT)**
    ```
    wmic path Win32_SerialPort get DeviceID,Name
    ```
    ![Device Manager](/img/device-manager.png)

- Linux
    ```
    ls /dev/ttyUSB*
    ```        
    e.g.  /dev/ttyUSB0

- Mac OS
    ```
    ls /dev/tty.*
    ```
    e.g. /dev/tty.usbmodemXXXX



### Flashing with Esptool
[esptool](https://github.com/espressif/esptool) : The official flashing tool from Espressif. It is a Python-based open-source, platform-independent commandline utility for flashing. 

Steps to install the firmware:
- Download & extract the latest binary release for your operating system & CPU architecture from [esptool release page](https://github.com/espressif/esptool/releases). 
- OR if you have Python3 in your system, install it via `pip`.
```
pip install esptool
```

- Upload/Write Flash
```
esptool -p PORT -b 460800 --chip esp32 write-flash -z --flash-mode dio --flash-freq 40m --flash-size detect 0x10000 FIRMWARE_BINARY
```
where: 
- `PORT` - is the USB serial port identified above.
- `FIRMWARE_BINARY` - is the binary file yu have downloaded from the release page e.g. `rapidomize-sdk-embedded-0.7.5-b26.bin`

- You may also use Bootloader & Partitions if you have a device that had different Bootloader & Partitions previously. Download, additional binary files (bootloader.bin, partitions.bin, & boot_app0.bin) from the releases page.

```
esptool -p PORT -b 460800 --chip esp32 write-flash -z --flash-mode dio --flash-freq 40m --flash-size detect 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 rapidomize-sdk-embedded-0.7.5-b27.bin
```

After successfully executing the command, esptool will hard reset the device, causing it to reboot and run the new firmware.

#### Factory Reset using Esptool
Run following command to reset the device to it's factory state.
```
esptool --chip esp32 --port PORT erase_region 0x9000 0x6000
```

### Flashing with Flash Download Tool
If you have a computer with Windows you can use `Flash Download Tool` to flash the firmware.
- Download & extract the latest binary release [Flash Download Tool](https://dl.espressif.com/public/flash_download_tool.zip)
- Use the previoiusly downloaded firmware binary as follows and click `start` to install it.

    ![Firmware](/img/fw-flash-download.png)

- You may also download Bootloader & Partitions if you have a device that had different Bootloader & Partitions previously.

    ![Firmware, Bootloader & Partitions](/img/fw-flash-download-boot.png)

After successfully writing the firmware, device will reboot and run the new firmware.    

For more info on download tool, refer to [Flash Download Tool User Guide](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html)


## Monitoring
Once the flashing is done, device can be monitored using a serial monitor. Device events can also be monitored using the webUI `Logs` tab. Use one of the following tools to monitor you device via serial port.

- [Putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html) - serial monitor for Windows, Linux & MacOS
- Minicom - Linux 
- screen - MacOS terminal
- Developers can use e.g. VSCode Serial monitor plugin or Arduino IDE.

![Putty Windows](/img/putty-windows.png)

![Putty Linux](/img/putty-linux.png)



## Configuration

### Step 1: Connect to the device's WiFi network
- Power up the device either using USB-C cable or 12V power adopter if using [IoT Edge Controller: rpz-d2x2t2ux-we](https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/), 
- Device will automatically broadcast its own WiFi network with SSID `iot_edge` 
- Connect your phone/tab/computer to the `iot_edge` WiFi SSID network. 

### Step 2: Configure your WiFi credentials
- Open a web browser and go to http://192.168.4.1. You will see the IoT Edge web UI.
- From WiFi settings tab, select your WiFi network from the list of SSID shown and enter your WiFi password and click `Connect` button.
- Make sure you have your WiFi network setup with DHCP, so that device will be able to connect and obtain a new IP address.
- If successfully connected, it will show 'Success' message just above the tabs
- Refresh your browser to see the IP address it has obtained.
- Now you should be able to connect to the device using this new IP address on your WiFi network.

### Step 3: Access the device on your network
You should now be able to access the IoT Edge web UI by typing its new local IP address. 
- Go to MQTT tab and provide Rapidomize MQTT broker detail or your own MQTT broker connection details.
- Connect to MQTT broker by clicking 'Connect'. If successfully connected, it will show 'Success' message just above the tabs

### Step 4: Configure the device peripherals
Now you can configure the device peripherals as per your need. Refer to peripheral's data sheets for more information on required values for each peripheral.
There are few example peripherals in the default firmware build. If you need any other peripherals, you can customize the code and build a new version of firmware binary. Refer to developer documentation for more info.


# Status
version 0.7.5 - 'Dugong Weasel'

# Contributions?
Contributions are highly welcome. If you’re interested in contributing leave a note with your username.

# Policy for Security Disclosures
If you suspect you have uncovered a vulnerability, **contact us privately**, as outlined in our security policy document; we will immediately prioritize your disclosure.


# License
Apache 2.0
