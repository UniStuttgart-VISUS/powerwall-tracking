// <copyright file="VrpnButtonDevice.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#include <atomic>
#include <thread>
#include <iostream>

#include "VrpnButtonDevice.h"

tracking::VrpnButtonDevice::VrpnButtonDevice(void) : tracking::VrpnDevice<vrpn_Button_Remote>()
, m_initialised(false)
, m_connected(false)
, m_run_thread_loop(false)
, m_button(0) {

	// intentionally empty...
}


tracking::VrpnButtonDevice::~VrpnButtonDevice(void) {

	this->Disconnect();
}


bool tracking::VrpnButtonDevice::Initialise(const tracking::VrpnDevice<vrpn_Button_Remote>::Params& params) {

	this->m_initialised = tracking::VrpnDevice<vrpn_Button_Remote>::Initialise(params);

	return this->m_initialised;
}


bool tracking::VrpnButtonDevice::Connect(void) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [VrpnButtonDevice] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	// Establish connection.
	if (!tracking::VrpnDevice<vrpn_Button_Remote>::Connect()) {
		return false;
	}

	// Register handle for button changes.
	this->Register<vrpn_BUTTONCHANGEHANDLER>(&VrpnButtonDevice::on_button_changed, this);

	// Start vrpn main loop thread.
	std::cout << "[INFO] [VrpnButtonDevice] Starting VRPN main loop thread for \"" << this->GetDeviceName().c_str() << "\"" << std::endl;
	this->m_run_thread_loop.store(true);
	std::thread thread([this]() {
		while (this->m_run_thread_loop.load()) {
#ifdef TRACKING_DEBUG_OUTPUT
			//std::cout << "[DEBUG] [VrpnButtonDevice] Inside VRPN main loop ..." << std::endl;
#endif
			this->MainLoop();
			std::this_thread::yield();
		}
		});
	thread.detach();

	this->m_connected = true;

	return true;
}


bool tracking::VrpnButtonDevice::Disconnect(void) {

	// End main loop thread
	this->m_run_thread_loop.store(false);

	this->m_connected = false;

	return tracking::VrpnDevice<vrpn_Button_Remote>::Disconnect();
}


tracking::Button tracking::VrpnButtonDevice::GetButton(void) const {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [VrpnButtonDevice] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [VrpnButtonDevice] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	return this->m_button.load();
}


void VRPN_CALLBACK tracking::VrpnButtonDevice::on_button_changed(void* userData, const vrpn_BUTTONCB vrpnData) {

	auto that = static_cast<VrpnButtonDevice*>(userData);
	if (that == nullptr) {
		std::cerr << std::endl << "[ERROR] [VrpnButtonDevice] Invalid user data. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return;
	}

	// Remember the button state.
	tracking::Button m_button = that->m_button.load();
	tracking::Button mask = (1 << vrpnData.button);
	if (vrpnData.state != 0) {
		that->m_button.store(m_button |= mask);
	}
	else {
		that->m_button.store(m_button &= ~mask);
	}
#ifdef TRACKING_DEBUG_OUTPUT
	std::cout << "[DEBUG] [VrpnButtonDevice] Button = " << vrpnData.button << " | State = " << ((mask & (1 << vrpnData.button)) ? (1) : (0)) << std::endl;
#endif
}
