
::set your IP webcam ip bellow. Do not append http:// or /video here!

set addr=192.168.1.102:8080

::set the wanted exposure. Set this to as low as you can while the image is still bright enough.
::in general, 10000 is enough for regular usage, while 5000 will allow extremely fast movement.

set exposure=7000


curl %addr%/settings/manual_sensor?set=on
curl %addr%/settings/iso?set=1000
curl %addr%/settings/exposure_ns?set=%exposure%000
curl %addr%/settings/focusmode?set=off
curl %addr%/settings/focus_distance?set=1
