// <copyright file="Tracker.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#include <iostream>

#include "Tracker.h"

tracking::Tracker::Tracker(void)
	: m_initialised(false)
	, m_connected(false)
	, m_button_devices()
	, m_motion_devices()
	, m_active_node() {

	// intentionally empty...
}


tracking::Tracker::~Tracker(void) {

	this->Disconnect();

	for (auto& v : this->m_button_devices) {
		v.reset(nullptr);
	}
}


bool tracking::Tracker::Initialise(const tracking::Tracker::Params& params) {

	bool check = true;
	this->m_initialised = false;

	std::string active_node;
	try {
		active_node = std::string(params.active_node);
		if (active_node.length() != params.active_node_len) {
			std::cerr << std::endl << "[ERROR] [Tracker] String \"active_node\" has not expected length. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			check = false;
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "[ERROR] [Tracker] Error reading string param 'active_node': " << e.what() <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	this->m_button_devices.clear();
	std::vector<tracking::VrpnDevice<vrpn_Button_Remote>::Params> vrpn_params;
	try {
		for (size_t i = 0; i < params.vrpn_params_count; i++) {
			vrpn_params.emplace_back(params.vrpn_params[i]);
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "[ERROR] [Tracker] Error reading 'vrpn_params' array: " << e.what() <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (!this->m_motion_devices.Initialise(params.natnet_params)) {
		check = false;
	}

	if (check) {
		m_active_node = active_node;

		for (int i = 0; i < vrpn_params.size(); ++i) {
			this->m_button_devices.emplace_back(std::make_unique<tracking::VrpnButtonDevice>());
			if (!this->m_button_devices.back()->Initialise(vrpn_params[i])) {
				check = false;
			}
		}

		this->print_params();
		this->m_initialised = true;
	}

	return this->m_initialised;
}


void tracking::Tracker::print_params(void) {

	std::cout << "[PARAMETER] [Tracker] Active Node:                      " << ((this->m_active_node.empty()) ? ("<all>") : (this->m_active_node.c_str())) << std::endl;
}


bool tracking::Tracker::Connect(void) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [Tracker] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	// Terminate previous connection.
	this->Disconnect();

	// Check active node.
	std::string computerName;

	const size_t bufSize = 255;
	// Windows
#ifdef _WIN32 
	TCHAR infoBuf[bufSize];
	DWORD bufCharCount = bufSize;
	if (GetComputerName(infoBuf, &bufCharCount)) {
		computerName = infoBuf;
	}
	// Linux
#else
	char hostname[bufSize];
	gethostname(hostname, bufSize);
#endif /** _WIN32 */

	if (!this->m_active_node.empty() && (computerName != m_active_node)) {
		std::cout << std::endl << "[WARNING] [Tracker] Node \"" << computerName.c_str() << "\" is not enabled to receive tracker updates (otherwise set as active node)." << std::endl << std::endl;
		return false;
	}

	// Connect button devices.
	bool vrpn_con_status = true;
	for (auto& v : this->m_button_devices) {
		if (!v->Connect()) {
			this->Disconnect();
			vrpn_con_status = false;
		}
	}

	// Connect motion devices.
	bool natnetConStatus = true;
	if (!this->m_motion_devices.Connect()) {
		this->Disconnect();
		natnetConStatus = false;
	}

	this->m_connected = (vrpn_con_status && natnetConStatus);
	return this->m_connected;
}


bool tracking::Tracker::Disconnect(void) {

	for (auto& v : this->m_button_devices) {
		v->Disconnect();
	}
	this->m_motion_devices.Disconnect();

	this->m_connected = false;
	return true;
}


bool tracking::Tracker::GetData(const std::string& i_rigid_body, const std::string& i_button_device, tracking::Tracker::TrackingData& o_data) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [Tracker] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [Tracker] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

#ifdef TRACKING_DEBUG_OUTPUT
	std::cout << "[DEBUG] [Tracker] Requested: Button Device \"" << i_button_device.c_str() << "\" and Rigid Body \"" << i_rigid_body.c_str() << "\"." << std::endl;
#endif

	// Set data of requested rigid body
	o_data.rigid_body.orientation = this->m_motion_devices.GetOrientation(i_rigid_body);
	o_data.rigid_body.position = this->m_motion_devices.GetPosition(i_rigid_body);

	// Set data of requested button device 
	o_data.button = 0;
	for (auto& v : this->m_button_devices) {
		if (i_button_device == v->GetDeviceName()) {
			o_data.button = (v->GetButton());
			break; /// Break if button device is found.
		}
	}

	return true;
}
