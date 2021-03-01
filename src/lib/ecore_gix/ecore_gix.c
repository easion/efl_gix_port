#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>


#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Gix.h"
#include "ecore_gix_private.h"
#include "ecore_Gix_Keys.h"
#include "Ecore.h"
#include "ecore_private.h"

/* ecore_gix */
/******************/
/* About */
/* with this you can create windows of gix and handle events through ecore
 * TODO:
 * - handle all event types
 * -
 * */
int _ecore_gix_log_dom = -1;

static int _ecore_gix_init_count = 0;
static int _window_event_fd = 0;


EAPI int ECORE_GIX_EVENT_WINDOW_PROPERTY = 0;
EAPI int ECORE_GIX_EVENT_CLIENT_MESSAGE = 0;
EAPI int ECORE_GIX_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_GIX_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_DAMAGE = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_DELETE_REQUEST = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_GIX_EVENT_DESTROYED = 0;
EAPI int ECORE_GIX_EVENT_GESTURE_TOUCH = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_HIDE = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_SHOW = 0;

EAPI int ECORE_GIX_EVENT_WINDOW_CREATE = 0;
EAPI int ECORE_GIX_EVENT_WINDOW_REPARENT = 0;
EAPI int ECORE_GIX_EVENT_SELECTION_CLEAR = 0;
EAPI int ECORE_GIX_EVENT_SELECTION_REQUEST = 0;
EAPI int ECORE_GIX_EVENT_SELECTION_NOTIFY = 0;
EAPI int ECORE_GIX_EVENT_MOUSE_IN = 0;
EAPI int ECORE_GIX_EVENT_MOUSE_OUT = 0;

static Ecore_Fd_Handler *_window_event_fd_handler_handle = NULL;

static Ecore_Gix_Window _ecore_gix_mouse_down_last_window = NULL;
static Ecore_Gix_Window _ecore_gix_mouse_down_last_last_window = NULL;
static long                _ecore_gix_mouse_down_last_time = 0  ;
static long                _ecore_gix_mouse_down_last_last_time = 0  ;
static int                 _ecore_gix_mouse_down_did_triple = 0;
static int                 _ecore_gix_mouse_up_count = 0;
double              _ecore_gix_double_click_time = 0.25;
long                _ecore_gix_event_last_time = 0;

static int _ecore_x_last_event_mouse_move;

void _ecore_x_event_handle_selection_clear(gi_msg_t *xevent);
void _ecore_x_event_handle_selection_request(gi_msg_t *xevent);
void _ecore_x_event_handle_selection_notify(gi_msg_t *xevent);


static int (*event_filter_func)(gi_msg_t *msg) = NULL;

void ecore_gix_set_default_event_filter(int (*func)(gi_msg_t *msg))
{
	event_filter_func = func;
}


EAPI time_t
ecore_gix_current_time_get(void)
{
   return _ecore_gix_event_last_time;
} /* ecore_gix_current_time_get */

EAPI void
ecore_gix_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_gix_double_click_time = t;
}


/*******************/
/* local functions */
/*******************/
static int get_mouse_button(int flags)
{
	if (flags & GI_BUTTON_L)
		return 1;
	if (flags & GI_BUTTON_M)
		return 2;
	if (flags & GI_BUTTON_R)
		return 3;
	if (flags & GI_BUTTON_WHEEL_UP)
		return 4;
	if (flags & GI_BUTTON_WHEEL_DOWN)
		return 5;
	return 0;
}

/* free ecore gix events functions */
/****************************************/

static void
_ecore_gix_event_free_key_down(void *data , void *ev)
{
   Ecore_Event_Key *e;

   e = ev;
   if (e->keyname) free((char *)e->keyname);
   if (e->key) free((char *)e->key);
   if (e->string) free((char *)e->string);
   free(e);
} /* _ecore_gix_event_free_key_down */

static void
_ecore_gix_event_free_key_up(void *data , void *ev)
{
   Ecore_Event_Key *e;

   e = ev;
   if (e->keyname) free((char *)e->keyname);
   if (e->key) free((char *)e->key);
   if (e->string) free((char *)e->string);
   free(e);
} /* _ecore_gix_event_free_key_up */

/* gix window input events handler */
/****************************************/


static unsigned int
_ecore_gix_event_modifiers(int mod)
{
   unsigned int        modifiers = 0;

   if(mod & G_MODIFIERS_LSHIFT) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & G_MODIFIERS_RSHIFT) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & G_MODIFIERS_LCTRL) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & G_MODIFIERS_RCTRL) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & G_MODIFIERS_LALT) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if(mod & G_MODIFIERS_RALT) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if(mod & G_MODIFIERS_NUMLOCK) modifiers |= ECORE_EVENT_LOCK_NUM;
   if(mod & G_MODIFIERS_CAPSLOCK) modifiers |= ECORE_EVENT_LOCK_CAPS;

   return modifiers;
}


static void
_ecore_gix_event_handle_motion(gi_msg_t *evt)
{
	Ecore_Event_Mouse_Move *e;
	int touched = 0;

	if (evt->params[2] != 0)
	{
		//Not Touch
		//return;
		touched = 1;
	}

	e = calloc(1, sizeof(Ecore_Event_Mouse_Move));
	if (!e) return;

	e->x = evt->body.rect.x;
	e->y = evt->body.rect.y;
	e->root.x = evt->params[0];
    e->root.y = evt->params[1];
	e->window = evt->ev_window;
    e->event_window = e->window;
	e->timestamp = evt->time;

	e->modifiers = _ecore_gix_event_modifiers(evt->params[3]);

	 e->multi.device = 0; //单点触摸设备
	 e->multi.radius = e->multi.radius_x = e->multi.radius_y = 0;
	 e->multi.pressure = e->multi.angle = 0;
	 e->multi.x = e->multi.y = e->multi.root.x = e->multi.root.y = 0;

	ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);
} /* _ecore_gix_event_handle_motion */

int utf8_wctomb (unsigned char *dest, uint32_t wc, int dest_size);


static Ecore_Event_Key*
_ecore_sdl_event_key(gi_msg_t *event)
{
   Ecore_Event_Key *ev;
   unsigned int i;
   int keyval;

   ev = calloc(1, sizeof(Ecore_Event_Key));
   if (!ev) return NULL;

   keyval = event->params[3];

   ev->timestamp = event->time;
   ev->window = event->ev_window;
   ev->event_window = ev->window;
   ev->modifiers = _ecore_gix_event_modifiers(event->body.message[3]);
   ev->key = NULL;
   ev->compose = NULL;
   ev->same_screen = 1;

   if (event->attribute_1)
	{
		unsigned char utf8_buffer[16];
		int count;

		memset(utf8_buffer,0,sizeof(utf8_buffer));
		count = utf8_wctomb(utf8_buffer, keyval , 16);
		ev->keyname = strdup(utf8_buffer);
		ev->string = strdup(utf8_buffer);
		ev->key = strdup(utf8_buffer);
		return ev;
	}
	else{

		if (keyval == G_KEY_ENTER)
		{
			keyval = G_KEY_RETURN;
		}

	 for (i = 0; i < sizeof(keystable) / sizeof(struct _ecore_sdl_keys_s); ++i)
       if (keystable[i].code == keyval)
       {
          ev->keyname = strdup(keystable[i].name);
          ev->string = strdup(keystable[i].compose);
		  ev->key = strdup(keystable[i].compose);
          return ev;
       }
	}

   free(ev);
   return NULL;
}


static void
_ecore_gix_event_handle_key_down(gi_msg_t *evt)
{
   Ecore_Event_Key *e;

   e = _ecore_sdl_event_key(evt);
	if (!e) return;   

   ecore_event_add(ECORE_EVENT_KEY_DOWN, e, _ecore_gix_event_free_key_down, NULL);
} /* _ecore_gix_event_handle_key_down */

static void
_ecore_gix_event_handle_key_up(gi_msg_t *evt)
{
   Ecore_Event_Key *e;

   e = _ecore_sdl_event_key(evt);
 
   if (!e) return;

   ecore_event_add(ECORE_EVENT_KEY_UP, e, _ecore_gix_event_free_key_up, NULL);
} /* _ecore_gix_event_handle_key_up */

static void
_ecore_gix_event_handle_button_down(gi_msg_t *evt)
{
   Ecore_Event_Mouse_Button *e;

   e = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!e) return;

	e->buttons = get_mouse_button(evt->params[2]);;
#if 0
	e->x = evt->body.rect.x;
	e->y = evt->body.rect.y;
	e->root.x = evt->params[0];
    e->root.y = evt->params[1];
