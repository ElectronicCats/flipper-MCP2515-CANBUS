//---------------------------------------------------------------------
//  Here you gonna put all the scenes you wanna use, they have to be in
//  order.
//
//  For any scene you "add", you have to create 3 functions in every scene,
//  they can be on one file or can be in differents files but must be included
//  in the program
//
//  The name of the functions will be given in the app_scene_functions.c
//  there is a code that defines the names and the functions
//---------------------------------------------------------------------

ADD_SCENE(app, menu, main_menu)
ADD_SCENE(app, sniffing, sniffing_option)
ADD_SCENE(app, box_sniffing, box_sniffing)
ADD_SCENE(app, device_no_connected, device_no_connected)
ADD_SCENE(app, sender, sender_option)
ADD_SCENE(app, send_message, send_message)
ADD_SCENE(app, warning_log, warning_log_sender)
ADD_SCENE(app, id_list, id_list_option)
ADD_SCENE(app, input_text, input_text_option)
ADD_SCENE(app, read_logs, read_logs)
ADD_SCENE(app, settings, settings_option)
ADD_SCENE(app, about_us, about_us)
ADD_SCENE(app, obdii_menu, obdii_option)
ADD_SCENE(app, obdii_typical_codes, obdii_typical_codes_option)
ADD_SCENE(app, draw_obdii, draw_obii_option)
ADD_SCENE(app, menu_supported_pid, supported_pid_option)
ADD_SCENE(app, list_supported_pid, list_supported_pid_option)
ADD_SCENE(app, obdii_warnings, obdii_warning_scenes)
ADD_SCENE(app, obdii_get_errors, obdii_get_errors_option)
ADD_SCENE(app, obdii_delete_dtc, obdii_delete_errors_option)
ADD_SCENE(app, manual_sender_pid, manual_sender_pid_option)
ADD_SCENE(app, response_manual_pid, response_pid_option)
ADD_SCENE(app, input_manual_set_pid, input_manual_pid_option)
ADD_SCENE(app, get_car_data, car_data_option)
ADD_SCENE(app, play_logs, play_logs)
ADD_SCENE(app, file_browser, file_browser_option)
ADD_SCENE(app, play_logs_widget, play_logs_widget)

// For the UDS FUNCTIONS
ADD_SCENE(app, uds_menu, uds_menu_option)
ADD_SCENE(app, uds_single_frame_request_sender, uds_single_frame_request_sender_option)
ADD_SCENE(app, uds_single_frame_request_response, uds_single_frame_request_response_option)
ADD_SCENE(app, uds_single_frame_data, uds_single_frame_data_option)
ADD_SCENE(app, uds_request_vin, uds_request_vin_option)
ADD_SCENE(app, uds_set_diagnostic_session, uds_set_diagnostic_session_option)
ADD_SCENE(app, uds_set_session_response, uds_set_session_response)
ADD_SCENE(app, uds_ecu_reset, uds_ecu_reset_option)
ADD_SCENE(app, uds_ecu_reset_response, uds_ecu_reset_response_option)
