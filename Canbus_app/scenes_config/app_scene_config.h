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

// On development
ADD_SCENE(app, obdii_menu, obdii_option)
ADD_SCENE(app, engine_rpm_speed, engine_speed_option)
