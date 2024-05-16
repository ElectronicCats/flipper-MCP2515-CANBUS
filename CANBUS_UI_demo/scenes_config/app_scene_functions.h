#include <gui/scene_manager.h>

//--------------------------------------------------------------------
//  This part of this code works to create the enum of the scenes
//  They have to be in order from the app_scene_config
//--------------------------------------------------------------------

#define ADD_SCENE(prefix, name, id) app_scene_##id,
typedef enum {
#include "app_scene_config.h"
    app_scene_enum,
} Appscenes;
#undef ADD_SCENE

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//  This code define the functions for every scene we have, it gives the name
//  the functions "on_enter", "on_exit" and "on_event"
//---------------------------------------------------------------------
//---------------------------------------------------------------------

extern const SceneManagerHandlers app_scene_handlers; // We define the scene manager handler

//---------------------------------------------------------------------
//  This defines and name of the "on_enter" functions
//---------------------------------------------------------------------

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "app_scene_config.h"
#undef ADD_SCENE

//---------------------------------------------------------------------
//  This defines and name of the "on_event" functions
//---------------------------------------------------------------------

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "app_scene_config.h"
#undef ADD_SCENE

//---------------------------------------------------------------------
//  This defines and name of the "on_exit" functions
//---------------------------------------------------------------------

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "app_scene_config.h"
#undef ADD_SCENE
