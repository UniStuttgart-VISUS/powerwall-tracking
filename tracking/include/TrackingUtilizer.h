// <copyright file="TrackingUtilizer.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for details.
// </copyright>
// <author>Matthias Braun</author>

#ifndef TRACKING_TRACKINGUTILIZER_H_INCLUDED
#define TRACKING_TRACKINGUTILIZER_H_INCLUDED

#ifdef TRACKING_EXPORTS  
#define TRACKING_API __declspec(dllexport)   
#else  
#define TRACKING_API __declspec(dllimport)   
#endif  

#include "Tracker.h"

namespace tracking {

	/***************************************************************************
	*
	* Provides processed tracking data:
	*
	* - Raw tracking data (button, position, orientation).
	* - Manipulated camera parameters depending on pressed button(s).
	* - Screen Intersection as point in relative 2D coordinates.
	* - Field of view as rectangle in relative 2D coordinates.
	* - State of button defined for general selection processing.
	*
	***************************************************************************
	* TROUBLESHOOTING:
	*
	* - If your program does not receive any tracking data:
	*     For your program there must be firewall rules which allow incoming
	*     traffic for TCP port 3884 and UDP ports 1510 and 1511.
	*     This is required by the NatNet client implemented in NatNetDevicePool.
	*
	* - If your program only receives zero valued tracking data:
	*     Switch from `Multicast` to `Unicast` in settings of 'Motive' software
	*     on NatNet server `mini`.
	*
	***************************************************************************/
	class TRACKING_API TrackingUtilizer {

	public:

		enum Dim {
			DIM_2D = 0,
			DIM_3D = 1
		};

		enum FovMode {
			WIDTH_AND_HEIGHT = 0,
			WIDTH_AND_ASPECT_RATIO = 1,
			HORIZONTAL_ANGLE_AND_VERTICAL_ANGLE = 2,
			HORIZONTAL_ANGLE_AND_ASPECT_RATIO = 3
		};

		enum FovAspectRatio {
			AR_2_35__1 = 0,
			AR_1_85__1 = 1,
			AR_1_77__1 = 2, /** = 16:9  */
			AR_1_60__1 = 3, /** = 16:10 */
			AR_1_33__1 = 4, /** = 4:3   */
			AR_1__1 = 5
		};

		/** Data structure for setting parameters as batch. */
		struct Params {
			std::string                       btn_device_name;       /** Name of the button device to use.                                                             */
			std::string                       rigid_body_name;       /** Name of the rigid body to use.                                                                */
			tracking::Button                  select_btn;            /** The button that must be pressed for selection (set to 0 to dissolve link to any button).      */
			tracking::Button                  rotate_btn;            /** The button that must be pressed for rotation (set to 0 to dissolve link to any button).       */
			tracking::Button                  translate_btn;         /** The button that must be pressed for translation (set to 0 to dissolve link to any button).    */
			tracking::Button                  zoom_btn;              /** The button that must be pressed for dolly zoom (set to 0 to dissolve link to any button).     */
			bool                              invert_rotate;         /** Inverts the rotation direction.                                                               */
			bool                              invert_translate;      /** Inverts the rotation direction.                                                               */
			bool                              invert_zoom;           /** Inverts the zoom direction.                                                                   */
			float                             rotate_speed;          /** The rotation speed.                                                                           */
			float                             translate_speed;       /** The translation speed.                                                                        */
			float                             zoom_speed;            /** Transformation of distance to zoom speed.                                                     */
			bool                              single_interaction;    /** Disables multiple interactions at the same time.                                              */
			TrackingUtilizer::FovMode         fov_mode;              /** Select the parameters to define field of view projected on screen.                            */
			float                             fov_height;            /** Set relative fixed width for fov on screen.                                                   */
			float                             fov_width;             /** Set relative fixed height for fov on screen.                                                  */
			float                             fov_horiz_angle;       /** Set fixed horizontal angle in degrees for fov.                                                */
			float                             fov_vert_angle;        /** Set fixed vertical angle in degrees for fov.                                                  */
			TrackingUtilizer::FovAspectRatio  fov_aspect_ratio;      /** Set fixed aspect ratio for fov for given angle.                                               */
			float                             physical_height;       /** Set physical height of screen.                                                                */
			float                             physical_width;        /** Set physical width of screen.                                                                 */
			glm::vec3                         physical_origin;       /** Set physical origin of screen.                                                                */
			glm::vec3                         physical_x_dir;        /** Set physical x-direction of screen.                                                           */
			glm::vec3                         physical_y_dir; 	     /** Set physical y-direction of screen.                                                           */
		};