#endif
	e->window = evt->ev_window;
	e->event_window = e->window;
	e->double_click = 0;
    e->triple_click = 0;
	e->timestamp = evt->time;
	 e->same_screen = 1;
	e->modifiers = _ecore_gix_event_modifiers(evt->body.message[3]);

	if (_ecore_gix_mouse_down_did_triple)
	{
	   _ecore_gix_mouse_down_last_window = NULL;
	   _ecore_gix_mouse_down_last_last_window = NULL;
	   _ecore_gix_mouse_down_last_time = 0;
	   _ecore_gix_mouse_down_last_last_time = 0;
	}


  if (((e->timestamp - _ecore_gix_mouse_down_last_time) <= (unsigned long)(1000 * _ecore_gix_double_click_time)) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_window))
	e->double_click = 1;

	if (((e->timestamp - _ecore_gix_mouse_down_last_last_time) <= (unsigned long)(2 * 1000 * _ecore_gix_double_click_time)) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_window) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_last_window))
	{
	   e->triple_click = 1;
	   _ecore_gix_mouse_down_did_triple = 1;
	}
	else
	_ecore_gix_mouse_down_did_triple = 0;

	if (!e->double_click && !e->triple_click)
	_ecore_gix_mouse_up_count = 0;

	_ecore_gix_event_last_time = e->timestamp;
	//_ecore_gix_event_last_window = (Ecore_Gix_Window)e->window;

	if (!_ecore_gix_mouse_down_did_triple)
	{
	   _ecore_gix_mouse_down_last_last_window = _ecore_gix_mouse_down_last_window;
	   _ecore_gix_mouse_down_last_window = (Ecore_Gix_Window)e->window;
	   _ecore_gix_mouse_down_last_last_time = _ecore_gix_mouse_down_last_time;
	   _ecore_gix_mouse_down_last_time = e->timestamp;
	}
	//gi_set_focus_window(evt->ev_window);
   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
} /* _ecore_gix_event_handle_button_down */

static void
_ecore_gix_event_handle_button_up(gi_msg_t *evt)
{
   Ecore_Event_Mouse_Button *e;

   e = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!e) return;
	e->buttons = get_mouse_button(evt->params[3]);
#if 0
	e->x = evt->body.rect.x;
	e->y = evt->body.rect.y;
	e->root.x = evt->params[0];
    e->root.y = evt->params[1];
#endif
	e->window = evt->ev_window;
    e->event_window = e->window;
	e->timestamp = evt->time;
	e->double_click = 0;
    e->triple_click = 0;
	e->same_screen = 1;
	e->modifiers = _ecore_gix_event_modifiers(evt->body.message[3]);

	_ecore_gix_mouse_up_count++;

	if ((_ecore_gix_mouse_up_count >= 2) &&
	  ((e->timestamp - _ecore_gix_mouse_down_last_time) <= (unsigned long)(1000 * _ecore_gix_double_click_time)) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_window)){
	e->double_click = 1;
	}

	if ((_ecore_gix_mouse_up_count >= 3) &&
	  ((e->timestamp - _ecore_gix_mouse_down_last_last_time) <= (unsigned long)(2 * 1000 * _ecore_gix_double_click_time)) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_window) &&
	  (e->window == (Ecore_Window)_ecore_gix_mouse_down_last_last_window)){
	e->triple_click = 1;
	}

	_ecore_gix_event_last_time = e->timestamp;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
} /* _ecore_gix_event_handle_button_up */


static void
_ecore_gix_event_handle_enter(gi_msg_t *evt)
{
   //Ecore_Event_Mouse_IO *e;
   Ecore_Gix_Event_Mouse_In *e;

   e = calloc(1, sizeof(Ecore_Gix_Event_Mouse_In));
   if (!e) return ;

   e->modifiers = 0;
   e->x = evt->body.rect.x;
   e->y = evt->body.rect.y;

   e->modifiers = _ecore_gix_event_modifiers(evt->params[3]);
   e->root.x = evt->params[0];
   e->root.y = evt->params[1];

   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;   

   ecore_event_add(ECORE_GIX_EVENT_MOUSE_IN, e, NULL, NULL);
   //ecore_event_add(ECORE_EVENT_MOUSE_IN, e, NULL, NULL);
} /* _ecore_gix_event_handle_enter */

static void
_ecore_gix_event_handle_leave(gi_msg_t *evt)
{
   //Ecore_Event_Mouse_IO *e;
   Ecore_Gix_Event_Mouse_Out *e;

   e = calloc(1, sizeof(Ecore_Gix_Event_Mouse_Out));
   if (!e) return;

   e->modifiers = 0;
   e->x = evt->body.rect.x;
   e->y = evt->body.rect.y;

   e->modifiers = _ecore_gix_event_modifiers(evt->params[3]);
   e->root.x = evt->params[0];
   e->root.y = evt->params[1];
   
   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;

   //ecore_event_add(ECORE_EVENT_MOUSE_OUT, e, NULL, NULL);
   ecore_event_add(ECORE_GIX_EVENT_MOUSE_OUT, e, NULL, NULL);
} /* _ecore_gix_event_handle_leave */

static void
_ecore_gix_event_handle_wheel(gi_msg_t *evt, int step)
{
   Ecore_Event_Mouse_Wheel *e;

   e = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
   if (!e) return;

   // currently there's no direction (only up/down);
   e->direction = 0;
   e->z = step;
   e->modifiers = 0;
   e->window = evt->ev_window;
   e->event_window = e->window;
   e->root.x = evt->params[0];
   e->root.y = evt->params[1];
   e->timestamp = evt->time;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);
} /* _ecore_gix_event_handle_wheel */


static void
_ecore_gix_event_handle_window_destroy(gi_msg_t *evt)
{
   Ecore_Gix_Event_Window_Destroy *e;

   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Destroy));
   if (!e) return ;

   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;
   ecore_event_add(ECORE_GIX_EVENT_DESTROYED, e, NULL, NULL);
}

static void
_ecore_gix_event_handle_gesture_touch(gi_msg_t *evt)
{
   Ecore_Gix_Event_Gesture_Touch *e;

   e = calloc(1, sizeof(Ecore_Gix_Event_Gesture_Touch));
   if (!e) return ;
   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;
   memcpy(e->params,evt->params,sizeof(evt->params));
   e->subtype = evt->sub_type;
   e->x = evt->body.rect.x;
   e->y = evt->body.rect.y;
   ecore_event_add(ECORE_GIX_EVENT_GESTURE_TOUCH, e, NULL, NULL);
}

static void
_ecore_gix_event_handle_window_show(gi_msg_t *evt)
{
   Ecore_Gix_Event_Window_Show *e;
   //if (evt->ev_window == evt->effect_window)
    // return;

	//printf("_ecore_gix_event_handle_window_show == \n");

   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Show));
   if (!e) return ;

   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;

   ecore_event_add(ECORE_GIX_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

static void
_ecore_gix_event_handle_window_hide(gi_msg_t *evt)
{
   Ecore_Gix_Event_Window_Hide *e;

   if (evt->ev_window == evt->effect_window)
     return;

   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Hide));
   if (!e) return ;

   e->win = evt->ev_window;
   e->event_win = e->win;
   e->time = evt->time;

   ecore_event_add(ECORE_GIX_EVENT_WINDOW_HIDE, e, NULL, NULL);
}


void
_ecore_gix_event_handle_expose(gi_msg_t *xevent)
{
   Ecore_Gix_Event_Window_Damage *e;

   //_ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Damage));
   if (!e) return;

   e->win = xevent->ev_window;
   e->time = xevent->time;
   e->x = xevent->body.rect.x;
   e->y = xevent->body.rect.y;
   e->w = xevent->body.rect.w;
   e->h = xevent->body.rect.h;

   //e->count = xevent->xexpose.count;
   ecore_event_add(ECORE_GIX_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
} /* _ecore_x_event_handle_expose */

static void
_ecore_gix_event_handle_got_focus(gi_msg_t *evt)
{
   Ecore_Gix_Event_Got_Focus *e;
   e = calloc(1, sizeof(Ecore_Gix_Event_Got_Focus));
   if (!e) return ;

   e->win = evt->ev_window;
   e->time = evt->time;

   gi_composition_form_t spot;

   spot.x = 0;
   spot.y = 0;
   gi_ime_associate_window(evt->ev_window,&spot);

   ecore_event_add(ECORE_GIX_EVENT_GOT_FOCUS, e, NULL, NULL);
} /* _ecore_gix_event_handle_got_focus */


static void
_ecore_gix_event_handle_lost_focus(gi_msg_t *evt)
{
   Ecore_Gix_Event_Lost_Focus *e;
   e = calloc(1, sizeof(Ecore_Gix_Event_Lost_Focus));
   if (!e) return;

   e->win = evt->ev_window;
   e->time = evt->time;

   gi_ime_associate_window(evt->ev_window,NULL);
   ecore_event_add(ECORE_GIX_EVENT_LOST_FOCUS, e, NULL, NULL);
} /* _ecore_gix_event_handle_lost_focus */




static void
_ecore_gix_event_handle_window_delete_request(gi_msg_t *evt)
{
   Ecore_Gix_Event_Window_Delete_Request *e;
   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Delete_Request));
   if (!e) return;

   e->win = evt->ev_window;
   e->time = evt->time;

   ecore_event_add(ECORE_GIX_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
} /* _ecore_gix_event_handle_lost_focus */


