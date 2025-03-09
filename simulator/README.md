# osmin simulation tool
The tool has been designed to test osmin indoor, on an Unix desktop. The tool will instantiate its own tracker connected to virtual sensors (compass and position).
Then the user only has to start the application, which will connect to the tracker already available, that is to say the one provided by the simulator. It is therefore important to start the simulator before the application to be tested.
The simulator allows via the command line, to modify the azimuth and the position provided to the application, and this in a transparent way. It is then possible to test the application in real conditions. It allows among other things, to load a GPX file, and to launch the RUN of track at the desired speed and tick.

## Testing osmin using the simulator

Starting the simulation tool, you will facing the command prompt. 
```
./osmin-simulator

Version 1.12.10 compiled on Sep 27 2024 at 15:52:29
Type HELP, for a list of commands.
>>>
```

At this time, you can launch the osmin application, and type HELP, for a list of commands.
```
>>> help
Commands:
HELP                       Print this help
EXIT                       Quit the simulator
STATUS                     Print current state
GOTO lat lon [alt]         Move to position
LEFT                       Rotate left
RIGHT                      Rotate right
ANGLE deg                  Rotate at angle of deg
MOVE [dm]                  Move forward dm or 0 (meters)
PAUSE [sec]                Pause for a tick or duration (1..59 seconds)
BREAK                      Stop playback of the running script
LOAD gpx                   Load the GPX file
LIST                       List all tracks contained in the loaded file
RUN trkid [speed [pts]]    Run the identified track of the loaded file
RUN                        Resume the stopped run
PLAY script                Play the script file
PLAY                       Resume the stopped playback

>>>
```

Set the virtual position (latitude, longitude and optionally the elevation).
```
>>> goto 45.918858 6.869745 1040
>>> status
Pos 45.91886 6.86974 Alt 1040 meters Ang 0°00'00.00"
```
On osmin, the main button is now green. Press it so it's blue, and adjust the zoom.

By typing the commands below, osmin should follows the movement.
```
>>> angle 45
>>> move 5
>>> move 5
>>> right
>>> move
```
The tracker should calculate the current speed based on the time interval between the two commands "move 5", then send the value to osmin. Above a speed of 3 km/h, the azimuth is estimated on the movement. So, typing "right" will not change the direction shown by osmin if the speed is higher than 3 km/h. The last command "move" (i.e. move 0) makes the speed zero, and forces the compass angle to be used as the azimuth.
 
To test a RUN from file GPX, first you have to load the file, and then start running of a selected track. You could specify only the track Id. Below, I set the speed and the start point. A RUN can be stopped by pressing the key CTRL+C, and resumed typing RUN without argument.
```
>>> load test.gpx
.................................................
File load succeeded.
Path: test.gpx
Name: 2024-07-24T18:55:19
Track 1: 2043 pts, 15 km [Track]
```

The GPX track must contain time data. Otherwise, the simulator will use a fake 1sec interval. The following sample will launch the run of the track 1 with speed 1.0 and starting from the point 1029.
```
>>> run 1 1.0 1029
1029: Pos 42.83893 2.47883 Alt 418 meters Ang 0°00'00.00"
[TRANSITION 1030] 42,838930 2,478830 418
[TRANSITION 1030] 42,838930 2,478830 418
1030: Pos 42.83893 2.47883 Alt 418 meters Ang 0°00'00.00"
[TRANSITION 1031] 42,838950 2,478868 418
[TRANSITION 1031] 42,838970 2,478906 418
[TRANSITION 1031] 42,838990 2,478944 418
[TRANSITION 1031] 42,839010 2,478982 418
1031: Pos 42.83903 2.47902 Alt 418 meters Ang 0°00'00.00"
[TRANSITION 1032] 42,839035 2,479032 418
[TRANSITION 1032] 42,839040 2,479043 418
[TRANSITION 1032] 42,839045 2,479055 418
[TRANSITION 1032] 42,839050 2,479067 418
[TRANSITION 1032] 42,839055 2,479078 417
1032: Pos 42.83906 2.47909 Alt 417 meters Ang 0°00'00.00"
[TRANSITION 1033] 42,839057 2,479092 418
[TRANSITION 1033] 42,839055 2,479095 418
[TRANSITION 1033] 42,839052 2,479097 418
1033: Pos 42.83905 2.47910 Alt 419 meters Ang 0°00'00.00"
^CRun is stopped
```

Between 2 track points, the simulator will be able to create some calculated positions, to ensure the GPS sensor ticks as in real life.

Finally, the simulator can play a scenario. This is a script file containing the list of commands to play, including the inclusion of other scripts, or the execution of GPX tracks. The playback of a scenario is interruptible and resumable, just like running a track.