		///////////////////////////////////////////////////////////////////////

		/**
		* CTOR
		*/
		TrackingUtilizer(void);

		/**
		* DTOR
		*/
		virtual ~TrackingUtilizer(void);

		/**
		* Initialisation.
		*
		* @return True for success, false otherwise.
		*/
		bool Initialise(const tracking::TrackingUtilizer::Params& params, std::shared_ptr<tracking::Tracker> tracker);

		/**********************************************************************/
		// GET

		/**
		*  Get the raw tracking data.
		*
		* @param o_button                 Output the current button of the given button device.
		* @param o_position_(x,y,z)       Output the current position of the given rigid body.
		* @param o_orientation_(x,y,z,w)  Output the current orientation of the given rigid body.
		*
		* @return True for success, false otherwise.
		*/
		bool GetRawData(unsigned int& o_button,
			float& o_position_x, float& o_position_y, float& o_position_z,
			float& o_orientation_x, float& o_orientation_y, float& o_orientation_z, float& o_orientation_w);

		/**
		*  Get the current state of the select button.
		*
		* @param o_selecttion  Output the current selection state. True if select button is pressed, false otherwise.
		*
		* @return True for success, false otherwise.
		*/
		bool GetSelectionState(bool& o_selecttion);

		/**
		*  Get the current intersection with the screen.
		*
		* @param o_intersection_(x,y)  Output the relative 2D screen intersection coordinates (in range [0,1]).
		*
		* @return True for success, false otherwise.
		*/
		bool GetIntersection(float& o_intersection_x, float& o_intersection_y);

		/**
		*  Get the current field of view.
		*
		* @param (lt,lb,rt,rb)_(x,y)  Output the relative 2D screen space field of view vertices (in range [0,1]).
		*
		* @return True for success, false otherwise.
		*/
		bool GetFieldOfView(float& o_left_top_x, float& o_left_top_y,
			float& o_left_bottom_x, float& o_left_bottom_y,
			float& o_right_top_x, float& o_right_top_y,
			float& o_right_bottom_x, float& o_right_bottom_y);

		/**
		* Get the updated camera vectors depending on pressed buttons.
		*
		* Requires previous call of SetCurrentCamera(...).
		*
		* @param _idim                 The current world space dimension.
		*                              3D: Transformations are applied in three-dimensional space.
		*                              2D: Transformations are applied in two-dimensional screen space.
		* @param i_distance_center     The distance along the view vector to the rotation center of the camera.
		* @param io_cam_position_(x,y) Input current camera position. Output of the updated camera position.
		* @param io_cam_view_(x,y)     Input current camera view direction. Output of the updated camera view direction.
		* @param io_cam_up_(x,y)       Input current camera up direction. Output of the updated camera up direction.
		*
		* @return True for success, false otherwise.
		*/
		bool GetUpdatedCamera(TrackingUtilizer::Dim i_dim,
			float i_distance_center,
			float& io_cam_position_x, float& io_cam_position_y, float& io_cam_position_z,
			float& io_cam_view_x, float& io_cam_view_y, float& io_cam_view_z,
			float& io_cam_up_x, float& io_cam_up_y, float& io_cam_up_z);

		/**
		* Get the button device name.
		*
		* @return The button device name.
		*/
		inline const char* GetButtonDeviceName(void) const {
			return this->m_button_device_name.c_str();
		}

		/**
		* Get the rigid body name.
		*
		* @return The rigid body name.
		*/
		inline const char* GetRigidBodyName(void) const {
			return this->m_rigid_body_name.c_str();
		}

