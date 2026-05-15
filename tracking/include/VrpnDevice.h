// <copyright file="VrpnDevice.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2026 Visualisierungsinstitut der Universit�t Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#ifndef TRACKING_VRPNDEVICE_H_INCLUDED
#define TRACKING_VRPNDEVICE_H_INCLUDED

#include <string>
#include <sstream>
#include <iostream>

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"

namespace tracking {

	/***************************************************************************
	*
	* Template for connecting to remote VRPN device.
	*
	***************************************************************************/
	template <class R> class VrpnDevice {

	public:

		/** Supported network protocols for VRPN. */
		enum Protocols {
			VRPN_TCP = 0,
			VRPN_UDP = 1
		};

		/** Data structure for setting PARAMETERs as batch. */
		struct Params {
			std::string                                 device_name;     /** The VRPN button device name. */
			std::string                                 server_ip;       /** The VRPN server IP address.        */
			unsigned int                                port;            /** The VRPN port.               */
			typename tracking::VrpnDevice<R>::Protocols protocol;        /** The VRPN protocol.           */
		};

		///////////////////////////////////////////////////////////////////////

		/**
		* CTOR
		*/
		VrpnDevice(void);

		/**
		* Initialisation.
		*
		* @return True for success, false otherwise.
		*/
		bool Initialise(const typename tracking::VrpnDevice<R>::Params& params);

		/**
		* DTOR
		*/
		~VrpnDevice(void);

		/**
		* Connect to VRPN device.
		*
		* @return True on success, false otherwise.
		*/
		virtual bool Connect(void);

		/**
		* Disconnect from VRPN device
		*
		* @return True on success, false otherwise.
		*/
		virtual bool Disconnect(void);

		/**********************************************************************/
		// GET

		inline std::string GetDeviceName(void) const {
			return this->m_device_name;
		}

	protected:

		/**
		* Register callback function which is called by vrpn.
		*
		* @param handler  Handler of callback function.
		* @param userData Pointer to class (this), which registers callback.
		*
		* @return True on success, false otherwise.
		*/
		template <typename H> bool Register(H handler, void* userData);

		/**
		* Starting main loop of VRPN device.
		*
		* @return True on success, false otherwise.
		*/
		bool MainLoop(void);

	private:

		/***********************************************************************
		* variables
		**********************************************************************/

		bool m_initialised;
		bool m_connected;
		std::unique_ptr<R> m_remote_device;

		/** PARAMETERs ********************************************************/

		/**
		* Button device name.
		* The name of the device (defined in vrpn cfg file).
		*/
		std::string m_device_name;

		/**
		* VRPN server name.
		* The sever IP address or server host name running vrpn and hosting the device.
		*/
		std::string m_server_ip;

		/**
		* The port used for connecting to the vrpn server.
		* Defined in following file on server: \\mini\c$\vrpn\start64.bat
		*/
		unsigned int m_port;

		/**
		* The protocol used for connecting to the vrpn server.
		*/
		typename VrpnDevice<R>::Protocols m_protocol;

		/***********************************************************************
		* functions
		**********************************************************************/

		/** Print used PARAMETER values. */
		void print_params(void);

	};

} /** end namespace tracking */


/// Template classes must be declared AND defined in the header file.


template <class R>
tracking::VrpnDevice<R>::VrpnDevice(void)
	: m_initialised(false)
	, m_connected(false)
	, m_remote_device(nullptr)
	, m_device_name("ControlBox")
	, m_server_ip("129.69.205.86")
	, m_port(3884)
	, m_protocol(VrpnDevice<R>::Protocols::VRPN_TCP) {

	// intentionally empty...
}


template <class R>
bool tracking::VrpnDevice<R>::Initialise(const typename VrpnDevice<R>::Params& params) {

	bool check = true;
	this->m_initialised = false;

	if (params.device_name.empty()) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Parameter \"device_name\" must not be empty string. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
	}

	if (params.server_ip.empty()) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Parameter \"server_ip\" must not be empty string. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (params.port >= 65535) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Parameter \"port\" must be less than 65535. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (check) {
		this->m_device_name = params.device_name;
		this->m_server_ip = params.server_ip;
		this->m_port = params.port;
		this->m_protocol = params.protocol;

		this->print_params();
		this->m_initialised = true;
	}

	return this->m_initialised;
}


