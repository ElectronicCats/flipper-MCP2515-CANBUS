#include "app_scene_functions.h"

//--------------------------------------------------------------------
//  This part works to create the name of the functions
//  of the "on_enter" for every scene.
//--------------------------------------------------------------------

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const app_on_enter_handlers[])(void*) = {
#include "app_scene_config.h"
};
#undef ADD_SCENE

//--------------------------------------------------------------------
//  This part works to create the name of the functions
//  of the "on_event" for every scene.
//--------------------------------------------------------------------

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const app_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "app_scene_config.h"
};
#undef ADD_SCENE

//--------------------------------------------------------------------
//  This part works to create the name of the functions
//  of the "on_exit" for every scene.
//--------------------------------------------------------------------

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const app_on_exit_handlers[])(void* context) = {
#include "app_scene_config.h"
};
#undef ADD_SCENE

//--------------------------------------------------------------------
//  Here we add the callback functions for the scenes.
//--------------------------------------------------------------------

// Initialize scene handlers configuration structure
const SceneManagerHandlers app_scene_handlers = {
    .on_enter_handlers = app_on_enter_handlers,
    .on_event_handlers = app_on_event_handlers,
    .on_exit_handlers = app_on_exit_handlers,
    .scene_num = app_scene_enum,
};