static void
_ecore_gix_event_handle_window_configure(gi_msg_t *evt)
{
   Ecore_Gix_Event_Window_Configure *e;

   if (evt->params[0] != GI_STRUCT_CHANGE_RESIZE && 
	   evt->params[0] != GI_STRUCT_CHANGE_MOVE)
	   return;
   //if (evt->client_sent)
   //  return; //wm bug, fixme

   e = calloc(1, sizeof(Ecore_Gix_Event_Window_Configure));
   if (!e) return;

   e->win = evt->ev_window;
   e->x = evt->body.rect.x;
   e->y = evt->body.rect.y;
   e->w = evt->body.rect.w;
   e->h = evt->body.rect.h;
   e->event_win = e->win;
   e->time = evt->time;

#if 0
   gi_window_info_t info;
   gi_get_window_info(e->win,&info);  

   if (evt->params[0] != GI_STRUCT_CHANGE_RESIZE)
   {  
   e->w = info.width;
   e->h = info.height;
   }
   printf("_ecore_gix_event_handle_window_configure:(%d,equ=%s)(%d,%d,%d,%d) (%d,%d,%d,%d)\n",evt->params[0], 
	   evt->ev_window == evt->effect_window?"YES":"NO",
	   evt->body.rect.x,evt->body.rect.y,evt->body.rect.w,evt->body.rect.h,info.x,info.y,info.width,info.height);
#endif

   ecore_event_add(ECORE_GIX_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);
} /* _ecore_gix_event_handle_lost_focus */

void
_ecore_gix_event_handle_property_notify(gi_msg_t *xevent)
{
   //_ecore_x_last_event_mouse_move = 0;
   {
      Ecore_Gix_Event_Window_Property *e;

      e = calloc(1, sizeof(Ecore_Gix_Event_Window_Property));
      if (!e)
         return;

      e->win = xevent->ev_window;
      e->atom = xevent->params[1];
      e->time = xevent->time;
     // _ecore_x_event_last_time = e->time;
      ecore_event_add(ECORE_GIX_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
   }
} /* _ecore_x_event_handle_property_notify */


static void
_ecore_x_event_free_xdnd_enter(void *data ,
                               void *ev)
{
   Ecore_Gix_Event_Xdnd_Enter *e;
   int i;

   e = ev;
   for (i = 0; i < e->num_types; i++)
     free(e->types[i]);
   free(e->types);
   free(e);
}


static int
_ecore_gix_event_handle_dnd_message(gi_msg_t *xevent)
{
	int ret = 1;

#if 1
	 if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_ENTER)
     {
        Ecore_Gix_Event_Xdnd_Enter *e;
        Ecore_Gix_DND_Target *target;

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Enter));
        if (!e) return 1;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        target->state = ECORE_GIX_DND_TARGET_ENTERED;
        target->source = xevent->params[0];
        target->win = xevent->ev_window;
        target->version = (int)(xevent->params[1] >> 24);
        if (target->version > ECORE_GIX_DND_VERSION)
          {
             printf("DND: Requested version %d, we only support up to %d",
                 target->version, ECORE_GIX_DND_VERSION);
             free(e);
             return 1;
          }

        if (xevent->params[1] & 0x1UL)
          {
             /* source supports more than 3 types, fetch property */
             unsigned char *data;
             Ecore_Gix_Atom *types;
             int i, num_ret;

             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             if (!(ecore_x_window_prop_property_get(target->source,
                                                    ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                                    GA_ATOM,
                                                    32, &data, &num_ret)))
               {
                  printf(
                    "DND: Could not fetch data type list from source window, aborting.");
                  free(e);
                  return 1;
               }

             types = (Ecore_Gix_Atom *)data;
             e->types = calloc(num_ret, sizeof(char *));
             if (e->types)
               {
                  LOGFN(__FILE__, __LINE__, __FUNCTION__);
                  for (i = 0; i < num_ret; i++)
                    e->types[i] = gi_get_atom_name( types[i]);
               }

             e->num_types = num_ret;
          }
        else
          {
             int i = 0;

             e->types = calloc(3, sizeof(char *));
             if (e->types)
               {
                  LOGFN(__FILE__, __LINE__, __FUNCTION__);
                  while ((i < 3) && (xevent->params[i + 2]))
                    {
                       e->types[i] = gi_get_atom_name(
                                                  xevent->params[i + 2]);
                       i++;
                    }
               }

             e->num_types = i;
          }

        e->win = target->win;
        e->source = target->source;
        ecore_event_add(ECORE_GIX_EVENT_XDND_ENTER, e,
                        _ecore_x_event_free_xdnd_enter, NULL);
     }
   /* Message Type: XdndPosition target */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_POSITION)
     {
        Ecore_Gix_Event_Xdnd_Position *e;
        Ecore_Gix_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_Gix_Window)xevent->params[0]) ||
            (target->win != xevent->ev_window))
          return 1;

        target->pos.x = xevent->params[2] >> 16;
        target->pos.y = xevent->params[2] & 0xFFFFUL;
        target->action = xevent->params[4]; /* Version 2 */

        target->time =  xevent->time;

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Position));
        if (!e) return 1;

        e->win = target->win;
        e->source = target->source;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        e->action = target->action;
        ecore_event_add(ECORE_GIX_EVENT_XDND_POSITION, e, NULL, NULL);
     }
   /* Message Type: XdndStatus source */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_STATUS)
     {
        Ecore_Gix_Event_Xdnd_Status *e;
        Ecore_Gix_DND_Source *source;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        source = _ecore_x_dnd_source_get();
        /* Make sure source/target match */
        if ((source->win != xevent->ev_window) ||
            (source->dest != (gi_window_id_t)xevent->params[0]))
          return 1;

        source->await_status = 0;

        source->will_accept = xevent->params[1] & 0x1UL;
        source->suppress = (xevent->params[1] & 0x2UL) ? 0 : 1;

        source->rectangle.x = xevent->params[2] >> 16;
        source->rectangle.y = xevent->params[2] & 0xFFFFUL;
        source->rectangle.width = xevent->params[3] >> 16;
        source->rectangle.height = xevent->params[3] & 0xFFFFUL;

        source->accepted_action = xevent->params[4];

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Status));
        if (!e) return 1;

        e->win = source->win;
        e->target = source->dest;
        e->will_accept = source->will_accept;
        e->rectangle.x = source->rectangle.x;
        e->rectangle.y = source->rectangle.y;
        e->rectangle.w = source->rectangle.width;
        e->rectangle.h = source->rectangle.height;
        e->action = source->accepted_action;

        ecore_event_add(ECORE_GIX_EVENT_XDND_STATUS, e, NULL, NULL);
     }
   /* Message Type: XdndLeave target */
   /* Pretend the whole thing never happened, sort of */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_LEAVE)
     {
        Ecore_Gix_Event_Xdnd_Leave *e;
        Ecore_Gix_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_Gix_Window)xevent->params[0]) ||
            (target->win != xevent->ev_window))
          return 1;

        target->state = ECORE_GIX_DND_TARGET_IDLE;

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Leave));
        if (!e) return 1;

        e->win = xevent->ev_window;
        e->source = (gi_window_id_t)xevent->params[0];
        ecore_event_add(ECORE_GIX_EVENT_XDND_LEAVE, e, NULL, NULL);
     }
   /* Message Type: XdndDrop target */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_DROP)
     {
        Ecore_Gix_Event_Xdnd_Drop *e;
        Ecore_Gix_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        /* Match source/target */
        if ((target->source != (gi_window_id_t)xevent->params[0]) ||
            (target->win != xevent->ev_window))
          return 1;

        target->time =     xevent->time ;

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Drop));
        if (!e) return 1;

        e->win = target->win;
        e->source = target->source;
        e->action = target->action;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        ecore_event_add(ECORE_GIX_EVENT_XDND_DROP, e, NULL, NULL);
     }
   /* Message Type: XdndFinished source */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_XDND_FINISHED)
     {
        Ecore_Gix_Event_Xdnd_Finished *e;
        Ecore_Gix_DND_Source *source;
        Eina_Bool completed = EINA_TRUE;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        source = _ecore_x_dnd_source_get();
        /* Match source/target */
        if ((source->win != xevent->ev_window) ||
            (source->dest != (gi_window_id_t)xevent->params[0]))
          return 1;

        if ((source->version < 5) || (xevent->params[1] & 0x1UL))
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             /* Target successfully performed drop action */
             ecore_x_selection_xdnd_clear();
             source->state = ECORE_GIX_DND_SOURCE_IDLE;
          }
        else if (source->version >= 5)
          {
             completed = EINA_FALSE;
             source->state = ECORE_GIX_DND_SOURCE_CONVERTING;

             /* FIXME: Probably need to add a timer to switch back to idle
              * and discard the selection data */
          }

        e = calloc(1, sizeof(Ecore_Gix_Event_Xdnd_Finished));
        if (!e) return 1;

        e->win = source->win;
        e->target = source->dest;
        e->completed = completed;
        if (source->version >= 5)
          {
             source->accepted_action = xevent->params[2];
             e->action = source->accepted_action;
          }
        else
          {
             source->accepted_action = 0;
             e->action = source->action;
          }

        ecore_event_add(ECORE_GIX_EVENT_XDND_FINISHED, e, NULL, NULL);
     }
	 else
#endif
	{
		 ret = 0;
	 }

	return ret;
}

