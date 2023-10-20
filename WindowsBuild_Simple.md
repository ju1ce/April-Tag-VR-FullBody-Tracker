Compile steps for Windows:

__1-__ Download and install Visual Studio 2022 17.8 Preview 2 or newer [(This includes a Msys fix we need)](https://github.com/microsoft/vcpkg/issues/31565#issuecomment-1723267213), check enable building for C/C++ desktop applications during installation.

__2-__ Copy paste this into the address bar of your browser, to clone and open the master branch in VS2022. When it finishes, confirm Retargeting as shown.
```
git-client://clone?repo=https%3A%2F%2Fgithub.com%2Fju1ce%2FApril-Tag-VR-FullBody-Tracker
```

![image](https://github.com/Skyrion9/April-Tag-VR-FullBody-Tracker/assets/74653117/59955543-1596-4e4a-b804-5a1c8000b3cc)




We'll switch to Folder view so VS will recognize the project properly. Click on the leftmost icon in Solution Explorer, and click on Folder View.

![image](https://github.com/Skyrion9/April-Tag-VR-FullBody-Tracker/assets/74653117/6c675ca3-c60d-46e8-9c72-ffb421c78895)






__4-__ Let it rip. Hopefully it'll finish without errors. When you get output : ```[CMake] -- Running vcpkg install - done```, proceed to next step. If you got CXX compiler errors after that line ignore those. VS is trying to use an incorrect install build configuration (of its own making). Remember you can CTRL+F and search for the output mentioned here, as the output window can get cluttered. What's important is vcpkg install finishing.

If you got ```1> [CMake] CMake Error at scripts/cmake/vcpkg_acquire_msys.cmake:150 (file):```

You didn't install the preview version, did you? Go back and do it you lazy sloth you.

__5-__ In VS, hit View > Terminal and paste the following code into the Developer PowerShell:
```
cmake -B build
cmake --build build --config Release --target install
```

You can find the program in "Install" folder if it compiled successfully.

## FAQ (Probably?)
There are still ongoing fixes from Microsoft's side, which you won't find out about until you run into an error. Those have been pending for months. This is why Preview version is recommended as it has a fix we need, when the stable version is updated to 17.8 or newer you won't need this.

__1-__ I still can't compile

Either use a precompiled exe from Release, or make sure to follow each step exactly, Without alternating to different IDEs etc. Pay attention to Output window and fix anything missing. It's usually a vcpkg bug. In case of missing directory errors, double check your Windows Environment path and add any missing paths that show up in the output. Remember to restart VS and regenerate cache so path changes are recognized.

Keep in mind, if you get errors at step 4 you can still try step 5. We only need VS to prepare the dependencies for us. It might fail to build, but we bypass this by using the powershell terminal command. You still need to make sure it succeeds in installing vcpkg in the very least ```[CMake] -- Running vcpkg install - done```

__1.1-__ If you've an error related to Powershell, I recommend installing Powershell 7. Add it to your User ENV path (Regardless of installation ENV setting. As it adds to System path, but I had to manually add it to User ENV. Alternatively, running VS as admin might fix this and break some other things.) Sometimes VS installer clears up your path.

Same path addition should fix the default Powershell if you want to skip PS7.

__1.2-__ [There's a bug where Vcpkg can't install/find Msys](https://github.com/microsoft/vcpkg/issues/31565#issuecomment-1723267213). If your VS 2022 version is older than 17.8, you'll have this error. It is however fixed in 17.8 Preview 2 or newer. 

__2-__ I've an error, how can I get help?

Double check your steps and make sure you've read through all of this. Reminder Visual Studio installer sometimes resets PATH environment variable when installing in an attempt to tidy it up. If CMake fails to recognize something you've installed, this is probably the reason. You must add anything missing to Path manually.

Otherwise try [ApriltagVR's discord](https://discord.gg/g2ctkXB4bb). You'll get better help there. 

__3-__ Wow you've read this far? Here's a little reward for you.
```
@@echo off
start "" "%~dp0AprilTagTrackers.exe"
call "%~dp0\utilities\set_exposure.bat"
exit
```
Optional batch script to start Apriltag and apply IP Webcam settings simultaneously, unnecessary if you don't use IP Webcam.
Create a notepad file, copy paste the above code in, rename it to "APTGVR_IPCam.bat" file and move it next to AprilTagTrackers.exe and run it as Administrator (Make a shortcut and in properties > shortcut > advanced enable it)

It'll automatically apply your parameters configured within utilities\set_exposure.bat and launch APTVR, so make sure to update that batch file with your IP Camera parameters first!
