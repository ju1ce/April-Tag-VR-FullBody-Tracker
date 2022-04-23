## This guide is also available in the following languages:

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

This version uses the much more accurate AprilTag system and includes many improvements to make the system easier to use, such as a GUI interface and a more straightforward  calibration.

If you have any issues or encounter any bugs, feel free to open an issue on github or message me on discord: https://discord.gg/g2ctkXB4bb

The program can be downloaded from the [releases](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/releases) tab.

![demo](images/demo.gif)

Beatsaber demo: https://youtu.be/Akps-dH0EeA

### Short setup video:
I am too bad at making actual tutorials, but I did record a short video of me setting up everything. It's not a replacement for the tutorial below, but it may help you understand some of the steps better.

https://youtu.be/ncN8Vw_0DrE

## Build instructions:

**NOTE: THIS IS ONLY FOR DEVELOPERS. IF YOU ONLY WANT TO USE APRILTAGTRACKERS AND NOT WRITE CODE, THE TUTORIAL IS ON THE [WIKI](https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker/wiki)**

The project is a CMake project. You should be able to build it either using CMake and your favourite IDE/compiler, or some IDEs already support opening cmake projects directly.

### Linux prerequisites
```
sudo apt install build-essentials libgtk-3-dev
```

Note that since this project is mostly developed on windows, some additions may occasionally break linux compatibility. It can usually be solved by commenting out the additions, or just rolling back a few commits.


### Windows prerequisites
Open in Visual Studio, or use the Visual Studio Command Prompt.


### Clone and build
```
git clone https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker
cd April-Tag-VR-FullBody-Tracker
cmake -B build
cmake --build build --target install
```

That should be it! In case you try it before a more detailed guide is up, we are always there to help on the discord server! (link above)

### Visual Studio

Visual Studio is a bit picky about external projects, so some additional steps are necessary:

1. Git clone with all submodules
2. Generate Visual Studio solution files with CMake
3. Open build/April-Tag-VR-FullBody-Tracker.sln in VS
4. Build it, both in Debug and Release configurations. This will build and configure all dependencies. Note that you cannot run/debug ATT from here, so don't be alarmed if you get a file not found or access denied error when it attempts to do so.
5. When done, close the solution, and open build/ApriltagTrackers/ApriltagTrackers.sln in VS
6. The project is now configured! You should be able to change code, build, run and debug ATT from this solution.

### Troubleshooting

If anything goes wrong when building, please notify us in the discord/via a github issue!

## Sources
Olson, Edwin. "AprilTag: A robust and flexible visual fiducial system." 2011 IEEE International Conference on Robotics and Automation. IEEE, 2011.

https://github.com/AprilRobotics/apriltag

WxWidgets: A Cross-Platform GUI Library

https://github.com/wxWidgets/wxWidgets
