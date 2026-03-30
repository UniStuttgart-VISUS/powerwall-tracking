// <copyright file="NatNetDevicePool.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#include <iostream>

#include "NatNetDevicePool.h"

tracking::NatNetDevicePool::NatNetDevicePool(void)
	: m_initialised(false)
	, m_connected(false)
	, m_natnet_client(nullptr)
	, m_rigid_bodies()
	, m_callback_counter(0)
	, m_rigid_body_names()
	, m_client_ip("129.69.205.76") // minyou
	, m_server_ip("129.69.205.86") // mini
	, m_cmd_port(1510)
	, m_data_port(1511)
	, m_con_type(NatNetDevicePool::ConnectionType::UniCast)
	, m_verbose_client(false) {

	// intentionally empty...

}


bool tracking::NatNetDevicePool::Initialise(const NatNetDevicePool::Params& params) {

	bool check = true;
	this->m_initialised = false;
	this->m_natnet_client = nullptr;

	std::string client_ip;
	try {
		client_ip = std::string(params.client_ip);
		if (client_ip.length() != params.client_ip_len) {
			std::cerr << std::endl << "[ERROR] [NatNetClient] String \"client_ip\" has not expected length. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			check = false;
		}
		if (client_ip.empty()) {
			std::cerr << std::endl << "[ERROR] [NatNetClient] Parameter \"client_ip\" must not be empty string. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			check = false;
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "[ERROR] [Tracker] Error reading string param 'active_node': " << e.what() <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	std::string server_ip;
	try {
		server_ip = std::string(params.server_ip);
		if (server_ip.length() != params.server_ip_len) {
			std::cerr << std::endl << "[ERROR] [NatNetClient] String \"server_ip\" has not expected length. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			check = false;
		}
		if (server_ip.empty()) {
			std::cerr << std::endl << "[ERROR] [NatNetClient] Parameter \"server_ip\" must not be empty string. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			check = false;
		}
	}
	catch (const std::exception& e) {
		std::cerr << std::endl << "[ERROR] [Tracker] Error reading string param 'active_node': " << e.what() <<
			" [" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (params.cmd_port >= 65535) {
		std::cerr << std::endl << "[ERROR] [NatNetClient] Parameter \"cmd_port\" must be less than 65535. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (params.data_port >= 65535) {
		std::cout << std::endl << "[ERROR] [NatNetClient] Parameter \"data_port\" must be less than 65535. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		check = false;
	}

	if (check) {
		this->m_callback_counter = 0;
		this->m_client_ip = client_ip;
		this->m_server_ip = server_ip;

		this->m_cmd_port = params.cmd_port;
		this->m_data_port = params.data_port;
		this->m_con_type = params.con_type;
		this->m_verbose_client = params.verbose_client;

		this->print_params();
		this->m_initialised = true;
	}

	return this->m_initialised;
}


void tracking::NatNetDevicePool::print_params(void) {

	std::cout << "[PARAMETER] [NatNetDevicePool] Client IP:               " << this->m_client_ip.c_str() << std::endl;
	std::cout << "[PARAMETER] [NatNetDevicePool] Server IP:               " << this->m_server_ip.c_str() << std::endl;
	std::cout << "[PARAMETER] [NatNetDevicePool] Command Port:            " << this->m_cmd_port << std::endl;
	std::cout << "[PARAMETER] [NatNetDevicePool] Data Port:               " << this->m_data_port << std::endl;
	std::cout << "[PARAMETER] [NatNetDevicePool] Connection Type:         " << (int)this->m_con_type << std::endl;
	std::cout << "[PARAMETER] [NatNetDevicePool] Verbose NatNet client:   " << ((this->m_verbose_client) ? ("yes") : ("no")) << std::endl;
}


tracking::NatNetDevicePool::~NatNetDevicePool(void) {

	this->Disconnect();
}


bool tracking::NatNetDevicePool::Connect(void) {

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
	}

	::sServerDescription  server_desc;
	::sDataDescriptions* data_desc = nullptr;
	::ErrorCode           error_code = ::ErrorCode_OK;

	// Terminate previous connection.
	this->Disconnect();

	// Print local natnet version.
	//unsigned char        version[4];
	//::NatNet_GetVersion(version);
	//std::cout << "[info] [NatNetDevicePool] Local NatNet version: " << version[0] << "." << version[1] << "." << version[2] << "." << version[3] << std::endl;

	// Get connection parameters
	::sNatNetClientConnectParams connect_params;
	connect_params.connectionType = static_cast<::ConnectionType>(this->m_con_type);
	connect_params.localAddress = this->m_client_ip.c_str();
	connect_params.serverAddress = this->m_server_ip.c_str();
	connect_params.serverCommandPort = static_cast<uint16_t>(this->m_cmd_port);
	connect_params.serverDataPort = static_cast<uint16_t>(this->m_data_port);
	if (connect_params.connectionType == ::ConnectionType::ConnectionType_Multicast) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] MultiCast is currently not supported. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return false;
		//connect_params.multicastAddress = "224.0.0.1";
	}

	// Create the natnet client.
	this->m_natnet_client = std::make_unique<::NatNetClient>();

	std::cout << "[INFO] [NatNetDevicePool] Connecting to NatNet server ..." << std::endl;
	error_code = this->m_natnet_client->Connect(connect_params);

	// Check whether the connection was successful.
	if (error_code == ::ErrorCode_OK) {

		// Reset server description struct and get new server description.
		memset(&server_desc, 0, sizeof(server_desc));
		this->m_natnet_client->GetServerDescription(&server_desc);

		if (!server_desc.HostPresent) {
			std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Disconnecting. No NatNet host is present. " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
			this->m_natnet_client.reset(nullptr);
			return false;
		}

		// Print some information on connection.
		std::cout << "[INFO] [NatNetDevicePool] Successfully connected to NatNet server: " << server_desc.szHostComputerName << " - IP: " <<
			(int)server_desc.HostComputerAddress[0] << "." << (int)server_desc.HostComputerAddress[1] << "." <<
			(int)server_desc.HostComputerAddress[2] << "." << (int)server_desc.HostComputerAddress[3] << std::endl;

		std::cout << "[INFO] [NatNetDevicePool] NatNet host application: " << server_desc.szHostApp << " " <<
			(int)server_desc.HostAppVersion[0] << "." << (int)server_desc.HostAppVersion[1] << "." <<
			(int)server_desc.HostAppVersion[2] << "." << (int)server_desc.HostAppVersion[3] << std::endl;

		std::cout << "[INFO] [NatNetDevicePool] Server side NatNet version: " << (int)server_desc.NatNetVersion[0] << "." <<
			(int)server_desc.NatNetVersion[1] << "." << (int)server_desc.NatNetVersion[2] << "." <<
			(int)server_desc.NatNetVersion[3] << std::endl;
	}
	else {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Failed to connect to NatNet server. - NATNET ERROR CODE: " << (int)error_code << " (see NatNetTypes.h, line 115). " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		this->m_natnet_client.reset(nullptr);
		return false;
	}

	// Register callback handlers.
	std::cout << "[INFO] [NatNetDevicePool] Registering callbacks ..." << std::endl;
	this->m_natnet_client->SetFrameReceivedCallback(NatNetDevicePool::on_data, const_cast<NatNetDevicePool*>(this));
	if (this->m_verbose_client) {
		NatNet_SetLogCallback(NatNetDevicePool::on_message);
	}

	// Send remote command(s)
	float* frResponse = nullptr;
	int    cntResponse = 0;
	// FrameRate
	error_code = this->m_natnet_client->SendMessageAndWait("FrameRate", (void**)&frResponse, &cntResponse);
	if (error_code == ErrorCode_OK) {
		std::cout << "[INFO] [NatNetDevicePool] NatNet remote command test PASSED. Current framerate: " << (*frResponse) << std::endl;
	}
	else {
		std::cerr << std::endl << "[WARNING] [NatNetDevicePool] Unable to process NatNet framerate request. - NATNET ERROR CODE: " << (int)error_code << " (see NatNetTypes.h, line 115). " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		this->m_natnet_client.reset(nullptr);
		return false;
	}

	// Look up rigid body data descriptions.
	this->m_rigid_body_names.clear();
	std::cout << "[INFO] [NatNetDevicePool] Looking up rigid bodies ..." << std::endl;
	error_code = this->m_natnet_client->GetDataDescriptionList(&data_desc);
	if (error_code == ErrorCode_OK) {
		for (int i = 0; i < data_desc->nDataDescriptions; ++i) {
			if (data_desc->arrDataDescriptions[i].type == Descriptor_RigidBody) { // DataDescriptors
				auto* rb = data_desc->arrDataDescriptions[i].Data.RigidBodyDescription;
				if (rb == nullptr) {
					std::cout << std::endl << "[WARNING] [NatNetDevicePool] Empty rigid body description. " <<
						"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
					continue;
				}
				// Create new lock free ring for rigid body data
				this->m_rigid_bodies.emplace_back(std::make_shared<RigidBody>(rb->ID, rb->szName));
				this->m_rigid_body_names.emplace_back(this->m_rigid_bodies.back()->name);

				std::cout << "[INFO] [NatNetDevicePool] >>> PROVIDED RIGID BODY \"" << this->m_rigid_bodies.back()->name.c_str() << "\"." << std::endl;
			}
		}
	}
	else {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Unable to retrieve rigid body data descriptions. - NATNET ERROR CODE: " << (int)error_code << " (see NatNetTypes.h, line 115). " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		this->Disconnect();
		return false;
	}

	this->m_connected = true;

	return true;
}