static void
_ecore_gix_event_handle_client_message(gi_msg_t *xevent)
{
   _ecore_x_last_event_mouse_move = 0;

   if (_ecore_gix_event_handle_dnd_message(xevent))
   {
	   return;
   }

#if 0
   if ((xevent->body.client.client_type == ECORE_GIX_ATOM_NET_WM_MOVERESIZE) &&
            (xevent->body.client.client_format == 32) &&
/* Ignore move and resize with keyboard */
            (xevent->params[2] < 9))
     {
        Ecore_Gix_Event_Window_Move_Resize_Request *e;

        e = calloc(1, sizeof(Ecore_Gix_Event_Window_Move_Resize_Request));
        if (!e)
          return 1;

        e->win = xevent->ev_window;
        e->x = xevent->params[0];
        e->y = xevent->params[1];
        e->direction = xevent->params[2];
        e->button = xevent->params[3];
        e->source = xevent->params[4];
        ecore_event_add(ECORE_GIX_EVENT_WINDOW_MOVE_RESIZE_REQUEST, e, NULL, NULL);
     }
   /* Xdnd Client Message Handling Begin */
   /* Message Type: XdndEnter target */
   else if (xevent->body.client.client_type == ECORE_GIX_ATOM_NET_WM_STATE)
     {
        Ecore_Gix_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_Gix_Event_Window_State_Request));
        if (!e) return 1;

        e->win = xevent->ev_window;
        if (xevent->params[0] == 0)
          e->action = ECORE_GIX_WINDOW_STATE_ACTION_REMOVE;
        else if (xevent->params[0] == 1)
          e->action = ECORE_GIX_WINDOW_STATE_ACTION_ADD;
        else if (xevent->params[0] == 2)
          e->action = ECORE_GIX_WINDOW_STATE_ACTION_TOGGLE;
        else
          {
             free(e);
             return 1;
          }

        LOGFN(__FILE__, __LINE__, __FUNCTION__);
        e->state[0] = _ecore_x_netwm_state_get(xevent->params[1]);
        if (e->state[0] == ECORE_GIX_WINDOW_STATE_UNKNOWN)
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);
          }
        e->state[1] = _ecore_x_netwm_state_get(xevent->params[2]);
        if (e->state[1] == ECORE_GIX_WINDOW_STATE_UNKNOWN)
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);

          }

        e->source = xevent->params[3];

        ecore_event_add(ECORE_GIX_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->body.client.client_type == ECORE_GIX_ATOM_WM_CHANGE_STATE)
            && (xevent->body.client.client_format == 32)
            && (xevent->params[0] == IconicState))
     {
        Ecore_Gix_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_Gix_Event_Window_State_Request));
        if (!e)
          return 1;

        e->win = xevent->ev_window;
        e->action = ECORE_GIX_WINDOW_STATE_ACTION_ADD;
        e->state[0] = ECORE_GIX_WINDOW_STATE_ICONIFIED;

        ecore_event_add(ECORE_GIX_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   /*else if ((xevent->body.client.client_type == ECORE_GIX_ATOM_NET_WM_DESKTOP)
            && (xevent->body.client.client_format == 32))
     {
        Ecore_Gix_Event_Desktop_Change *e;

        e = calloc(1, sizeof(Ecore_Gix_Event_Desktop_Change));
        if (!e)
          return;

        e->win = xevent->ev_window;
        e->desk = xevent->params[0];
        e->source = xevent->params[1];

        ecore_event_add(ECORE_GIX_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if ((xevent->body.client.client_type ==
             ECORE_GIX_ATOM_NET_REQUEST_FRAME_EXTENTS))
     {
        Ecore_Gix_Event_Frame_Extents_Request *e;

        e = calloc(1, sizeof(Ecore_Gix_Event_Frame_Extents_Request));
        if (!e)
          return;

        e->win = xevent->ev_window;

        ecore_event_add(ECORE_GIX_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }*/
   else if ((xevent->body.client.client_type == ECORE_GIX_ATOM_WM_PROTOCOLS)
            && ((Ecore_Gix_Atom)xevent->params[0] ==
                ECORE_GIX_ATOM_NET_WM_PING)
            && (xevent->body.client.client_format == 32))
     {
        Ecore_Gix_Event_Ping *e;
        Ecore_Gix_Window root = 0;

        e = calloc(1, sizeof(Ecore_Gix_Event_Ping));
        if (!e)
          return;

        e->win = xevent->ev_window;
        e->time = xevent->params[1];
        e->event_win = xevent->params[2];

        /* send a reply anyway - we are alive... eventloop at least */
        ecore_event_add(ECORE_GIX_EVENT_PING, e, NULL, NULL);
        if (ScreenCount(_ecore_x_disp) > 1)
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             root = ecore_x_window_root_get(e->win);
          }
        else
          root = DefaultRootWindow(_ecore_x_disp);

        if (xevent->ev_window != root)
          {
             xevent->ev_window = root;
             XSendEvent(_ecore_x_disp, root, False,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        xevent);
          }
     }
   /*else if ((xevent->body.client.client_type ==
             ECORE_GIX_ATOM_NET_STARTUP_INFO_BEGIN) &&
            (xevent->body.client.client_format == 8))
     _ecore_x_netwm_startup_info_begin(xevent->ev_window,
                                       xevent->xclient.data.b);
   else if ((xevent->body.client.client_type == ECORE_GIX_ATOM_NET_STARTUP_INFO) &&
            (xevent->body.client.client_format == 8))
     _ecore_x_netwm_startup_info(xevent->ev_window,
                                 xevent->xclient.data.b);
   else if ((xevent->body.client.client_type == 27777)
            && (xevent->params[0] == 0x7162534)
            && (xevent->body.client.client_format == 32)
            && (xevent->ev_window == _ecore_x_private_win))
     {
        if (xevent->params[1] == 0x10000001)
          _ecore_x_window_grab_remove(xevent->params[2]);
        else if (xevent->params[1] == 0x10000002)
          _ecore_x_key_grab_remove(xevent->params[2]);
     }*/
   else
