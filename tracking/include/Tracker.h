// <copyright file="Tracker.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#ifndef TRACKING_TRACKER_H_INCLUDED
#define TRACKING_TRACKER_H_INCLUDED

#ifdef TRACKING_EXPORTS  
#define TRACKING_API __declspec(dllexport)   
#else  
#define TRACKING_API __declspec(dllimport)   
#endif  

#include "VrpnButtonDevice.h"
#include "NatNetDevicePool.h"

namespace tracking {

	/***************************************************************************
	*
	* Collects 6 DOF tracking data of rigid bodies (via NetNet) and
	* the states of VRPN button devices.
	*
	***************************************************************************/
	class TRACKING_API Tracker {

	public:

		/** Data structure for setting parameters as batch. */
		struct Params {
			std::string                                       active_node;      /** The name of the active node which should receive the tracking data exclusively. */
			tracking::VrpnDevice<vrpn_Button_Remote>::Params* vrpn_params;
			size_t                                            vrpn_params_count;
			tracking::NatNetDevicePool::Params                natnet_params;
		};

		/** Current tracking raw data. */
		struct TrackingData {
			tracking::NatNetDevicePool::RigidBodyData rigid_body;
			tracking::Button                          button;
		};

		///////////////////////////////////////////////////////////////////////

		/**
		* CTOR
		*/
		Tracker(void);

		/**
		* DTOR
		*/
		~Tracker(void);

		/**
		* Initialisation.
		*
		* @return True for success, false otherwise.
		*/
		bool Initialise(const tracking::Tracker::Params& params);

		/**
		* Callback for connection tracking.
		*
		* @return True for success, false otherwise.
		*/
		bool Connect(void);

		/**
		* Callback for disconnection tracking.
		*
		* @return True for success, false otherwise.
		*/
		bool Disconnect(void);

		/**********************************************************************/
		// GET

		/**
		* Get number of available rigid body names.
		*
		* @return All available rigid body names.
		*/
		inline size_t GetRigidBodyCount(void) {
			return this->m_motion_devices.GetRigidBodyNames().size();
		}

		/**
		* Get all available rigid body names.
		*
		* @return All available rigid body names.
		*/
		const char* GetRigidBodyName(size_t index) {
			if (index < this->m_motion_devices.GetRigidBodyNames().size()) {
				return this->m_motion_devices.GetRigidBodyNames()[index].c_str();
			}
			else {
				return nullptr;
			}
		}

		/**********************************************************************/
		// Only to be called by TrackingUtilizer (only use inside dll).

		/**
		*  Get current tracking data.
		*
		* @param i_rigid_body     The name of the rigid body getting data for
		* @param i_button_device  The name of the button device getting data for.
		* @param o_data           Returns the current tracking raw data.
		*
		* @return True for success, false otherwise.
		*/
		bool GetData(const std::string& i_rigid_body, const std::string& i_button_device, tracking::Tracker::TrackingData& o_data);

	private:

		/**********************************************************************
		* types
		**********************************************************************/

		typedef std::vector<std::unique_ptr<tracking::VrpnButtonDevice>> VrpnButtonPoolType;

		/**********************************************************************
		* variables
		**********************************************************************/

		bool m_initialised;
		bool m_connected;
		VrpnButtonPoolType m_button_devices;
		tracking::NatNetDevicePool m_motion_devices;

		/** parameters ********************************************************/

		/** Enables the tracker only on the node with the specified name. */
		std::string m_active_node;

		/**********************************************************************
		* functions
		**********************************************************************/

		void print_params(void);

		/** Read physical values from file. */
		bool read_params_from_file(tracking::Tracker::Params& params);

	};

} /** end namespace tracking */

#endif /** TRACKING_TRACKER_H_INCLUDED */