bool tracking::NatNetDevicePool::Disconnect(void) {

	if (this->m_natnet_client != nullptr) {
		::ErrorCode error_code = this->m_natnet_client->Disconnect();
		this->m_natnet_client.reset(nullptr);
		if (error_code == ErrorCode_OK) {
			std::cout << "[INFO] [NatNetDevicePool] Successfully disconnected from NatNet server." << std::endl;
		}
		else {
			std::cerr << "[ERROR] [NatNetDevicePool] Disconnected from NatNet server. - NATNET ERROR CODE: " << (int)error_code << " (see NatNetTypes.h, line 115). " <<
				"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		}
	}

	// Clear rigid body data after disconnecting (otherwise callback for natnet might still be accessing data).
	for (unsigned int i = 0; i < this->m_rigid_bodies.size(); ++i) {
		this->m_rigid_bodies[i].reset();
	}
	this->m_rigid_bodies.clear();

	this->m_connected = false;

	return true;
}


glm::quat tracking::NatNetDevicePool::GetOrientation(const std::string& rigid_body) {

	glm::quat ret_orientation((std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)());

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return ret_orientation;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return ret_orientation;
	}

	// Check for updated data
#ifdef TRACKING_DEBUG_OUTPUT
	std::cout << "[DEBUG]  [NatNetDevicePool] Callback Counter = " << this->m_callback_counter << std::endl;
	if (this->m_callback_counter <= 0) {
		this->m_callback_counter--;
		if (this->m_callback_counter < -10) {
			std::cout << std::endl << "[DEBUG] [NatNetDevicePool] Didn't receive updated tracking data yet. " <<
				">>> Please check your firewall settings if this warning appears repeatedly! ." << std::endl;
		}
	}
	else {
		this->m_callback_counter = 0;
	}
#endif

	for (auto& it : this->m_rigid_bodies) {
		if (rigid_body == it->name) {
			ret_orientation = it->lockFreeData[it->read.load()].orientation;
			break;
		}
	}

	return ret_orientation;
}


