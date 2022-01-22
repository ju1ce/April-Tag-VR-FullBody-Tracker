# Building AprilTag VR FullBody Tracking

## Linux
This is easy! First, clone the project with `git clone --recurse-submodules https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker`. Then, from the root of the project, simply run `cmake -B build && cmake --build build`.

## Windows
#### Build dependency OpenCV
Clone opencv/opencv and opencv/opencv_contrib from github and build opencv with the `aruco` module.
You need to build the debug and release verions and install them on top of eachother inorder to create a debug build of ATT.
Recommended to use version `4.5.3`.
#### Build ATT
Clone with `git clone --recurse-submodules https://github.com/ju1ce/April-Tag-VR-FullBody-Tracker`
From the root of the project run 
```
cmake -B build -DOpenCV_DIR="C:/path/to/opencv/install"
cmake --build build --config Release # or Debug
cmake --install build --config Release --prefix "C:/path/to/install/" # Places the exe in `bin` folder
```
To run the exe, copy the shared libraries from the release package,
or the equivalent ones you have built,
to the bin folder with your exe.
