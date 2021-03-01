#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_CLICKABLE_PROTECTED 1

#include <Efl_Ui.h>
#include "elm_priv.h"

typedef struct {

} Efl_Ui_Action_Connector_Data;

static void
_on_press_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   efl_input_clickable_press(data, 1);
}

static void
_on_unpress_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   efl_input_clickable_unpress(data, 1);
}

static void
_on_mouse_out(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   efl_input_clickable_button_state_reset(data, 1);
}

static void
_theme_move_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Input_Pointer *pointer = ev->info;

   if (efl_input_processed_get(pointer))
     efl_input_clickable_button_state_reset(data, 1);
}

EFL_CALLBACKS_ARRAY_DEFINE(bind_clickable_to_theme_callbacks,
  {EFL_EVENT_POINTER_MOVE, _theme_move_cb},
)

EOLIAN static void
_efl_ui_action_connector_bind_clickable_to_theme(Efl_Canvas_Layout *object, Efl_Input_Clickable *clickable)
{
   efl_event_callback_array_add(object, bind_clickable_to_theme_callbacks(), clickable);

   efl_layout_signal_callback_add(object, "efl,action,press", "*", clickable, _on_press_cb, NULL);
   efl_layout_signal_callback_add(object, "efl,action,unpress", "*", clickable, _on_unpress_cb, NULL);
   efl_layout_signal_callback_add(object, "efl,action,mouse_out", "*", clickable, _on_mouse_out, NULL);
}

static void
_press_cb(void *data, const Efl_Event *ev)
{
   Efl_Input_Pointer *pointer = ev->info;
   if (!efl_input_processed_get(pointer))
     {
        efl_input_clickable_press(data, 1);
     }
}

static void
_unpress_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Input_Pointer *pointer = ev->info;
   Eina_Position2D mouse_pos = efl_input_pointer_position_get(pointer);
   Eina_Rect geom = efl_gfx_entity_geometry_get(data);
   if (efl_input_processed_get(pointer))
     {
        efl_input_clickable_button_state_reset(data, 1);
     }
   else if (!eina_rectangle_coords_inside(&geom.rect, mouse_pos.x, mouse_pos.y))
     {
        //we are emulating edje behavior here, do press unpress on the event, but not click
        efl_input_clickable_button_state_reset(data, 1);
        if (efl_canvas_object_pointer_mode_get(data) == EFL_INPUT_OBJECT_POINTER_MODE_AUTO_GRAB)
          {
             efl_input_clickable_unpress(data, 1);
          }
     }
   else
     {
        efl_input_clickable_unpress(data, 1);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(bind_clickable_to_object_callbacks,
  {EFL_EVENT_POINTER_DOWN, _press_cb},
  {EFL_EVENT_POINTER_UP, _unpress_cb},
)

EOLIAN static void
_efl_ui_action_connector_bind_clickable_to_object(Efl_Input_Interface *object, Efl_Input_Clickable *clickable)
{
   efl_event_callback_array_add(object, bind_clickable_to_object_callbacks(), clickable);
}


#include "efl_ui_action_connector.eo.c"