glm::vec3 tracking::NatNetDevicePool::GetPosition(const std::string& rigid_body) {

	glm::vec3 ret_position((std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)(),
		(std::numeric_limits<float>::max)());

	if (!this->m_initialised) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Not initialised. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return ret_position;
	}
	if (!this->m_connected) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Not connected. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return ret_position;
	}

	// Check for updated data
#ifdef TRACKING_DEBUG_OUTPUT
	std::cout << "[DEBUG]  [NatNetDevicePool] Callback Counter = " << this->m_callback_counter << std::endl;
	if (this->m_callback_counter <= 0) {
		this->m_callback_counter--;
		if (this->m_callback_counter < -10) {
			std::cout << std::endl << "[DEBUG] [NatNetDevicePool] Didn't receive updated tracking data yet. " <<
				">>> Please check your firewall settings if this warning appears repeatedly! ." << std::endl;
		}
	}
	else {
		this->m_callback_counter = 0;
	}
#endif

	for (auto& it : this->m_rigid_bodies) {
		if (rigid_body == it->name) {
			ret_position = it->lockFreeData[it->read.load()].position;
			break;
		}
	}

	return ret_position;
}



void __cdecl tracking::NatNetDevicePool::on_data(sFrameOfMocapData* pFrameOfData, void* pUserData) {

	auto that = static_cast<NatNetDevicePool*>(pUserData);
	if ((pFrameOfData == nullptr) || (that == nullptr)) {
		std::cerr << std::endl << "[ERROR] [NatNetDevicePool] Pointer to userData is NULL. " <<
			"[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
		return;
	}

#ifdef TRACKING_DEBUG_OUTPUT
	// Simple counter to be able to check if callback has been called
	that->m_callback_counter++;
	// Prevent overflow
	if ((that->m_callback_counter > ((std::numeric_limits<int>::max)() - 2)) ||
		(that->m_callback_counter < ((std::numeric_limits<int>::min)() + 2))) {
		that->m_callback_counter = 0;
	}
#endif

	for (int i = 0; i < pFrameOfData->nRigidBodies; ++i) {
		// All zero seems to be an indicator that the rigid body is not
		// visible at the moment. Skip ...
		bool is_valid = (
			(pFrameOfData->RigidBodies[i].qx != 0.0f)
			|| (pFrameOfData->RigidBodies[i].qy != 0.0f)
			|| (pFrameOfData->RigidBodies[i].qz != 0.0f)
			|| (pFrameOfData->RigidBodies[i].qw != 0.0f)
			|| (pFrameOfData->RigidBodies[i].x != 0.0f)
			|| (pFrameOfData->RigidBodies[i].y != 0.0f)
			|| (pFrameOfData->RigidBodies[i].z != 0.0f)
			);
#ifdef TRACKING_DEBUG_OUTPUT
		//std::cout << "[DEBUG] [NatNetDevicePool] ID = " << pFrameOfData->RigidBodies[i].ID << " MeanError = " << pFrameOfData->RigidBodies[i].MeanError <<
		//    "; Params = " << pFrameOfData->RigidBodies[i].params << "; Orientation = " << pFrameOfData->RigidBodies[i].qx << ", " <<
		//    pFrameOfData->RigidBodies[i].qy << ", " << pFrameOfData->RigidBodies[i].qz << ", " << pFrameOfData->RigidBodies[i].qw <<
		//    "; Position = " << pFrameOfData->RigidBodies[i].x << ", " << pFrameOfData->RigidBodies[i].y << ", " << pFrameOfData->RigidBodies[i].z << "; (is_valid = " << is_valid << ")." << std::endl;
#endif

		if (is_valid) {
			sRigidBodyData data = pFrameOfData->RigidBodies[i];

			// Set data for current rigid body
			for (auto& it : that->m_rigid_bodies) {
				if (data.ID == it->id) {

					// Write new data to object with index denoted as 'write'
					it->lockFreeData[it->write.load()].orientation.x = data.qx;
					it->lockFreeData[it->write.load()].orientation.y = data.qy;
					it->lockFreeData[it->write.load()].orientation.z = data.qz;
					it->lockFreeData[it->write.load()].orientation.w = data.qw;

					it->lockFreeData[it->write.load()].position.x = data.x;
					it->lockFreeData[it->write.load()].position.y = data.y;
					it->lockFreeData[it->write.load()].position.z = data.z;

					// Determine index of currently unused free object
					unsigned int free = (it->read.load() + 1) % 3;
					free = (it->write.load() == free) ? ((it->write.load() + 1) % 3) : (free);
#ifdef TRACKING_DEBUG_OUTPUT
					//std::cout << "[DEBUG] [NatNetDevicePool] READ = " << it->read.load() << " - WRTIE = " << it->write.load() << " - FREE = " << free << "." << std::endl;
#endif
					// Swap lock free read/write buffers
					it->read.store(it->write.load());
					it->write.store(free);

					break; /// Break loop if rigid body is found
				}
			}
		}
	}
}


void __cdecl tracking::NatNetDevicePool::on_message(Verbosity level, const char* message) {

	switch (level) {
	case (::Verbosity::Verbosity_None):    break;
	case (::Verbosity::Verbosity_Debug):   std::cout << "[DEBUG] [NatNetClient] " << message << std::endl; break;
	case (::Verbosity::Verbosity_Info):    std::cout << "[INFO] [NatNetClient] " << message << std::endl; break;
	case (::Verbosity::Verbosity_Warning): std::cout << "[WARNING] [NatNetClient] " << message << std::endl; break;
	case (::Verbosity::Verbosity_Error):   std::cout << "[ERROR] [NatNetClient] " << message << std::endl; break;
	default: break;
	}
}

