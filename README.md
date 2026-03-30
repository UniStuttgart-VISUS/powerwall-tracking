# Tracking Library `powerwall-tracking`

The `powerwall-tracking` library provides the interface that allows to communicate with the tracking system in front of the Powerwall at the Visualization Research Center (VISUS) of the University of Stuttgart.
This library can be used for interaction with tracking devices (rigid-bodies) and button devices (stick, glasses).
Additional detailed information and documentation can be found in the `documentation` folder of the repository.

---

## Building

Since the required library NatNet is not available for Linux and the library is only usable in combination with the Powerwall, it only works with Windows.
- [NatNet](http://optitrack.com/products/natnet-sdk/) is included in this package and it is required for this library to build. 
- [VRPN](https://github.com/vrpn/vrpn.git) is also required and it is automatically installed as submodule.
- [glm](https://github.com/g-truc/glm.git) is used to do all the math.

**NOTES:** 
- Building was tested with Visaul Studio 17 (2022) for x64 architecture.


## Interface Classes

The tracking library interface provides the classes `Tracker` and `TrackingUtilizer`. 

The `Tracker` class includes on the one hand a VRPN client. It receives updates from the VRPN server about the button states of the button device(s). On the other hand it handles the connection to the NatNet server (Motive software) via the included NatNet client. The NatNet server streams the spatial data of the available rigid bodies recorded by the tracking cameras.
While the available rigid bodies are provided by the NatNet server, all available button devices have to be defined explicitly in the designated parameter list of the `Tracker`.
Only one `Tracker` class should be declared at a time.

The `TrackingUtilizer` manipulates the raw data from the `Tracker`. By changing the orientation of a pointing device while pressing the associated button, camera parameters can be manipulated. Further the intersection of the pointing device with the powerwall as well as the field of view is provided (in relative screen space coordinates). The class also allows to acccess the raw tracking data.
Multiple `TrackingUtilizers` can be connected to the `Tracker` simultaneously. Each `TrackingUtilizer` utilizes only one rigid body (motion or pointing device) and and button device. They are defined by their names in the corresponding parameters.

## Test Program

* Configure and generate projects with `cmake`.
* Adjust client IP parameter in `test/src/test.cpp` in line 35.
* Build all projects using `INSTALL` target in Viusal Studio. 
* Start 'Motive' software on NatNet server `mini`.
* Start VRPN server (`C:/vrpn/start64.bat`) on VRPN server `mini`.
* Place rigid body inside of tracking area.
* Start test program: `bin/test.exe`.

The given default parameters in the example test program `test/src/test.cpp` fit the current VISUS tracking setup (Dezember 2019).

## Troubleshooting

### Network connection to NatNet and/or VPRN server fails
If the network connection to the NatNet and/or the VRPN server fails for the test program, make sure there are (windows defender) firewall rules allowing incoming traffic for the TCP port 3884 and the UDP ports 1510 and 1511.

### Receiving only zero valued tracking data
Switch from `Multicast` to `Unicast` in settings of 'Motive' software on NatNet server `mini`.

---