template <class R>
void tracking::VrpnDevice<R>::print_params(void) {
	std::cout << "[PARAMETER] [VrpnDevice] Device Name:                   " << this->m_device_name.c_str() << std::endl;
	std::cout << "[PARAMETER] [VrpnDevice] Server Name:                   " << this->m_server_ip.c_str() << std::endl;
	std::cout << "[PARAMETER] [VrpnDevice] Port:                          " << this->m_port << std::endl;
	std::cout << "[PARAMETER] [VrpnDevice] Protocol:                      " << (int)this->m_protocol << std::endl;
}


template <class R>
tracking::VrpnDevice<R>::~VrpnDevice(void) {

	this->Disconnect();
}


template <class R>
bool tracking::VrpnDevice<R>::Connect(void) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	std::string url;

	// Terminate previous connection.    
	this->Disconnect();

	// Translate protocol to string
	std::string prot = "";
	switch (this->m_protocol) {
		case(VrpnDevice<R>::Protocols::VRPN_TCP): prot = "tcp";  break;
		case(VrpnDevice<R>::Protocols::VRPN_UDP): prot = "udp";  break;
		default: break;
	}

	/// URL: [device_name]@[protocol]://[server_ip]:[port]
	std::ostringstream str;
	str << this->m_device_name.c_str() << "@" << prot.c_str() << "://" << this->m_server_ip.c_str() << ":" << this->m_port;
	url = str.str();

	std::cout << "[INFO] [VrpnDevice] Connecting to VRPN server: " << url.c_str() << std::endl;

	vrpn_Connection* connection = vrpn_get_connection_by_name(
		url.c_str()
#ifdef TRACKING_VRPN_DEVICE_WRITE_PLAYBACKLOG
		, (this->deviceName + "_local_in.log").c_str(), (this->deviceName + "_local_out.log").c_str(),
		(this->deviceName + "_remote_device_in.log").c_str(), (this->deviceName + "_remote_device_out.log").c_str()
#endif
	);

	if (connection->doing_okay() == vrpn_false) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Failed to connect to VRPN server. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		connection->removeReference(); /// Adjust reference count manually.
		return false;
	}

	/// Create remote_device object calling CTOR with make_unique() for type R (e.g. vrpn_Button_remote_device, vrpn_Tracker_remote_device)
	this->m_remote_device = std::make_unique<R>(url.c_str(), connection);
	this->m_remote_device->shutup = true;
	std::cout << "[INFO] [VrpnDevice] >>> AVAILABLE BUTTON DEVICE: \"" << this->m_device_name.c_str() << "\"" << std::endl;

	std::cout << "[INFO] [VrpnDevice] Successfully connected to VRPN server." << std::endl;

	connection->removeReference(); /// Adjust reference count manually.

	this->m_connected = true;

	return true;
}


template <class R>
bool tracking::VrpnDevice<R>::Disconnect(void) {

	if (!this->m_remote_device) {
		return false;
	}

	// Disconnecting (= deletion) is handled by vrpn_ConnectionManager which 
	// is called in DTOR of vrpn_Connection. Deleting the remote device object will 
	// also delete the connection object and this is the way to disconnect.
	// The callbacks are unregistered in this way, too. Resetting unique_ptr 
	// to nullptr will delete the m_remote_device object and calls the other DTORs recursively.
	this->m_remote_device.reset(nullptr);

	std::cout << "[INFO] [VrpnDevice] Successfully disconnected from VRPN server." << std::endl;

	this->m_connected = false;

	return true;
}


template <class R> template <typename H>
bool tracking::VrpnDevice<R>::Register(H handler, void* userData) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	if (!this->m_remote_device) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] No remote device present. Call 'Connect()' prior to 'Register()'. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	if (userData == nullptr) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Pointer to userData is NULL. " << __FILE__ << " " << __LINE__ << std::endl << std::endl;
		return false;
	}

	return (this->m_remote_device->register_change_handler(userData, handler) == 0);
}


template <class R>
bool tracking::VrpnDevice<R>::MainLoop(void) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	if (!this->m_remote_device) {
		std::cerr << std::endl << "[ERROR] [VrpnDevice] No remote device present. Call 'Connect()' and 'Register()' prior to 'MainLoop()'. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	try {
		this->m_remote_device->mainloop();
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "[ERROR] [Tracker] Error executing remote device main loop: " << e.what() <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		this->Disconnect();
		return false;
	}
	catch (...) {
		std::cerr << std::endl << "[ERROR] [Tracker] Unknown error executing remote device main loop. " <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		this->Disconnect();
		return false;
	}

	return true;
}


#endif /** TRACKING_VRPNDEVICE_H_INCLUDED */
