// <copyright file="VrpnButtonDevice.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#ifndef TRACKING_VRPNBUTTONDEVICE_H_INCLUDED
#define TRACKING_VRPNBUTTONDEVICE_H_INCLUDED

#include <atomic>
#include <array> 

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "VrpnDevice.h"

namespace tracking {


	/// DEFINES ///////////////////////////////////////////////////////////////////

	// If defined, additional ouput for debugging ist written.
	///  #define TRACKING_DEBUG_OUTPUT

	// If defined, playback logs of vrpn device are written (see VrpnDevice.h line 222).
	///  #define TRACKING_VRPN_DEVICE_WRITE_PLAYBACKLOG 

	/// TYPES /////////////////////////////////////////////////////////////////////

	typedef unsigned int Button;

	typedef std::array<glm::vec2, 4> Rectangle;
	/// [0] = left_top
	/// [1] = left_bottom 
	/// [2] = right_top
	/// [3] = right_bottom


	/***************************************************************************
	*
	* VRPN button device.
	*
	***************************************************************************/
	class VrpnButtonDevice : public tracking::VrpnDevice<vrpn_Button_Remote> {

	public:

		/**
		* CTOR
		*/
		VrpnButtonDevice(void);

		/**
		* DTOR
		*/
		~VrpnButtonDevice(void);

		/**
		* Initialisation.
		*
		* @return True for success, false otherwise.
		*/
		bool Initialise(const tracking::VrpnDevice<vrpn_Button_Remote>::Params& params);

		/**
		* Connect to vrpn button device.
		*
		* @return True on success, false otherwise.
		*/
		bool Connect(void);

		/**
		* Disconnect from vrpn button device
		*
		* @return True on success, false otherwise.
		*/
		bool Disconnect(void);

		/**********************************************************************/
		// GET

		/**
		* Get current button states.
		*
		* @return The current button states.
		*/
		tracking::Button GetButton(void) const;

	private:

		/***********************************************************************
		* variables
		**********************************************************************/

		bool m_initialised;
		bool m_connected;
		std::atomic<bool> m_run_thread_loop;
		std::atomic<tracking::Button> m_button;

		/***********************************************************************
		* functions
		**********************************************************************/

		/**
		* Vrpn device callback for button changes.
		*
		* @param userData Pinter to class, which register callback (that).
		* @param vrpnData Data struct holding current button data.
		*/
		static void VRPN_CALLBACK on_button_changed(void* userData, const vrpn_BUTTONCB vrpnData);
	};

} /** end namespace tracking */

#endif /** TRACKING_VRPNBUTTONDEVICE_H_INCLUDED */
