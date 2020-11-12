# April-Tag-VR-FullBody-Tracker
Full-body tracking in VR using AprilTag markers.

This is my second attemt at creating a full-body tracking system using fiducial markers. This version uses the much more accurate AprilTag system, and includes
many improvements to make the system easier to use.

This program allows you to get working full-body tracking in SteamVR using some printed markers, some cardboard, and either a USB camera or a phone.

## Connecting a camera

The first step is connecting a camera feed to you computer. This step is probably the most complex, since you will have to find out what works best for you.
Each of the methods has its pros and cons, so try them out and see what works best.

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

## Using the program

When we establish a connection to a camera, we can now start using the program.

# TODO




