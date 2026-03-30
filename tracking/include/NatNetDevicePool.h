// <copyright file="NatNetDevicePool.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#ifndef TRACKING_NATNETDEVICEPOOL_H_INCLUDED
#define TRACKING_NATNETDEVICEPOOL_H_INCLUDED

#include <string>
#include <vector>
#include <atomic> 
#include <thread> 

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "NatNetTypes.h"
#include "NatNetClient.h"
#include "NatNetCAPI.h"

namespace tracking {

	/***************************************************************************
	*
	* Manages the connection to a NatNet host application and holds the
	* tracking data of available rigid bodies.
	*
	***************************************************************************/
	class NatNetDevicePool {

	public:

		/** Supported connection types for NatNet. */
		enum ConnectionType {
			MultiCast = 0,
			UniCast = 1
		};

		/** Data structure for setting parameters as batch. */
		struct Params {
			const char* client_ip;      /** The IP address of the NatNet client.       */
			size_t                           client_ip_len;
			const char* server_ip;      /** The IP address of the NatNet server.       */
			size_t                           server_ip_len;
			unsigned int                     cmd_port;       /** The NatNet command port.                   */
			unsigned int                     data_port;      /** The NatNet data port.                      */
			NatNetDevicePool::ConnectionType con_type;       /** The NatNet connection type.                */
			bool                             verbose_client; /** Turn on/off NatNet client massage output.  */
		};

		/** Data of one rigid body. */
		struct RigidBodyData {
			glm::quat                        orientation;    /** The current orientation of the motion device. */
			glm::vec3                        position;       /** The current position of the motion device. */
		};

		///////////////////////////////////////////////////////////////////////

		/**
		* CTOR
		*/
		NatNetDevicePool(void);

		/**
		* DTOR
		*/
		~NatNetDevicePool(void);

		/**
		* Initialisation.
		*
		* @return True for success, false otherwise.
		*/
		bool Initialise(const NatNetDevicePool::Params& params);

		/**
		* Connect to natnet server.
		*
		* @return Id name pairs of all found rigid bodies.
		*/
		bool Connect(void);

		/**
		* Disconnect from natnet server.
		*
		* @return True on success, false otherwise.
		*/
		bool Disconnect(void);

		/*********************************************************************/
		// GET

		/**
		* Get orientation of rigid body.
		* If no name is given, the data of the first rigid body is returned.
		*
		* @param rigid_body The rigid body name to get the orientation of
		*
		* @return The current orientation of the given rigid body.
		*/
		glm::quat GetOrientation(const std::string& rigid_body);

		/**
		* Get position of rigid body.
		* If no name is given, the data of the first rigid body is returned.
		*
		* @param rigid_body The rigid body name to get the position of.
		*
		* @return The current position of the given rigid body.
		*/
		glm::vec3 GetPosition(const std::string& rigid_body);

		/**
		* Get all available rigid body names.
		*
		* @return All available rigid body names.
		*/
		inline std::vector<std::string>& GetRigidBodyNames(void) {
			return this->m_rigid_body_names;
		}

	private:

		/***********************************************************************
		* types and structs
		**********************************************************************/

		/**
		*  Data structure for rigid bodies.
		*
		*  Atomic variables have no copy constructor, therefore class
		*  can only be used in std:.vector via (shared) pointer.
		*
		*  Mutable data is stored in lock free (triple-)buffer for concurrent
		*  access by natnet callback and megamol callback.
		*/
		class RigidBody {
		public:
			RigidBody(int id, const std::string& name)
				: id(id)
				, name(name)
				, read(0)
				, write(1)
				, lockFreeData() {
			}

			const int                 id;                // ID of motoin device (never changes)
			const std::string         name;              // Name of motion device (never changes)
			std::atomic<unsigned int> read;              // index of readable RigidBodyData
			std::atomic<unsigned int> write;             // index of writable RigidBodyData
			RigidBodyData             lockFreeData[3];   // triple buffer of data for one rigid body
		};

		/**********************************************************************
		* variables
		**********************************************************************/

		bool m_initialised;
		bool m_connected;
		std::unique_ptr<NatNetClient> m_natnet_client;
		std::vector<std::shared_ptr<RigidBody>> m_rigid_bodies;
		int m_callback_counter;
		std::vector<std::string> m_rigid_body_names;

		/** parameters ********************************************************/

		/**
		* Specifies the localnatnetClient IP address used for NatNet.
		*/
		std::string m_client_ip;

		/**
		* Specifies the server IP address used for NatNet.
		*/
		std::string m_server_ip;

		/**
		* Specifies the port used for NatNet commands.
		*/
		unsigned int m_cmd_port;

		/**
		* Specifies the port used for NatNet data.
		*/
		unsigned int m_data_port;

		/** *
		* Specifies the type of NatNet connection.
		*/
		NatNetDevicePool::ConnectionType m_con_type;

		/**
		* If 'true' message NatNet log callback is set.
		*/
		bool m_verbose_client;

		/**********************************************************************
		* functions
		**********************************************************************/

		/** Print used parameter values. */
		void print_params(void);

		/**
		* NatNet client callback for data.
		*
		* @param pFrameOfData Single frame of data for all tracked objects.
		* @param pUserData    Pointer to class which registered callback (that).
		*/
		static void __cdecl on_data(sFrameOfMocapData* pFrameOfData, void* pUserData);

		/**
		* NatNet client callback for messages.
		*
		* @param level   The verbosity level of the message.
		* @param message The message.
		*/
		static void __cdecl on_message(Verbosity level, const char* message);

	};

} /** end namespace tracking */

#endif /** TRACKING_NATNETDEVICEPOOL_H_INCLUDED */
