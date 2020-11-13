# April-Tag-VR-FullBody-Tracker
Full-body tracking in VR using AprilTag markers.

This is my second attemt at creating a full-body tracking system using fiducial markers. This version uses the much more accurate AprilTag system, and includes
many improvements to make the system easier to use.

This program allows you to get working full-body tracking in SteamVR using some printed markers, some cardboard, and either a USB camera or a phone.

The program can be downloaded from the releases tab.

## Connecting a camera

The first step is connecting a camera feed to you computer. This step is probably the most complex, since you will have to find out what works best for you.
Each of the methods has its pros and cons, so try them out and see what works best. If you know of any other option, feel free to use that!

### Using a USB webcam:

#### Pros:
* Simple to setup
* High quality cameras will offer good performance(1080p 60fps)

#### Cons:
* Most cameras have a too low resolution and too much motion blur to use effectively

If you have a USB camera, you should try that first. If tracking is too bad, you can always switch to a phone later. A ps3 eye camera will work, but just barely due to its low resolution.

#### Tutorial:

Connect the camera to your PC and you are done! If you use a PS3 eye camera, also install the PS3 eye universal driver.

### Using IP-Webcam wireless:

#### Pros:
* Fairly simple to setup
* Plenty of video options

#### Cons:
* Requires a good wifi connection
* Your PC and phone must be connected to the same network
* Only for android phones

If you have your pc and android phone connected to the same router and you have a strong wifi connection on your phone, this is the option you should use.

#### Tutorial:

Download the app IP Webcam from the play store. Start the app. Under video preferences->video resolution, select the resolution you wish to use. You should try to use a 4:3 aspect ration with a resolution of around 800x600. Then, go back and click start server. Try to connect to your phone through your browser: click the help icon 
if you dont know how

### Using IP-Webcam wired:

#### Pros:
* Plenty of video options

#### Cons:
* A little harder to setup
* May not work on all phones and computers

If you dont have a good wifi connection, but have a half recent android phone, you should try this option. It may not work, however.

#### Tutorial:

For this we will use our phones network over USB feature. This is usualy used to share the phones network or wifi with a computer, but if we disable wifi and mobile network on our phone, we can also use it as a direct connection between our phone and PC. First disable wifi and mobile network. Then connect your phone to your PC with a USB cable. Now enable the internet over USB option on your phone. Now, you can follow the same instructions as for the wireless one!

NOTE: Make sure that your networks are disabled or this wont work!

### Using DroidCam OBS:

#### Pros:
* Should work on any device, including iphones
* Wired or wireless

#### Cons:
* Less video options than IP-Webcam
* Higher latency

If previous options dont work for you or you have an iphone, this is the option you should choose. It should work on any device, either wireless or wired.

#### Tutorial:

First, follow the DroidCam OBS official tutorial to get the phone-OBS connection. Use the 720p video resolution to ensure there is no watermark.
Then, follow the OBS VirtualCam plugin tutorial to stream to a virtual camera.
The phone will now act as a regular webcam.

## Making the markers

The next step is making the markers. Print the AprilTags.pdf page inside the Apriltag Trackers folder and cut the markers along the following lines:

Now, for each of the three trackers, cut out a piece of cardboard the same size. Bend it down the middle at a 90 degree angle and glue the marker onto it, like this:

Make sure you add some supports to ensure the tracker cant bend!

Tracker 0 will be used on our hips, and should use some additional supports.

Add some way of fixing the trackers to your body. I use some hooks, through which i can then fix the trackers using rubber bands.

NOTE: Make sure the pattern on the trackers is clearly visible and is not obstructed! Ensure the markers bend as little as possible!

## Installing the SteamVR driver

Inside the driver_files folder, there is a apriltagtrackers folder. Copy this folder to "Steam/steamapps/common/SteamVR/drivers". Now, open "Steam/config/steamvr.vrsettings" and, under "steamvr", add the field ```"activateMultipleDrivers" : true,``` .

This will ensure that every time we launch steamvr, it will attempt to connect to the ApriltagTrackers program through our driver.

## Running Apriltag Trackers

You can now run Start_ApriltagTrackers.bat! The first time you launch it, you may see a black console window for a few seconds.

You should now see the window with buttons to start diffrent parts of our program. First, lets set the most important parameters of our program on the second tab.

#### Ip or ID of camera:

If you have a webcam or OBS, this value will be a number, usualy 0, 1, or more if you have more cameras connected. Best way to figure out the correct index of the camera is to try them: Type in 0, press save, go back to Camera tab, check preview camera and press Start/Stop camera. If the correct camera shows up, great, you can go to the next step! If not, repeat the process with 1, then 2 etc until you find it.

If you use IP Webcam, you should enter your IP address, the same one as you used in your browser but ending with /video. The field should look something like this: ```http://192.168.1.100:8080/video``` but with some diffrent numbers.

#### Number of trackers:

The number of trackers you wish to use. For full body, you have to use 3.

#### Size of markers in cm:

Measure the size of your printed markers in cm, and input the value here. Measure like this:

#### Rotate camera 90°:

This will flip the camera view 90°. This will enable you to stand closer to the camera, which is usefull if you dont have much space or you have a low resolution camera.

#### Quad decimate:

This is the quality setting. The value can either be 1, 1.5, 2, 3 or 4. The higher is this value, the faster will the tracking be, but the range will decrease. You should first try to run the program on 1, and then increase the value if the tracking is too slow.

These were the settings important when setting up. I will explain the others later.

### Calibrating the camera

First, start the camera and disable the preview. Now press the Calibrate camera button. This will open two windows, one with a chessboard pattern, the other with the view from the camera. The camera will take a picture every few seconds. Try to film the chessboard pattern from as many angles as you can by slowly moving the camera around. When enough pictures are taken, you should get an alert that the camera has been calibrated.


# TODO




