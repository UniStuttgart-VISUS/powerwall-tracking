// <copyright file="test.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

// Defines the entry point for the console application.  


#include <iomanip>
#include <iostream>

#include "TrackingUtilizer.h"

using namespace std;

/**** HOWTO: ******************************************************************
*
* 1) Start 'Motive' software on NatNet server 'mini'
* 2) Start VRPN server on VRPN server 'mini' (C:\vrpn\start64.bat)
* 3) Start this test program: .\test.exe
*
******************************************************************************/
/// Info: Powerwall Resolution: 10.800 x 4.096 pixel | Aspect Ratio: 2,64

int main() {

	// TRACKING INITIALISATION ////////////////////////////////////////////////

	/// Tracker Parameters
	tracking::Tracker::Params tp;
	std::string active_node = ""; // Allowing all clients to access tracking data
	tp.active_node = active_node;
	std::string client_ip = "129.69.205.76"; // = MINYOU
	tp.natnet_params.client_ip = client_ip;
	std::string server_ip = "129.69.205.86"; // = MINI
	tp.natnet_params.server_ip = server_ip;
	tp.natnet_params.cmd_port = 1510;
	tp.natnet_params.data_port = 1511;
	tp.natnet_params.con_type = tracking::NatNetDevicePool::ConnectionType::UniCast;
	tp.natnet_params.verbose_client = false;

	tracking::VrpnDevice<vrpn_Button_Remote>::Params bp;
	std::string device_name = "ControlBox"; // Leave empty to disable use of VRPN device
	bp.device_name = device_name;
	bp.server_ip = server_ip;
	bp.port = 3884;
	bp.protocol = tracking::VrpnDevice<vrpn_Button_Remote>::Protocols::VRPN_TCP;

	std::vector<tracking::VrpnDevice<vrpn_Button_Remote>::Params> bps;
	bps.emplace_back(bp);

	tp.vrpn_params = bps.data();
	tp.vrpn_params_count = bps.size();

	/// ----- Tracker -----
	// Handles all communication with NatNet and VRPN.
	// Creating one (!) Tracker which runs separate thread for receiving tracking data.
	auto tracker = std::make_shared<tracking::Tracker>();
	if (!tracker->Initialise(tp)) {
		std::cerr << std::endl << "[ERROR] [test] Unable to initialise <Tracker>. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return 1;
	}
	if (!tracker->Connect()) {
		std::cerr << std::endl << "[ERROR] [test] Failed to establish <Tracker> connection. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return 1;
	}
	size_t rigiBodyCount = tracker->GetRigidBodyCount();
	std::vector<std::string> rigidBodies;
	for (size_t i = 0; i < rigiBodyCount; i++) {
		rigidBodies.emplace_back(std::string(tracker->GetRigidBodyName(i)));
	}

	// TRACKING UTILIZERS /////////////////////////////////////////////////////

	/// TrackingUtilizer Parameters
	tracking::TrackingUtilizer::Params tup;
	tup.rigid_body_name = nullptr; // Will be set in line 116 depending on what rigid bodies are available.
	device_name = bp.device_name; // Insert bp.device_name only for rigid body you want to use.
	tup.btn_device_name = device_name;
	tup.select_btn = 0;
	tup.rotate_btn = 1;
	tup.translate_btn = 2;
	tup.zoom_btn = 3;
	tup.invert_rotate = true;
	tup.invert_translate = true;
	tup.invert_zoom = true;
	tup.rotate_speed = 1.0f;
	tup.translate_speed = 1.0f;
	tup.zoom_speed = 1.0f;
	tup.single_interaction = false;
	tup.fov_mode = tracking::TrackingUtilizer::FovMode::WIDTH_AND_ASPECT_RATIO;
	tup.fov_height = 0.2f;
	tup.fov_width = 0.2f;
	tup.fov_horiz_angle = 60.0f;
	tup.fov_vert_angle = 30.0f;
	tup.fov_aspect_ratio = tracking::TrackingUtilizer::FovAspectRatio::AR_1_77__1; // 16:9
	tup.physical_height = 2.4f;
	tup.physical_width = 6.0f;
	tup.physical_origin = glm::vec3(-3.0f, 0.3f, 0.0f);
	tup.physical_x_dir = glm::vec3(1.0f, 0.0f, 0.0f);
	tup.physical_y_dir = glm::vec3(0.0f, 1.0f, 0.0f);

	/// ----- Tracking Utilizers -----
	// Manage tracking data for one rigid body each.
	// Creating a TrackingUtilizer for each found rigid body (and corresponding button device).
	// Initialising TrackingUtilizer with exisiting Tracker.
	std::vector<tracking::TrackingUtilizer> utilizers;
	utilizers.clear();
	for (auto& rb : rigidBodies) {
		tup.rigid_body_name = rb;
		utilizers.emplace_back(tracking::TrackingUtilizer());
		if (!utilizers.back().Initialise(tup, tracker)) {
			std::cerr << std::endl << "[ERROR] [test] Failed to initialise <TrackingUtilizer>. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			return 1;

		}
	}

	// LOOP ///////////////////////////////////////////////////////////////////

	unsigned int btn;
	float pos_x, pos_y, pos_z;
	float orient_x, orient_y, orient_z, orient_w;
	float inters_x, inters_y;
	bool state;

	bool exit = false;
	while (!exit) {

		// Get current tracking data for each rigid body managed by the TrackingUtilizers.
		for (auto& tu : utilizers) {

			// Button State
			state = tu.GetRawData(btn, pos_x, pos_y, pos_z, orient_x, orient_y, orient_z, orient_w);
			std::cout << std::fixed << std::setprecision(4) <<
				"[INFO] [test] BUTTON-DEVICE \"" << tu.GetButtonDeviceName() << "\" - STATE (valid = "
				<< ((state) ? ("TRUE") : ("FALSE")) << ") ";
			if (state) {
				std::cout << " - " << btn;
			}
			std::cout << std::endl;

			// Intersection
			state = tu.GetIntersection(inters_x, inters_y);
			std::cout << std::fixed << std::setprecision(4) <<
				"[INFO] [test] RIGID-BODY \"" << tu.GetRigidBodyName() << "\" - INTERSECTION (valid = "
				<< ((state) ? ("TRUE") : ("FALSE")) << ") ";
			if (state) {
				std::cout << " - Coordinates: (" << inters_x << "," << inters_y << ") ";
			}
			std::cout << std::endl;

		}
		std::cout << std::endl;

		// Wait for user input (press 'ESC') to end loop
		std::cout << "[Press 'ESC' to exit program]" << std::endl << std::endl;
		int key = 0x1B; // ESC
		if (GetAsyncKeyState(key)) {
			exit = true;
		}

		// Sleep some ms to limit data output
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}
