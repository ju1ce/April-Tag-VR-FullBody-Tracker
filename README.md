## This guide is also availible in the folowing languages:

- ![汉语 (Simplified Chinese)](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/blob/master/%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87%E6%95%99%E7%A8%8B%EF%BC%88SimplifiedChineseTutorial%EF%BC%89.md)

## [>>Full Tutorial Here<<](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/wiki)

For the full setup guide, click on the link above to access the wiki!

## Having questions or issues?

Ask in the help channel of our discord server: https://discord.gg/g2ctkXB4bb

# April-Tag-VR-FullBody-Tracker

Full-body tracking in VR using AprilTag markers.

This is my second attempt at creating a full-body tracking system using fiducial markers. This should enable people to get fullbody tracking for free, using only a phone and some cardboard. It is possible to get pretty good tracking with trackers of sizes as small as 10cm and a PS eye camera of 640x480 resolution. Increasing the marker size or using a higher resolution and faster phone camera further improves tracking.

**NOTE: THIS IS A FREE AND OPEN SOURCE PROJECT. YOU DO NOT NEED DRIVER4VR FOR THIS!**

To use, you will have to make three trackers - one for each leg and one for hips. Using only leg trackers will not work in VRChat!

This version uses the much more accurate AprilTag system and includes many improvements to make the system easier to use, such as a GUI interface and a more straight forward calibration.

If you have any issues or encounter any bugs, feel free to open an issue on github or message me on discord: https://discord.gg/g2ctkXB4bb

The program can be downloaded from the [releases](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/releases) tab.

![demo](images/demo.gif)

Beatsaber demo: https://youtu.be/Akps-dH0EeA

### Short setup video:
I am too bad at making actual tutorials, but I did record a short video of me setting up everything. Its not a replacement for the tutorial bellow, but it may help you understand some of the steps better.

https://youtu.be/ncN8Vw_0DrE

## Build instructions:

**NOTE: THIS IS ONLY FOR DEVELOPERS. IF YOU ONLY WANT TO USE APRILTAGTRACKERS AND NOT WRITE CODE, THE TUTORIAL IS ON THE [WIKI](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/wiki)**

The project is a CMake project. You should be able to build it either using CMake and your favourite IDE/compiler, or some IDEs already support opening cmake projects directly.

### Linux prerequisites
```
sudo apt install build-essentials libgtk-3-dev
```
### Windows prerequisites
Open in Visual Studio, or use the Visual Studio Command Prompt.


### Clone and build
```
git clone --recurse-submodules https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker
cd April-Tag-VR-FullBody-Tracker
cmake -B build
cmake --build build --target install
```

That should be it! In case you try it before a more detailed guide is up, we are always there to help on the discord server! (link above)

## Sources
Olson, Edwin. "AprilTag: A robust and flexible visual fiducial system." 2011 IEEE International Conference on Robotics and Automation. IEEE, 2011.

https://github.com/AprilRobotics/apriltag

WxWidgets: A Cross-Platform GUI Library

https://github.com/wxWidgets/wxWidgets
