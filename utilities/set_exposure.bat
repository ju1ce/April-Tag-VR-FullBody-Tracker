::Set your IP webcam ip bellow. Do not append http:// or /video here!

set addr=192.168.1.102:8090

::Set the wanted exposure. Set this to as low as you can while the image is still bright enough.
::In general, 10000 is enough for regular usage, while 5000 will allow extremely fast movement.

set exposure=7000

::Set connection timeout. If you're bothered by CMD file staying open for long even when you're not using an IP webcam, reduce this. Default is 4 seconds.

set connectionTimeOut=4
Echo "Looking for IP Webcam... Timeout is %connectionTimeOut% seconds"

::Check if the IP webcam is alive.
curl -s -I --connect-timeout %connectionTimeOut% %addr% >nul

::Apply values if IP webcam was found. Otherwise exit immediately so CMD doesn't wait on curl to finish, obstructing view.
if errorlevel 1 (
	Echo "IP Webcam not found, exiting."
	timeout /T 2
	exit
	) else ( 
		curl %addr%/settings/manual_sensor?set=on
		::Change ISO here if your camera supports something higher without getting noisy. It'll help you get a lower exposure value by increasing light sensitivity. Common alternatives are 1600, 3200 etc.

		curl %addr%/settings/iso?set=1000
		curl %addr%/settings/exposure_ns?set=%exposure%000
		curl %addr%/settings/focusmode?set=off
		curl %addr%/settings/focus_distance?set=1
		)