		/**
		* Detect the calibration orientation of the pointing device.
		* >>> Put rigid body somewhere pointing vertically towards the powerwall screen and
		* >>> right- and up-Vector3D of rigid body must be parallel to x- and y-axis of powerwall screen.
		* >>> Previous calibration is stored in 'tracking.conf' file.
		*
		* @return True for success, false otherwise.
		*/
		bool Calibrate(void);

	private:

		/***********************************************************************
		* variables
		**********************************************************************/

		bool                                m_initialised;
		std::shared_ptr<tracking::Tracker>  m_tracker;
		glm::vec3                           m_current_cam_position;
		glm::vec3                           m_current_cam_up;
		glm::vec3                           m_current_cam_view;
		float                               m_current_cam_center_dist;
		glm::vec2                           m_current_intersection;
		tracking::Rectangle                 m_current_fov;
		glm::quat                           m_current_orientation;
		glm::vec3                           m_current_position;
		tracking::Button                    m_current_button;
		bool                                m_current_selecting;
		tracking::Button                    m_last_button;
		std::vector<glm::vec3>              m_buffer_positions;
		unsigned int                        m_buffer_idx;
		bool                                m_const_position;
		glm::vec3                           m_start_cam_view;
		glm::vec3                           m_start_cam_position;
		glm::vec3                           m_start_cam_up;
		float                               m_start_cam_center_dist;
		glm::vec3                           m_start_position;
		glm::quat                           m_start_orientation;
		glm::quat                           m_start_relative_orientation;
		bool                                m_is_rotating;
		bool                                m_is_translating;
		bool                                m_is_zooming;

		/** parameters ********************************************************/

		std::string                         m_button_device_name;
		std::string                         m_rigid_body_name;
		unsigned int                        m_select_button;
		unsigned int                        m_rotate_button;
		unsigned int                        m_translate_button;
		unsigned int                        m_zoom_button;
		bool                                m_invert_rotate;
		bool                                m_invert_translate;
		bool                                m_invert_zoom;
		float                               m_rotate_speed;
		float                               m_translate_speed;
		float                               m_zoom_speed;
		bool                                m_single_interaction;
		TrackingUtilizer::FovMode           m_fov_mode;
		float                               m_fov_height;
		float                               m_fov_width;
		float                               m_fov_hori_angle;
		float                               m_fov_vert_angle;
		TrackingUtilizer::FovAspectRatio    m_fov_aspect_ratio;

		float                               m_physical_height;
		float                               m_physical_width;
		glm::quat                           m_calibration_orientation;
		glm::vec3                           m_physical_origin;
		glm::vec3                           m_physical_x_dir;
		glm::vec3                           m_physical_y_dir;

		/***********************************************************************
		* functions
		**********************************************************************/

		/** Print used parameter values. */
		void print_params(void);

		/** Read physical values from file. */
		bool read_params_from_file(void);

		/**
		* Request updated tracking data.
		*
		* @return True for success, false otherwise.
		*/
		bool update_tracking_data(void);

		/**
		* Process button changes.
		*/
		bool process_button_changes(void);

		/**
		* Apply camera 3D transformations.
		*/
		bool process_camera_transformations_3d(void);

		/**
		* Apply camera 2D transformations.
		*/
		bool process_camera_transformations_2d(void);

		/**
		* Process screen interaction.
		*/
		bool process_screen_interaction(bool process_fov);

		glm::vec2 clip_rect(glm::vec2 intersection, glm::vec2 vertex);

		/**
		* Limit value "val" to range [min, max]
		*/
		template<typename T>
		T limit(T val, T min, T max);

		template<typename T>
		T limit(T val, T min, T max, bool& changed);

		/**
		* xform
		*
		* @return Answer the (normalised) rotation for matching 'u' on 'v'.
		*/
		glm::quat xform(const glm::vec3& u, const glm::vec3& v);

	};

} /** end namespace tracking */

#endif /** TRACKING_TRACKINGUTILIZER_H_INCLUDED */