#endif
     {
        Ecore_Gix_Event_Client_Message *e;
        int i;

        e = calloc(1, sizeof(Ecore_Gix_Event_Client_Message));
        if (!e)
          return;

        e->win = xevent->ev_window;
        e->message_type = xevent->body.client.client_type;
        e->format = xevent->body.client.client_format;
        for (i = 0; i < 5; i++)
          e->data.l[i] = xevent->params[i];

        ecore_event_add(ECORE_GIX_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}


static Eina_Bool
_ecore_gix_window_event_fd_handler(void *data ,Ecore_Fd_Handler *fd_handler )
{
   gi_msg_t evt;
   int v = 0;

   while (gi_get_event_count() > 0)
   {
	   v = read(_window_event_fd, &evt, sizeof(gi_msg_t));
	   
	   if (v < 0)
		  return EINA_TRUE;

	   if (v < 1)
		  return EINA_TRUE;

	   if (event_filter_func && event_filter_func(&evt) != 0)
	   {
		   return EINA_TRUE;
	   }

	   _ecore_gix_event_last_time = evt.time;
#if 0
	   printf("%s got type %d, window %ld effect %ld [%d,%d]\n", 
		   __func__, evt.type, evt.ev_window, evt.effect_window,
		   evt.body.rect.x,evt.body.rect.y);
#endif

	   switch (evt.type)
	   {
	   case GI_MSG_EXPOSURE:
		   _ecore_gix_event_handle_expose(&evt);
		   break;
	   case GI_MSG_PROPERTYNOTIFY:
		   _ecore_gix_event_handle_property_notify(&evt);
	   break;
	   case GI_MSG_WINDOW_DESTROY:
		   _ecore_gix_event_handle_window_destroy(&evt);
	   break;
	   case GI_MSG_GESTURE_TOUCH:
		   _ecore_gix_event_handle_gesture_touch(&evt);
	   break;
	   case GI_MSG_FOCUS_IN:
		   _ecore_gix_event_handle_got_focus(&evt);
		   break;
	   case GI_MSG_FOCUS_OUT:
		   _ecore_gix_event_handle_lost_focus(&evt);
		   break;
	   case GI_MSG_MOUSE_MOVE:
			_ecore_gix_event_handle_motion(&evt);
		   break;
	   case GI_MSG_BUTTON_DOWN:
		   if (evt.params[2] & (GI_BUTTON_WHEEL_UP|GI_BUTTON_WHEEL_DOWN))
		   {
			   int step;
			   if(evt.params[2] & GI_BUTTON_WHEEL_UP)
				step = -1;
			   else
				step = 1;
			   _ecore_gix_event_handle_wheel(&evt,step);
		   }
		   else
			_ecore_gix_event_handle_button_down(&evt);
	   break;

	   case GI_MSG_BUTTON_UP:		   
		   _ecore_gix_event_handle_button_up(&evt);
	   break;

	   case GI_MSG_MOUSE_ENTER:
		   _ecore_gix_event_handle_enter(&evt);
		   //
	   break;
	   case GI_MSG_MOUSE_EXIT:
		   _ecore_gix_event_handle_leave(&evt);
		   //
	   break;
	   
	   case GI_MSG_CONFIGURENOTIFY:
		   _ecore_gix_event_handle_window_configure(&evt);
		   break;
	   case GI_MSG_KEY_UP:
		   _ecore_gix_event_handle_key_up(&evt);
	   break;
	   case GI_MSG_KEY_DOWN:
		   _ecore_gix_event_handle_key_down(&evt);
	   break;
	   case GI_MSG_CLIENT_MSG:
		  if(evt.body.client.client_type == GA_WM_PROTOCOLS
			  &&evt.params[0] == GA_WM_DELETE_WINDOW){
		   _ecore_gix_event_handle_window_delete_request(&evt);
		   break;
		}

		_ecore_gix_event_handle_client_message(&evt);
		break;

		case GI_MSG_WINDOW_SHOW:
			_ecore_gix_event_handle_window_show(&evt);
			break;
		case GI_MSG_WINDOW_HIDE:
			_ecore_gix_event_handle_window_hide(&evt);
			break;
		case GI_MSG_CREATENOTIFY:
			break;

		case GI_MSG_SELECTIONNOTIFY:
			if (evt.params[0] == G_SEL_REQUEST)
			{
			_ecore_x_event_handle_selection_request(&evt);
			}
			else if (evt.params[0] == G_SEL_NOTIFY)
			{
			_ecore_x_event_handle_selection_notify(&evt);
			}
			else if (evt.params[0] == G_SEL_CLEAR)
			{
			_ecore_x_event_handle_selection_clear(&evt);
			}
			else{
			}
		break;		

		default:
		   printf("%s: got line%d evt.type=%d\n",__FUNCTION__,__LINE__,evt.type);
		   break;	   
	   }
   }
   return EINA_TRUE;
} /* _ecore_gix_window_event_fd_handler */

/* api functions */
/*****************/

EAPI Ecore_Gix_Window  
ecore_gix_window_new(gi_window_id_t parent, unsigned long window_falgs, int x, int y, int w, int h)
{
   Ecore_Gix_Window  window;
   gi_window_id_t id;   

	id = gi_create_window(parent, x,y,w, h,
		GI_RGB(236, 233, 216),window_falgs);
	gi_set_events_mask(id, 
		GI_MASK_EXPOSURE| GI_MASK_MOUSE_ENTER| GI_MASK_MOUSE_EXIT
		| GI_MASK_BUTTON_DOWN| GI_MASK_BUTTON_UP| GI_MASK_MOUSE_MOVE
		| GI_MASK_KEY_DOWN| GI_MASK_KEY_UP|GI_MASK_FOCUS_IN| GI_MASK_FOCUS_OUT
		| GI_MASK_CLIENT_MSG| GI_MASK_WINDOW_DESTROY| GI_MASK_CONFIGURENOTIFY
		| GI_MASK_WINDOW_HIDE| GI_MASK_WINDOW_SHOW | GI_MASK_GESTURE_TOUCH
		| GI_MASK_WINDOW_DESTROY| GI_MASK_REPARENT);
	//
	//gi_show_window(id);

	if (parent == GI_DESKTOP_WINDOW_ID)
     ecore_x_window_defaults_set(id);

	//printf("ecore_gix_window_new: parent %ld, new window %ld\n", parent, id);

   //window = calloc(1,sizeof(Ecore_Gix_Window));
   //window->id = id;
   window = id;   
   return window;
} /* ecore_gix_window_new */

EAPI void
ecore_gix_window_free(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
   gi_destroy_window(ECORE_GIX_WINDOW_ID(ecore_window));
   //free(ecore_window);
} /* ecore_gix_window_free */

EAPI void
ecore_gix_window_move(Ecore_Gix_Window  ecore_window, int x, int y)
{
   if (!ecore_window) return;
   gi_move_window(ECORE_GIX_WINDOW_ID(ecore_window),x,y);
} /* ecore_gix_window_move */

EAPI void
ecore_gix_window_resize(Ecore_Gix_Window  ecore_window, int w, int h)
{
   gi_resize_window(ECORE_GIX_WINDOW_ID(ecore_window),w,h);
   //printf("ecore_gix_window_resize:window (%ld) to %d,%d\n",ECORE_GIX_WINDOW_ID(ecore_window),w,h);
} /* ecore_gix_window_resize */

EAPI void
ecore_gix_window_focus(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
   //printf("ecore_gix_window_focus:%ld called\n", ecore_window);
   gi_set_focus_window(ECORE_GIX_WINDOW_ID(ecore_window));
} /* ecore_gix_window_focus */


EAPI void
ecore_gix_window_title_set(Ecore_Gix_Window  window,
                             const char         *title)
{
   if (!window){
	   printf("%s got line%d\n",__FUNCTION__,__LINE__);
	   return;
   }
   if (!title || !title[0]) {
	   printf("%s got line%d\n",__FUNCTION__,__LINE__);
	   return;
   }

	gi_set_window_utf8_caption(ECORE_GIX_WINDOW_ID(window), title);
}

EAPI void
ecore_gix_window_raise(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
   gi_raise_window(ECORE_GIX_WINDOW_ID(ecore_window));
} 

EAPI void
ecore_gix_window_lower(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
   gi_lower_window(ECORE_GIX_WINDOW_ID(ecore_window));
} 

EAPI void
ecore_gix_window_hide(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
	gi_hide_window(ECORE_GIX_WINDOW_ID(ecore_window));
} /* ecore_gix_window_hide */

EAPI void
ecore_gix_window_show(Ecore_Gix_Window  ecore_window)
{
   if (!ecore_window) return;
	gi_show_window_all(ECORE_GIX_WINDOW_ID(ecore_window));
} /* ecore_gix_window_show */

EAPI void
ecore_gix_window_shaped_set(Ecore_Gix_Window  ecore_window, Eina_Bool set)
{
   if (!ecore_window) return;
   printf("%s: got line%d\n",__FUNCTION__,__LINE__);

} /* ecore_gix_window_shaped_set */

EAPI void
ecore_gix_window_cursor_show(Ecore_Gix_Window  ecore_window, Eina_Bool show)
{
   if (!ecore_window) return;
  
  //printf("%s: now work now %d\n",__FUNCTION__,show);	
} /* ecore_gix_window_cursor_show */


EAPI void
ecore_gix_window_fullscreen_set(Ecore_Gix_Window  ecore_window, Eina_Bool on)
{
   if (!ecore_window) return;

   // always release the surface (we are going to get a new one in both cases)
   if(on)
     {
	   gi_set_window_stack(ECORE_GIX_WINDOW_ID(ecore_window),G_WINDOW_LAYER_ABOVE_DOCK);
	   gi_fullscreen_window(ECORE_GIX_WINDOW_ID(ecore_window),TRUE);
     }
   else
     {
	   gi_set_window_stack(ECORE_GIX_WINDOW_ID(ecore_window),G_WINDOW_LAYER_NORMAL);
	   gi_fullscreen_window(ECORE_GIX_WINDOW_ID(ecore_window),FALSE);
     }
} /* ecore_gix_window_fullscreen_set */

EAPI void
ecore_gix_window_iconified_set(Ecore_Gix_Window  ecore_window,
                                 Eina_Bool           on)
{
   if (!ecore_window) return;
   if (!on)  {
	   gi_show_window(ECORE_GIX_WINDOW_ID(ecore_window));
   }
   else{
	   gi_hide_window(ECORE_GIX_WINDOW_ID(ecore_window));
   }
}

EAPI void
ecore_gix_window_borderless_set(Ecore_Gix_Window  ecore_window,
                                  Eina_Bool           on)
{
   if (!ecore_window) return;
   if (on){
	   gi_set_window_desc(ECORE_GIX_WINDOW_ID(ecore_window), "borderless:on");
   }
   else{
 	   gi_set_window_desc(ECORE_GIX_WINDOW_ID(ecore_window), "borderless:off");
  }
}

EAPI void
ecore_gix_window_type_set(Ecore_Gix_Window      window,
                            int  type)
{
   if (!window)
     return;
}

EAPI void
ecore_gix_window_size_get(Ecore_Gix_Window  ecore_window, int *w, int *h)
{
	gi_window_info_t info;
	int err;

   if (!ecore_window) return;
	err = gi_get_window_info(ECORE_GIX_WINDOW_ID(ecore_window),&info);
	if (!err){
	  *w = info.width;
	  *h = info.height;
	}
   return;
} /* ecore_gix_window_size_get */




void
_ecore_x_event_handle_selection_clear(gi_msg_t *xevent)
{
//   Ecore_Gix_Selection_Intern *d;
   Ecore_Gix_Event_Selection_Clear *e;
   Ecore_Gix_Atom sel;

   //LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
/* errr..... why? paranoia.
   d = _ecore_x_selection_get(gi_message_clear_selection_property(xevent));
   if (d && (gi_message_clear_selection_time(xevent) > d->time))
     {
        _ecore_x_selection_set(0, NULL, 0,
                               gi_message_clear_selection_property(xevent));
     }
 */
/* Generate event for app cleanup */
   e = malloc(sizeof(Ecore_Gix_Event_Selection_Clear));
   e->win = gi_message_clear_selection_window(xevent);
   e->time = gi_message_clear_selection_time(xevent);
   e->atom = sel = gi_message_clear_selection_property(xevent);
   if (sel == ECORE_GIX_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_GIX_SELECTION_PRIMARY;
   else if (sel == ECORE_GIX_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_GIX_SELECTION_SECONDARY;
   else if (sel == ECORE_GIX_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_GIX_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_GIX_SELECTION_OTHER;

   ecore_event_add(ECORE_GIX_EVENT_SELECTION_CLEAR, e, NULL, NULL);
}

void
_ecore_x_event_handle_selection_request(gi_msg_t *xevent)
{
   Ecore_Gix_Event_Selection_Request *e;
   Ecore_Gix_Selection_Intern *sd;
   void *data = NULL;
   int len;
   int typesize;

   //LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
   /*
    * Generate a selection request event.
    */
   e = malloc(sizeof(Ecore_Gix_Event_Selection_Request));
   e->owner = gi_message_request_selection_window(xevent);
   e->requestor = gi_message_request_selection_requestor(xevent);
   e->time = gi_message_request_selection_time(xevent);
   e->selection = gi_message_request_selection_selection(xevent);
   e->target = gi_message_request_selection_target(xevent);
   e->property = gi_message_request_selection_property(xevent);
   ecore_event_add(ECORE_GIX_EVENT_SELECTION_REQUEST, e, NULL, NULL);

   if ((sd = _ecore_x_selection_get(gi_message_request_selection_selection(xevent))) &&
       (sd->win == gi_message_request_selection_window(xevent)))
     {
        Ecore_Gix_Selection_Intern *si;

        si = _ecore_x_selection_get(gi_message_request_selection_selection(xevent));
        if (si->data)
          {
             Ecore_Gix_Atom property = 0;
             Ecore_Gix_Atom type;

             /* Set up defaults for strings first */
             type = gi_message_request_selection_target(xevent);
             typesize = 8;
             len = sd->length;

             if (!ecore_x_selection_convert(gi_message_request_selection_selection(xevent),
                                            gi_message_request_selection_target(xevent),
                                            &data, &len, &type, &typesize))
               /* Refuse selection, conversion to requested target failed */
               property = 0;
             else if (data)
               {
                  /* FIXME: This does not properly handle large data transfers */
                  ecore_x_window_prop_property_set(
                    gi_message_request_selection_requestor(xevent),
                    gi_message_request_selection_property(xevent),
                    type,
                    typesize,
                    data,
                    len);
                  property = gi_message_request_selection_property(xevent);
                  free(data);
               }

             ecore_x_selection_notify_send(gi_message_request_selection_requestor(xevent),
                                           gi_message_request_selection_selection(xevent),
                                           gi_message_request_selection_target(xevent),
                                           property,
                                           gi_message_request_selection_time(xevent));
          }
     }
}


static void
_ecore_x_event_free_selection_notify(void *data ,
                                     void *ev)
{
   Ecore_Gix_Event_Selection_Notify *e;
   Ecore_Gix_Selection_Data *sel;

   e = ev;
   sel = e->data;
   if (sel->free)
     sel->free(sel);

   free(e->target);
   free(e);
}

void
_ecore_x_event_handle_selection_notify(gi_msg_t *xevent)
{
   Ecore_Gix_Event_Selection_Notify *e;
   unsigned char *data = NULL;
   Ecore_Gix_Atom selection;
   int num_ret, format;

   //LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
   selection = gi_message_notify_selection_selection(xevent);

   if (gi_message_notify_selection_target(xevent) == ECORE_GIX_ATOM_SELECTION_TARGETS)
     {
        format = ecore_x_window_prop_property_get(gi_message_notify_selection_requestor(xevent),
                                                  gi_message_notify_selection_property(xevent),
                                                  GA_ATOM, 32, &data, &num_ret);
        if (!format)
          {
             /* fallback if targets handling is not working and try get the
              * selection directly */
             gi_convert_selection( selection,
                               GA_UTF8_STRING,
                               selection,
                               gi_message_notify_selection_requestor(xevent),
                               0);
             return;
          }
     }
   else
     {
        format = ecore_x_window_prop_property_get(gi_message_notify_selection_requestor(xevent),
                                                  gi_message_notify_selection_property(xevent),
                                                  G_ANY_PROP_TYPE, 8, &data,
                                                  &num_ret);
        if (!format)
          return;
     }

   e = calloc(1, sizeof(Ecore_Gix_Event_Selection_Notify));
   if (!e)
     return;

   e->win = gi_message_notify_selection_requestor(xevent);
   e->time = gi_message_notify_selection_time(xevent);
   e->atom = selection;
   e->target = _ecore_x_selection_target_get(gi_message_notify_selection_target(xevent));

   if (selection == ECORE_GIX_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_GIX_SELECTION_PRIMARY;
   else if (selection == ECORE_GIX_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_GIX_SELECTION_SECONDARY;
   else if (selection == ECORE_GIX_ATOM_SELECTION_XDND)
     e->selection = ECORE_GIX_SELECTION_XDND;
   else if (selection == ECORE_GIX_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_GIX_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_GIX_SELECTION_OTHER;

   e->data = _ecore_x_selection_parse(e->target, data, num_ret, format);

   ecore_event_add(ECORE_GIX_EVENT_SELECTION_NOTIFY, e,
                   _ecore_x_event_free_selection_notify, NULL);
}


/* dnd atoms */
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_XDND = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_XDND = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_AWARE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ENTER = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_TYPE_LIST = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_POSITION = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_COPY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_MOVE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_PRIVATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_ASK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_LIST = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_LINK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_DESCRIPTION = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_PROXY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_STATUS = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_LEAVE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_DROP = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_FINISHED = 0;

/* dnd atoms that need to be exposed to the application interface */
EAPI Ecore_Gix_Atom ECORE_GIX_DND_ACTION_COPY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_DND_ACTION_MOVE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_DND_ACTION_LINK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_DND_ACTION_ASK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_DND_ACTION_PRIVATE = 0;

Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_TARGETS;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PRIMARY;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_SECONDARY;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_CLIPBOARD;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_PRIMARY;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_SECONDARY;
Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_CLIPBOARD;


EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_COMPOUND_TEXT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_FILE_NAME = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_TEXT = 0;


/* currently E specific illume extension */
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ZONE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ZONE_LIST = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CONFORMANT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_SINGLE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_TOP = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_LEFT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_BACK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_FORWARD = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_HOME = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLOSE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_HOME_NEW = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_HOME_DEL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_LOCKED = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_START = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_END = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_GEOMETRY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_SOFTKEY_GEOMETRY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_KEYBOARD_GEOMETRY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ON = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_OFF = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ZONE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_STATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_ON = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OFF = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPAQUE= 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSLUCENT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSPARENT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_AVAILABLE_ANGLE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_STATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_ON = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_OFF = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE_NORMAL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE_FLOATING = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_CONTROL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_NEXT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_PREV = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV = 0;


EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_STATE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ON = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_OFF = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ALPHA = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PIN = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HEX = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_IP = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HOST = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_FILE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_URL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_J2ME = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_ICON_GEOMETRY = 0;

EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_WM_WINDOW_ROLE = 0;


/* window type */
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DESKTOP = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DOCK = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_MENU = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_UTILITY = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_SPLASH = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DIALOG = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NORMAL = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_COMBO = 0;
EAPI Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DND = 0;

typedef struct _Atom_Item Atom_Item;

struct _Atom_Item
{
   const char   *name;
   Ecore_Gix_Atom *atom;
};

const Atom_Item atom_items[] =
{
   { "_NET_WM_ICON_GEOMETRY", &ECORE_GIX_ATOM_NET_WM_ICON_GEOMETRY },
   { "WM_WINDOW_ROLE", &ECORE_GIX_ATOM_WM_WINDOW_ROLE },

   { "_E_VIRTUAL_KEYBOARD", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD },
   { "_E_VIRTUAL_KEYBOARD_STATE", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_STATE },
   { "_E_VIRTUAL_KEYBOARD_ON", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ON },
   { "_E_VIRTUAL_KEYBOARD_OFF", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_OFF },
   { "_E_VIRTUAL_KEYBOARD_ALPHA", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ALPHA },
   { "_E_VIRTUAL_KEYBOARD_NUMERIC", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC },
   { "_E_VIRTUAL_KEYBOARD_PIN", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PIN },
   { "_E_VIRTUAL_KEYBOARD_PHONE_NUMBER",
     &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER },
   { "_E_VIRTUAL_KEYBOARD_HEX", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HEX },
   { "_E_VIRTUAL_KEYBOARD_TERMINAL",
     &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL },
   { "_E_VIRTUAL_KEYBOARD_PASSWORD",
     &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD },
   { "_E_VIRTUAL_KEYBOARD_IP", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_IP },
   { "_E_VIRTUAL_KEYBOARD_HOST", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HOST },
   { "_E_VIRTUAL_KEYBOARD_FILE", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_FILE },
   { "_E_VIRTUAL_KEYBOARD_URL", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_URL },
   { "_E_VIRTUAL_KEYBOARD_KEYPAD", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD },
   { "_E_VIRTUAL_KEYBOARD_J2ME", &ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_J2ME },


   { "_E_ILLUME_ZONE", &ECORE_GIX_ATOM_E_ILLUME_ZONE },
   { "_E_ILLUME_ZONE_LIST", &ECORE_GIX_ATOM_E_ILLUME_ZONE_LIST },
   { "_E_ILLUME_CONFORMANT", &ECORE_GIX_ATOM_E_ILLUME_CONFORMANT },
   { "_E_ILLUME_MODE", &ECORE_GIX_ATOM_E_ILLUME_MODE },
   { "_E_ILLUME_MODE_SINGLE", &ECORE_GIX_ATOM_E_ILLUME_MODE_SINGLE },
   { "_E_ILLUME_MODE_DUAL_TOP", &ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_TOP },
   { "_E_ILLUME_MODE_DUAL_LEFT", &ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_LEFT },
   { "_E_ILLUME_FOCUS_BACK", &ECORE_GIX_ATOM_E_ILLUME_FOCUS_BACK },
   { "_E_ILLUME_FOCUS_FORWARD", &ECORE_GIX_ATOM_E_ILLUME_FOCUS_FORWARD },
   { "_E_ILLUME_FOCUS_HOME", &ECORE_GIX_ATOM_E_ILLUME_FOCUS_HOME },
   { "_E_ILLUME_CLOSE", &ECORE_GIX_ATOM_E_ILLUME_CLOSE },
   { "_E_ILLUME_HOME_NEW", &ECORE_GIX_ATOM_E_ILLUME_HOME_NEW },
   { "_E_ILLUME_HOME_DEL", &ECORE_GIX_ATOM_E_ILLUME_HOME_DEL },
   { "_E_ILLUME_DRAG", &ECORE_GIX_ATOM_E_ILLUME_DRAG },
   { "_E_ILLUME_DRAG_LOCKED", &ECORE_GIX_ATOM_E_ILLUME_DRAG_LOCKED },
   { "_E_ILLUME_DRAG_START", &ECORE_GIX_ATOM_E_ILLUME_DRAG_START },
   { "_E_ILLUME_DRAG_END", &ECORE_GIX_ATOM_E_ILLUME_DRAG_END },
   { "_E_ILLUME_INDICATOR_GEOMETRY",
     &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_GEOMETRY },
   { "_E_ILLUME_SOFTKEY_GEOMETRY", &ECORE_GIX_ATOM_E_ILLUME_SOFTKEY_GEOMETRY },
   { "_E_ILLUME_KEYBOARD_GEOMETRY", &ECORE_GIX_ATOM_E_ILLUME_KEYBOARD_GEOMETRY },
   { "_E_ILLUME_QUICKPANEL", &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL },
   { "_E_ILLUME_QUICKPANEL_STATE", &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE },
   { "_E_ILLUME_QUICKPANEL_STATE_TOGGLE",
     &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE },
   { "_E_ILLUME_QUICKPANEL_ON", &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ON },
   { "_E_ILLUME_QUICKPANEL_OFF", &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_OFF },
   { "_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR",
     &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR },
   { "_E_ILLUME_QUICKPANEL_PRIORITY_MINOR",
     &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR },
   { "_E_ILLUME_QUICKPANEL_ZONE", &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ZONE },
   { "_E_ILLUME_QUICKPANEL_POSITION_UPDATE",
     &ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE },
   { "_E_ILLUME_INDICATOR_STATE", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_STATE },
   { "_E_ILLUME_INDICATOR_ON", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_ON },
   { "_E_ILLUME_INDICATOR_OFF", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OFF },
   { "_E_ILLUME_INDICATOR_OPACITY_MODE", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE },
   { "_E_ILLUME_INDICATOR_OPAQUE", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPAQUE },
   { "_E_ILLUME_INDICATOR_TRANSLUCENT", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSLUCENT },
   { "_E_ILLUME_INDICATOR_TRANSPARENT", &ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSPARENT },
   { "_E_ILLUME_ROTATE_WINDOW_AVAILABLE_ANGLES", &ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_AVAILABLE_ANGLE },
   { "_E_ILLUME_ROTATE_WINDOW_ANGLE", &ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE },
   { "_E_ILLUME_ROTATE_ROOT_ANGLE", &ECORE_GIX_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE },
   { "_E_ILLUME_CLIPBOARD_STATE", &ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_STATE },
   { "_E_ILLUME_CLIPBOARD_ON", &ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_ON },
   { "_E_ILLUME_CLIPBOARD_OFF", &ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_OFF },
   { "_E_ILLUME_CLIPBOARD_GEOMETRY", &ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY },
   { "_E_ILLUME_ACCESS_CONTROL", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_CONTROL },
   { "_E_ILLUME_ACCESS_ACTION_NEXT", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_NEXT },
   { "_E_ILLUME_ACCESS_ACTION_PREV", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_PREV },
   { "_E_ILLUME_ACCESS_ACTION_ACTIVATE", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE },
   { "_E_ILLUME_ACCESS_ACTION_READ", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ },
   { "_E_ILLUME_ACCESS_ACTION_READ_NEXT", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT },
   { "_E_ILLUME_ACCESS_ACTION_READ_PREV", &ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV },


   { "COMPOUND_TEXT", &ECORE_GIX_ATOM_COMPOUND_TEXT },
   { "FILE_NAME", &ECORE_GIX_ATOM_FILE_NAME },
   { "TEXT", &ECORE_GIX_ATOM_TEXT },

   { "JXSelectionWindowProperty", &ECORE_GIX_ATOM_SELECTION_PROP_XDND },
   { "XdndSelection", &ECORE_GIX_ATOM_SELECTION_XDND },
   { "XdndAware", &ECORE_GIX_ATOM_XDND_AWARE },
   { "XdndEnter", &ECORE_GIX_ATOM_XDND_ENTER },
   { "XdndTypeList", &ECORE_GIX_ATOM_XDND_TYPE_LIST },
   { "XdndPosition", &ECORE_GIX_ATOM_XDND_POSITION },
   { "XdndActionCopy", &ECORE_GIX_ATOM_XDND_ACTION_COPY },
   { "XdndActionMove", &ECORE_GIX_ATOM_XDND_ACTION_MOVE },
   { "XdndActionPrivate", &ECORE_GIX_ATOM_XDND_ACTION_PRIVATE },
   { "XdndActionAsk", &ECORE_GIX_ATOM_XDND_ACTION_ASK },
   { "XdndActionList", &ECORE_GIX_ATOM_XDND_ACTION_LIST },
   { "XdndActionLink", &ECORE_GIX_ATOM_XDND_ACTION_LINK },
   { "XdndActionDescription", &ECORE_GIX_ATOM_XDND_ACTION_DESCRIPTION },
   { "XdndProxy", &ECORE_GIX_ATOM_XDND_PROXY },
   { "XdndStatus", &ECORE_GIX_ATOM_XDND_STATUS },
   { "XdndLeave", &ECORE_GIX_ATOM_XDND_LEAVE },
   { "XdndDrop", &ECORE_GIX_ATOM_XDND_DROP },
   { "XdndFinished", &ECORE_GIX_ATOM_XDND_FINISHED },

   { "XdndActionCopy", &ECORE_GIX_DND_ACTION_COPY },
   { "XdndActionMove", &ECORE_GIX_DND_ACTION_MOVE },
   { "XdndActionLink", &ECORE_GIX_DND_ACTION_LINK },
   { "XdndActionAsk", &ECORE_GIX_DND_ACTION_ASK },
   { "XdndActionPrivate", &ECORE_GIX_DND_ACTION_PRIVATE },


   { "_NET_WM_WINDOW_TYPE", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE },
   { "_NET_WM_WINDOW_TYPE_DESKTOP", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DESKTOP },
   { "_NET_WM_WINDOW_TYPE_DOCK", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DOCK },
   { "_NET_WM_WINDOW_TYPE_TOOLBAR", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR },
   { "_NET_WM_WINDOW_TYPE_MENU", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_MENU },
   { "_NET_WM_WINDOW_TYPE_UTILITY", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_UTILITY },
   { "_NET_WM_WINDOW_TYPE_SPLASH", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_SPLASH },
   { "_NET_WM_WINDOW_TYPE_DIALOG", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DIALOG },
   { "_NET_WM_WINDOW_TYPE_NORMAL", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NORMAL },
   { "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
     &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU },
   { "_NET_WM_WINDOW_TYPE_POPUP_MENU",
     &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU },
   { "_NET_WM_WINDOW_TYPE_TOOLTIP", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP },
   { "_NET_WM_WINDOW_TYPE_NOTIFICATION",
     &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION },
   { "_NET_WM_WINDOW_TYPE_COMBO", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_COMBO },
   { "_NET_WM_WINDOW_TYPE_DND", &ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DND },


   { "TARGETS", &ECORE_GIX_ATOM_SELECTION_TARGETS },
   { "CLIPBOARD", &ECORE_GIX_ATOM_SELECTION_CLIPBOARD },
   { "PRIMARY", &ECORE_GIX_ATOM_SELECTION_PRIMARY },
   { "SECONDARY", &ECORE_GIX_ATOM_SELECTION_SECONDARY },
   { "_ECORE_SELECTION_PRIMARY", &ECORE_GIX_ATOM_SELECTION_PROP_PRIMARY },
   { "_ECORE_SELECTION_SECONDARY", &ECORE_GIX_ATOM_SELECTION_PROP_SECONDARY },
   { "_ECORE_SELECTION_CLIPBOARD", &ECORE_GIX_ATOM_SELECTION_PROP_CLIPBOARD }
};


static void
_ecore_x_atoms_init(void)
{
   //Atom *atoms;
   //char **names;
   int i, num;

   num = sizeof(atom_items) / sizeof(Atom_Item);
   //atoms = alloca(num * sizeof(Atom));
   //names = alloca(num * sizeof(char *));
   //for (i = 0; i < num; i++)
   //  names[i] = (char *) atom_items[i].name;
   //XInternAtoms( names, num, False, atoms);
   for (i = 0; i < num; i++){
     *(atom_items[i].atom) = gi_intern_atom(atom_items[i].name, FALSE);
   	}
}



EAPI int
ecore_gix_init(const char *name )
{
   int i = 0;

   if (++_ecore_gix_init_count != 1)
      return _ecore_gix_init_count;

   if (!eina_init())
     return --_ecore_gix_init_count;
   gi_init();  


   _ecore_gix_log_dom = eina_log_domain_register
     ("ecore_gix", ECORE_GIX_DEFAULT_LOG_COLOR);
   if(_ecore_gix_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the Ecore directFB module.");
        return _ecore_gix_init_count--;
     }

   if (!ecore_init())
     {
        EINA_LOG_ERR("Ecore_Win32: Could not init ecore_init");
        //goto unregister_log_domain;
     }

	 if (!ecore_event_init())
     {
        EINA_LOG_ERR("Ecore_Win32: Could not init ecore_event");
        //goto unregister_log_domain;
     }

	 event_filter_func = NULL;

  

   _window_event_fd = gi_get_connection_fd();
   _window_event_fd_handler_handle = ecore_main_fd_handler_add(_window_event_fd,
	   ECORE_FD_READ,_ecore_gix_window_event_fd_handler, NULL,NULL,NULL);

   /* register ecore gix events */
   ECORE_GIX_EVENT_WINDOW_PROPERTY = ecore_event_type_new();
   ECORE_GIX_EVENT_CLIENT_MESSAGE = ecore_event_type_new();
   ECORE_GIX_EVENT_DESTROYED = ecore_event_type_new();
   ECORE_GIX_EVENT_GOT_FOCUS = ecore_event_type_new();
   ECORE_GIX_EVENT_LOST_FOCUS = ecore_event_type_new();  
   ECORE_GIX_EVENT_WINDOW_DAMAGE = ecore_event_type_new();
   ECORE_GIX_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
   ECORE_GIX_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
   ECORE_GIX_EVENT_WINDOW_SHOW = ecore_event_type_new();
   ECORE_GIX_EVENT_WINDOW_HIDE = ecore_event_type_new();

   ECORE_GIX_EVENT_SELECTION_CLEAR = ecore_event_type_new();
   ECORE_GIX_EVENT_SELECTION_REQUEST = ecore_event_type_new();
   ECORE_GIX_EVENT_SELECTION_NOTIFY = ecore_event_type_new();

   ECORE_GIX_EVENT_GESTURE_TOUCH = ecore_event_type_new();
   ECORE_GIX_EVENT_MOUSE_IN = ecore_event_type_new();
   ECORE_GIX_EVENT_MOUSE_OUT = ecore_event_type_new();

   _ecore_x_atoms_init();
   _ecore_x_selection_data_init();
   _ecore_x_dnd_init();

 
   /* create the hash for the windows(key = windowid, val = Ecore_Gix_Window struct) */
   return _ecore_gix_init_count;
} /* ecore_gix_init */

EAPI int
ecore_gix_shutdown(void)
{
   if (--_ecore_gix_init_count != 0)
      return _ecore_gix_init_count;

   ecore_main_fd_handler_del(_window_event_fd_handler_handle);
  
  //printf("%s: got line%d\n",__FUNCTION__,__LINE__);

   eina_log_domain_unregister(_ecore_gix_log_dom);
   _ecore_gix_log_dom = -1;
   ecore_event_shutdown();
   eina_shutdown();
   event_filter_func = NULL;
   gi_exit();
   return _ecore_gix_init_count;
} /* ecore_gix_shutdown */


EAPI void
ecore_x_netwm_icons_set(Ecore_Gix_Window win,
                        Ecore_Gix_Icon *icon,
                        int num)
{
   unsigned int *data, *p, *p2;
   unsigned int i, size, x, y;
   
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   size = 0;
   for (i = 0; i < (unsigned int)num; i++)
     {
        size += 2 + (icon[i].width * icon[i].height);
     }
   data = malloc(size * sizeof(unsigned int));
   if (!data) return;
   p = data;
   for (i = 0; i < (unsigned int)num; i++)
     {
        p[0] = icon[i].width;
        p[1] = icon[i].height;
        p += 2;
        p2 = icon[i].data;
        for (y = 0; y < icon[i].height; y++)
          {
             for (x = 0; x < icon[i].width; x++)
               {
                  unsigned int r, g, b, a;
                  
                  a = (*p2 >> 24) & 0xff;
                  r = (*p2 >> 16) & 0xff;
                  g = (*p2 >> 8 ) & 0xff;
                  b = (*p2      ) & 0xff;
                  if ((a > 0) && (a < 255))
                    {
                       // unpremul
                       r = (r * 255) / a;
                       g = (g * 255) / a;
                       b = (b * 255) / a;
                    }
                  *p = (a << 24) | (r << 16) | (g << 8) | b;
                  p++;
                  p2++;
               }
          }
     }
   ecore_x_window_prop_card32_set(win, GA_NET_WM_ICON,
                                  data, size);
   free(data);
}

EAPI Eina_Bool
ecore_x_netwm_icons_get(Ecore_Gix_Window win,
                        Ecore_Gix_Icon **icon,
                        int *num)
{
   unsigned int *data, *p;
   unsigned int *src;
   unsigned int len, icons, i;
   int num_ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num)
     *num = 0;

   if (icon)
     *icon = NULL;

   num_ret = ecore_x_window_prop_card32_list_get(win, GA_NET_WM_ICON,
                                                 &data);
   if (num_ret <= 0)
     return EINA_FALSE;

   if (!data)
     return EINA_FALSE;

   if (num_ret < 2)
     {
        free(data);
        return EINA_FALSE;
     }

   /* Check how many icons there are */
   icons = 0;
   p = data;
   while (p)
     {
        len = p[0] * p[1];
        p += (len + 2);
        if ((p - data) > num_ret)
          {
             free(data);
             return EINA_FALSE;
          }

        icons++;

        if ((p - data) == num_ret)
          p = NULL;
     }
   if (num)
     *num = icons;

   /* If the user doesn't want the icons, return */
   if (!icon)
     {
        free(data);
        return EINA_TRUE;
     }

   /* Allocate memory */
   *icon = malloc(icons * sizeof(Ecore_Gix_Icon));
   if (!(*icon))
     {
        free(data);
        return EINA_FALSE;
     }

   /* Fetch the icons */
   p = data;
   for (i = 0; i < icons; i++)
     {
        unsigned int *ps, *pd, *pe;

        len = p[0] * p[1];
        ((*icon)[i]).width = p[0];
        ((*icon)[i]).height = p[1];
        src = &(p[2]);
        ((*icon)[i]).data = malloc(len * sizeof(unsigned int));
        if (!((*icon)[i]).data)
          {
             while (i)
               free(((*icon)[--i]).data);
             free(*icon);
             free(data);
             return EINA_FALSE;
          }

        pd = ((*icon)[i]).data;
        ps = src;
        pe = ps + len;
        for (; ps < pe; ps++)
          {
             unsigned int r, g, b, a;

             a = (*ps >> 24) & 0xff;
             r = (((*ps >> 16) & 0xff) * a) / 255;
             g = (((*ps >> 8) & 0xff) * a) / 255;
             b = (((*ps) & 0xff) * a) / 255;
             *pd = (a << 24) | (r << 16) | (g << 8) | (b);
             pd++;
          }
        p += (len + 2);
     }

   free(data);

   return EINA_TRUE;
}

EAPI void
ecore_x_netwm_icon_geometry_set(Ecore_Gix_Window win,
                                int x,
                                int y,
                                int width,
                                int height)
{
   unsigned int geometry[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   geometry[0] = x;
   geometry[1] = y;
   geometry[2] = width;
   geometry[3] = height;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_GIX_ATOM_NET_WM_ICON_GEOMETRY,
                                  geometry,
                                  4);
}

EAPI Eina_Bool
ecore_x_netwm_icon_geometry_get(Ecore_Gix_Window win,
                                int *x,
                                int *y,
                                int *width,
                                int *height)
{
   int ret;
   unsigned int geometry[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = ecore_x_window_prop_card32_get(win,
                                        ECORE_GIX_ATOM_NET_WM_ICON_GEOMETRY,
                                        geometry,
                                        4);
   if (ret != 4)
     return EINA_FALSE;

   if (x)
     *x = geometry[0];

   if (y)
     *y = geometry[1];

   if (width)
     *width = geometry[2];

   if (height)
     *height = geometry[3];

   return EINA_TRUE;
}

