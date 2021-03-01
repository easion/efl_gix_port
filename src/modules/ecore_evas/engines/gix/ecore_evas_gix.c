
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */
#include <inttypes.h> /* for UINT_MAX */

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Gix.h>

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include <string.h>

# ifdef BUILD_ECORE_EVAS_OPENGL_GIX
#  include <Evas_Engine_GL_Gix.h>
# endif
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>

#ifdef BUILD_ECORE_EVAS_GIX
//#include <Evas.h>
#include "../../../evas/engines/software_gix/Evas_Engine_Gix.h"
#define __UNUSED__
#else
#error "BUILD_ECORE_EVAS_GIX not defined"
#endif

#ifdef BUILD_ECORE_EVAS_GIX
static int _ecore_evas_init_count = 0;
#define ECORE_EVAS_EVENT_COUNT 10
static Ecore_Event_Handler *ecore_evas_event_handlers[ECORE_EVAS_EVENT_COUNT];

typedef struct _Ecore_Evas_Engine_Data_X11 Ecore_Evas_Engine_Data_X11;


struct _Ecore_Evas_Engine_Data_X11 {
   Ecore_Gix_Window win_root;
   Eina_List     *win_extra;
   Ecore_X_Pixmap pmap;
   Ecore_X_Pixmap mask;
  // Ecore_X_GC     gc;
 //  Ecore_X_XRegion *damages;
   Ecore_Timer     *outdelay;
  // Ecore_X_Event_Mouse_Out out_ev;
   Ecore_Gix_Window leader;
   //Ecore_X_Sync_Counter netwm_sync_counter;
   int            configure_reqs;
   int            netwm_sync_val_hi;
   unsigned int   netwm_sync_val_lo;
   int            screen_num;
   int            px, py, pw, ph;
   unsigned char  direct_resize : 1;
   unsigned char  using_bg_pixmap : 1;
   unsigned char  managed : 1;
   unsigned char  netwm_sync_set : 1;
   unsigned char  configure_coming : 1;
   struct {
     unsigned char modal : 1;
     unsigned char sticky : 1;
     unsigned char maximized_v : 1;
     unsigned char maximized_h : 1;
     unsigned char shaded : 1;
     unsigned char skip_taskbar : 1;
     unsigned char skip_pager : 1;
     unsigned char fullscreen : 1;
     unsigned char above : 1;
     unsigned char below : 1;
   } state;
   struct {
     unsigned char available : 1; // need to setup available profiles in a w
     unsigned char change : 1; // need to send change event to the WM
     unsigned char done : 1; // need to send change done event to the WM
   } profile;
   struct {
     unsigned char supported: 1;
     unsigned char prepare : 1;
     unsigned char request : 1;
     unsigned char done : 1;
     unsigned char configure_coming : 1;
     Ecore_Job    *manual_mode_job;
   } wm_rot;
   Ecore_Gix_Window win_shaped_input;
   struct
     {
        unsigned int front, back; // front and back pixmaps (double-buffer)
        Evas_Coord w, h; // store pixmap size (saves X roundtrips)
        int depth; // store depth to save us from fetching engine info pre_render
        void *visual; // store visual used to create pixmap
        unsigned long colormap; // store colormap used to create pixmap
     } pixmap;
   //Ecore_Evas_X11_Selection_Data selection_data[ECORE_EVAS_SELECTION_BUFFER_LAST];
   Eina_Array *xserver_atom_name_during_dnd;
   Ecore_Event_Handler *mouse_up_handler;
   Ecore_Job *init_job;
   int skip_clean_event;
   Eina_Bool destroyed : 1; // X window has been deleted and cannot be used
   Eina_Bool fully_obscured : 1; // X window is fully obscured
   Eina_Bool configured : 1; // X window has been configured
};

Ecore_Gix_Window  
ecore_gix_window_new(gi_window_id_t parent, unsigned long window_falgs, int x, int y, int w, int h);

#define EDBG(...)                                                       \
  EINA_LOG(_ecore_evas_log_dom, EINA_LOG_LEVEL_DBG + 1, __VA_ARGS__);

static int
_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

#if 0
   if (ee->prop.avoid_damage)
     {
        if (edata->using_bg_pixmap)
          {
             if (updates)
               {
                  Eina_List *l = NULL;
                  Eina_Rectangle *r;

                  EINA_LIST_FOREACH(updates, l, r)
                    ecore_x_window_area_clear(ee->prop.window,
                                              r->x, r->y, r->w, r->h);
                  if (ee->shaped)
                    {
                       ecore_x_window_shape_mask_set(ee->prop.window,
                                                     edata->mask);
                    }
                  if (ee->alpha)
                    {
//                     ecore_x_window_shape_input_mask_set(ee->prop.window, edata->mask);
                    }
                  _ecore_evas_idle_timeout_update(ee);
                  rend = 1;
               }
          }
        else
          {
             if (updates)
               {
                  Eina_List *l = NULL;
                  Eina_Rectangle *r;

                  EINA_LIST_FOREACH(updates, l, r)
                    {
                       Ecore_X_Rectangle rect;
                       Ecore_X_XRegion *tmpr;

                       if (!edata->damages)
                         edata->damages = ecore_x_xregion_new();
                       tmpr = ecore_x_xregion_new();
                       if (ee->rotation == 0)
                         {
                            rect.x = r->x;
                            rect.y = r->y;
                            rect.width = r->w;
                            rect.height = r->h;
                         }
                       else if (ee->rotation == 90)
                         {
                            rect.x = r->y;
                            rect.y = ee->h - r->x - r->w;
                            rect.width = r->h;
                            rect.height = r->w;
                         }
                       else if (ee->rotation == 180)
                         {
                            rect.x = ee->w - r->x - r->w;
                            rect.y = ee->h - r->y - r->h;
                            rect.width = r->w;
                            rect.height = r->h;
                         }
                       else if (ee->rotation == 270)
                         {
                            rect.x = ee->w - r->y - r->h;
                            rect.y = r->x;
                            rect.width = r->h;
                            rect.height = r->w;
                         }
                       ecore_x_xregion_union_rect(tmpr, edata->damages,
                                                  &rect);
                       ecore_x_xregion_free(edata->damages);
                       edata->damages = tmpr;
                    }
                  if (edata->damages)
                    {
                       if (ee->shaped)
                         {
                            /* if we have a damage pixmap - we can avoid exposures by
                             * disabling them just for setting the mask */
                            ecore_x_event_mask_unset(ee->prop.window, ECORE_X_EVENT_MASK_WINDOW_DAMAGE);
                            ecore_x_window_shape_mask_set(ee->prop.window,
                                                          edata->mask);
                            /* and re-enable them again */
                            ecore_x_event_mask_set(ee->prop.window, ECORE_X_EVENT_MASK_WINDOW_DAMAGE);
                         }
                       ecore_x_xregion_set(edata->damages, edata->gc);
                       ecore_x_pixmap_paste(edata->pmap, ee->prop.window,
                                            edata->gc, 0, 0, ee->w, ee->h,
                                            0, 0);
                       ecore_x_xregion_free(edata->damages);
                       edata->damages = NULL;
                    }
                  _ecore_evas_idle_timeout_update(ee);
                  rend = 1;
               }
          }
     }
   else if (((ee->visible) && (!ee->draw_block)) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (updates)
          {
             if (ee->shaped)
               {
                  ecore_x_window_shape_mask_set(ee->prop.window,
                                                edata->mask);
               }
             if (ee->alpha)
               {
//                ecore_x_window_shape_input_mask_set(ee->prop.window, edata->mask);
               }
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }
     }
   else
#endif
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

static int
_ecore_evas_gix_render(Ecore_Evas *ee)
{
   int rend = 0;

   if (ee->in_async_render)
     {
        EDBG("ee=%p is rendering asynchronously, skip.", ee);
        return 0;
     }

   rend = ecore_evas_render_prepare(ee);

   if (!ee->can_async_render)
     {
        Eina_List *updates = evas_render_updates(ee->evas);
        rend = _render_updates_process(ee, updates);
        evas_render_updates_free(updates);
     }
   else if (evas_render_async(ee->evas))
     {
        EDBG("ee=%p started asynchronous render.", ee);
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }
   else if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}



static Eina_Bool
_ecore_evas_gix_event_got_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Got_Focus *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//xx// filtering with these doesnt help   
//xx//   if (e->mode == ECORE_X_EVENT_MODE_UNGRAB) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   DBG("%s: Got line %d\n",__func__,__LINE__);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_gix_event_lost_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Lost_Focus *e;


   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//xx// filtering with these doesnt help   
//xx//   if (e->mode == ECORE_X_EVENT_MODE_GRAB) return ECORE_CALLBACK_PASS_ON;

//   if (ee->prop.fullscreen)
//     ecore_x_window_focus(ee->prop.window);
   evas_focus_out(ee->evas);
   ee->prop.focused = 0;
   DBG("%s: Got line %d\n",__func__,__LINE__);
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}


static Eina_Bool
_ecore_evas_gix_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Mouse_In *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   // disable. causes more problems than it fixes
   //   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) ||
   //       (e->mode == ECORE_X_EVENT_MODE_UNGRAB))
   //     return 0;
   /* if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0; */
   _ecore_evas_mouse_inout_set(ee, NULL, EINA_TRUE, EINA_FALSE);
   //if (!ee->in)
    // {
        //if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
        ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
        evas_event_feed_mouse_in(ee->evas, e->time, NULL);
		evas_focus_in(ee->evas);
        _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
     //   ee->in = EINA_TRUE;
   //  }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_gix_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Mouse_Out *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   /* if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0; */
//   DBG("OUT: ee->in=%i, e->mode=%i, e->detail=%i, dount_count=%i\n",
//          ee->in, e->mode, e->detail, evas_event_down_count_get(ee->evas));
  // if (ee->in)
        _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
  if (_ecore_evas_mouse_in_check(ee, NULL))
     {
	  if (evas_event_down_count_get(ee->evas) > 0) return ECORE_CALLBACK_PASS_ON;
        /*if ((evas_event_down_count_get(ee->evas) > 0) &&
            (!((e->mode == ECORE_GIX_EVENT_MODE_GRAB) &&
               (e->detail == ECORE_X_EVENT_DETAIL_NON_LINEAR))))
          return ECORE_CALLBACK_PASS_ON;*/
        ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
        //if (e->mode == ECORE_GIX_EVENT_MODE_GRAB)
        //  evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
        evas_event_feed_mouse_out(ee->evas, e->time, NULL);
        //if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
		_ecore_evas_mouse_inout_set(ee, NULL, EINA_FALSE, EINA_FALSE);
        //if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
		_ecore_evas_default_cursor_hide(ee);
        //ee->in = EINA_FALSE;
     }
   return ECORE_CALLBACK_PASS_ON;
}



static Eina_Bool
_ecore_evas_gix_event_destroy_window(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Window_Destroy *e;

   e = event;
   ee = ecore_event_window_match(e->win);  

   if (!ee) return EINA_TRUE; /* pass on event */
   if (e->win != ee->prop.window) return EINA_TRUE;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   //_ecore_evas_gix_sync_clear(ee);   
  ecore_evas_free(ee);
   return EINA_TRUE;
}

static Eina_Bool
_ecore_evas_gix_event_expose(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Window_Damage *e;

   e = event;
   ee = ecore_event_window_match(e->win);

  if (!ee){
	  DBG("%s: Got line %d, window not found\n",__func__,__LINE__);
	  return EINA_TRUE; /* pass on event */
  }
   if (e->win != ee->prop.window){
	   DBG("%s: Got line %d, window not dest\n",__func__,__LINE__);
	   return EINA_TRUE;
   }
   //if (ee->engine.x.using_bg_pixmap) return EINA_TRUE;

   /*if (ee->prop.avoid_damage)
     {
        Ecore_X_Rectangle rect;
        Ecore_X_XRegion  *tmpr;

        if (!ee->engine.x.damages) ee->engine.x.damages = ecore_gix_xregion_new();
        tmpr = ecore_gix_xregion_new();
        rect.x = e->x;
        rect.y = e->y;
        rect.width = e->w;
        rect.height = e->h;
        ecore_gix_xregion_union_rect(tmpr, ee->engine.x.damages, &rect);
        ecore_gix_xregion_free(ee->engine.x.damages);
        ee->engine.x.damages = tmpr;
		
		DBG("%s: Got line %d, avoid_damage..\n",__func__,__LINE__);
     }
   else*/
     {
        if (ee->rotation == 0)
          evas_damage_rectangle_add(ee->evas,
                                    e->x,
                                    e->y,
                                    e->w, e->h);
        else if (ee->rotation == 90)
          evas_damage_rectangle_add(ee->evas,
                                    ee->h - e->y - e->h,
                                    e->x,
                                    e->h, e->w);
        else if (ee->rotation == 180)
          evas_damage_rectangle_add(ee->evas,
                                    ee->w - e->x - e->w,
                                    ee->h - e->y - e->h,
                                    e->w, e->h);
        else if (ee->rotation == 270)
          evas_damage_rectangle_add(ee->evas,
                                    e->y,
                                    ee->w - e->x - e->w,
                                    e->h, e->w);
     }
   //return EINA_TRUE;
   return EINA_TRUE;
}


static Eina_Bool
_ecore_evas_gix_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                              *ee;
   Ecore_Gix_Event_Window_Delete_Request *e;

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   return ECORE_CALLBACK_PASS_ON;
}


static Eina_Bool
_ecore_evas_gix_event_configure_window(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   const Evas_Device *pointer;
   Ecore_Evas_Cursor *cursor;
   Ecore_Evas *ee;
   Ecore_Gix_Event_Window_Configure *e;


   INF("window configure");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   ee->draw_block = EINA_FALSE;
   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   pointer = evas_device_parent_get(pointer);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cursor, 1);

   if (ee->prop.override)
     {
        if ((ee->x != e->x) || (ee->y != e->y))
          {
             ee->x = e->x;
             ee->y = e->y;
             ee->req.x = ee->x;
             ee->req.y = ee->y;

             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
     }

   if ((ee->w != e->w) || (ee->h != e->h))
     {
        ee->w = e->w;
        ee->h = e->h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
/*         if (ee->shaped) */
/*           _ecore_evas_win32_region_border_resize(ee); */
        if ((ee->expecting_resize.w > 0) &&
            (ee->expecting_resize.h > 0))
          {
             if ((ee->expecting_resize.w == ee->w) &&
                 (ee->expecting_resize.h == ee->h))
               _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                              ecore_gix_current_time_get());
             ee->expecting_resize.w = 0;
             ee->expecting_resize.h = 0;
          }
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

#if 0
static Eina_Bool
_ecore_evas_gix_event_configure_window(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                         *ee;
   Ecore_Gix_Event_Window_Configure *e;


   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   
   //return ECORE_CALLBACK_PASS_ON;
   if ((ee->x != e->x) || (ee->y != e->y))
     {
	   //DBG("window move(%d,%d)\n", e->x,e->y);
        ee->x = e->x;
        ee->y = e->y;
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }


   if ((ee->w != e->w) || (ee->h != e->h))
     {
	   //DBG("window resize(%d,%d)\n", e->w,e->h);
		//DBG("############window configure RESIZE_START(%d,%d) (%d,%d)####################\n",ee->w,ee->h,e->w,e->h);
        ee->w = e->w;
        ee->h = e->h;
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
			DBG("############window configure RESIZE1 ####################\n");
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }

        if (ee->prop.avoid_damage)
          {
			DBG("############window configure RESIZE3 ####################\n");
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, 1);
          }
/*         if (ee->shaped) */
/*           _ecore_evas_gix_region_border_resize(ee); */
        if ((ee->expecting_resize.w > 0) &&
            (ee->expecting_resize.h > 0))
          {
             if ((ee->expecting_resize.w == ee->w) &&
                 (ee->expecting_resize.h == ee->h))
               _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                              ecore_gix_current_time_get());
			 DBG("############window configure RESIZE4 ####################\n");
             ee->expecting_resize.w = 0;
             ee->expecting_resize.h = 0;
          }
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }


   return ECORE_CALLBACK_PASS_ON;
}
#endif

static Eina_Bool
_ecore_evas_gix_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_Gix_Event_Window_Show *e;

   //DBG("window show\n");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   /* some GL drivers are doing buffer copy in a separate thread.
    * we need to check whether GL driver sends SYNC_DRAW_DONE msg afger copying
    * that are required in order to exactly render. - added by gl77.lee
    */
#ifdef BUILD_ECORE_EVAS_OPENGL_GIX
   static int first_map_bug = -1;

   if (ee->gl_sync_draw_done < 0)
     {
        if (getenv("ECORE_EVAS_GL_SYNC_DRAW_DONE"))
           ee->gl_sync_draw_done = atoi(getenv("ECORE_EVAS_GL_SYNC_DRAW_DONE"));
        else
           ee->gl_sync_draw_done = 0;
     }
   if (first_map_bug < 0)
     {
        char *bug = NULL;

        if ((bug = getenv("ECORE_EVAS_GL_FIRST_MAP_BUG")))
          first_map_bug = atoi(bug);
        else
          first_map_bug = 0;
     }

   if ((first_map_bug) && (!strcmp(ee->driver, "opengl_gix")))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
#endif
   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   if (ee->visible) return ECORE_CALLBACK_PASS_ON; /* dont pass it on */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_gix_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_Gix_Event_Window_Hide *e;

   //DBG("window hide\n");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   //if (ee->in)
     {
        //evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
        //evas_event_feed_mouse_out(ee->evas, e->time, NULL);
        //if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
        //if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
        //ee->in = EINA_FALSE;
     }
   if (!ee->visible) return ECORE_CALLBACK_PASS_ON; /* dont pass it on */
   ee->prop.withdrawn = EINA_TRUE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);

   return ECORE_CALLBACK_PASS_ON;
}

#if 0
static Eina_Bool
_ecore_evas_gix_event_property_change(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Gix_Event_Window_Property *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_gix_event_client_message(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Client_Message *e;

   e = event;
   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_BEGIN)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window) return ECORE_CALLBACK_PASS_ON;
        if (!ee->engine.x.sync_began)
          {
             // qeue a damage + draw. work around an event re-ordering thing.
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        ee->engine.x.sync_began = 1;
        ee->engine.x.sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_END)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window) return ECORE_CALLBACK_PASS_ON;
        ee->engine.x.sync_began = 0;
        ee->engine.x.sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_CANCEL)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window) return ECORE_CALLBACK_PASS_ON;
        ee->engine.x.sync_began = 0;
        ee->engine.x.sync_cancel = 1;
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

int
_ecore_evas_gix_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_GIX_EVENT_GOT_FOCUS, _ecore_evas_gix_event_got_focus, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_GIX_EVENT_LOST_FOCUS, _ecore_evas_gix_event_lost_focus, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_DAMAGE, _ecore_evas_gix_event_expose, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_GIX_EVENT_DESTROYED, _ecore_evas_gix_event_destroy_window, NULL);
   ecore_evas_event_handlers[4] = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_CONFIGURE, _ecore_evas_gix_event_configure_window, NULL);
   ecore_evas_event_handlers[5] = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_gix_event_window_delete_request, NULL);
   ecore_evas_event_handlers[6] = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_SHOW, _ecore_evas_gix_event_window_show, NULL);
   ecore_evas_event_handlers[7] = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_HIDE, _ecore_evas_gix_event_window_hide, NULL);

   ecore_evas_event_handlers[8] = ecore_event_handler_add(ECORE_GIX_EVENT_MOUSE_IN, _ecore_evas_gix_event_mouse_in, NULL);
   ecore_evas_event_handlers[9] = ecore_event_handler_add(ECORE_GIX_EVENT_MOUSE_OUT, _ecore_evas_gix_event_mouse_out, NULL);


   //ecore_evas_event_handlers[8] = ecore_event_handler_add(ECORE_GIX_EVENT_WINDOW_PROPERTY, _ecore_evas_gix_event_window_property, NULL);
   //ecore_evas_event_handlers[9] = ecore_event_handler_add(ECORE_GIX_EVENT_CLIENT_MESSAGE, _ecore_evas_gix_event_window_client_msg, NULL);

   ecore_event_evas_init(); 

   return _ecore_evas_init_count;
}


int
_ecore_evas_gix_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        int i;

        for (i = 0; i < ECORE_EVAS_EVENT_COUNT; i++)
          ecore_event_handler_del(ecore_evas_event_handlers[i]);

		ecore_event_evas_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

/* engine functions */
/********************/

static void
_ecore_evas_gix_free(Ecore_Evas *ee)
{
   Ecore_Gix_Window  window;

   window = ee->prop.window;

   ecore_gix_window_free(window);
   ecore_event_window_unregister(ee->prop.window);
   _ecore_evas_gix_shutdown();
   ecore_gix_shutdown();
}

static void
_ecore_evas_gix_move(Ecore_Evas *ee, int x, int y)
{
  if ((x != ee->x) || (y != ee->y)){
    ecore_gix_window_move(ee->prop.window, x, y);
    if (ee->func.fn_move) ee->func.fn_move(ee);
  }
}

static void
_ecore_evas_gix_resize(Ecore_Evas *ee, int w, int h)
{
   ee->req.w = w;
   ee->req.h = h;

   if ((w == ee->w) && (h == ee->h)) return;

   ecore_gix_window_resize(ee->prop.window, w, h);
   ee->w = w;
   ee->h = h;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        evas_output_size_set(ee->evas, ee->h, ee->w);
        evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
     }
   else
     {
        evas_output_size_set(ee->evas, ee->w, ee->h);
        evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
     }

  if (ee->prop.avoid_damage)
	{
	 int pdam;

	 pdam = ecore_evas_avoid_damage_get(ee);
	 ecore_evas_avoid_damage_set(ee, 0);
	 ecore_evas_avoid_damage_set(ee, pdam);
	}
	/*         if (ee->shaped) */
	/*           _ecore_evas_gix_region_border_resize(ee); */
	
	if (ee->func.fn_resize) ee->func.fn_resize(ee);

}

static void
_ecore_evas_gix_focus_set(Ecore_Evas *ee, int on __UNUSED__)
{
   ecore_gix_window_focus(ee->prop.window);
}

static void
_ecore_evas_gix_hide(Ecore_Evas *ee)
{
   ee->should_be_visible = 0;
   ecore_gix_window_hide(ee->prop.window);
}

static void
_ecore_evas_gix_show(Ecore_Evas *ee)
{
  ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_gix_render(ee);

   ecore_gix_window_show(ee->prop.window);
   if (ee->prop.fullscreen)
     ecore_gix_window_focus(ee->prop.window);
}

static void
_ecore_evas_gix_shaped_set(Ecore_Evas *ee, int shaped)
{
   Evas_Engine_Info_Software_Gix *einfo;
   if (((ee->shaped) && (shaped)) || ((!ee->shaped) && (!shaped)))
     return;
   ee->shaped = shaped;

#if 0
	einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);
	ee->shaped = shaped;
	if (einfo)
	  {
		 //ee->engine.win32.state.region = ee->shaped;
		 einfo->info.destination_alpha = shaped;
		 if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
		   {
			  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
		   }
		 if (ee->shaped)
		   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
	  }
#else
	DBG("_ecore_evas_gix_shaped_set() calling ..\n");
	
#endif

}


static void
_ecore_evas_x_protocols_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
}


static void
_ecore_evas_x_hints_update(Ecore_Evas *ee)
{
	#if 0
   Ecore_X_Window_State_Hint initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;

   if (ee->prop.iconified)
     initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;

   ecore_x_icccm_hints_set
     (ee->prop.window,
         !ee->prop.focus_skip /* accepts_focus */,
         initial_state /* initial_state */,
         0 /* icon_pixmap */,
         0 /* icon_mask */,
         0 /* icon_window */,
         ee->prop.group_ee_win /* window_group */,
         ee->prop.urgent /* is_urgent */);
   #endif
}

static void
_ecore_evas_gix_alpha_set(Ecore_Evas *ee, int alpha)
{
	gi_window_info_t att;
	Evas_Engine_Info_Software_Gix *einfo;

   if ((ee->alpha == alpha)) return;

   if (!strcmp(ee->driver, "gix")){
		einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);
		if (!einfo) return;

		//if (!ecore_gix_composite_query()) return;

		ee->shaped = 0;
		ee->alpha = alpha;

		if (ee->visible) ecore_gix_window_show(ee->prop.window);
		if (ee->prop.focused) ecore_gix_window_focus(ee->prop.window);
		if (ee->prop.title)
		  {
			 ecore_gix_window_title_set(ee->prop.window, ee->prop.title);
		  }

		  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
		  {
			 ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
		  }
		evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
		
		//_ecore_evas_gix_group_leader_update(ee);
		//ecore_gix_window_defaults_set(ee->prop.window);
		//_ecore_evas_gix_protocols_set(ee);
		//_ecore_evas_gix_sync_set(ee);
		DBG("_ecore_evas_gix_alpha_set(%ld) calling alpha=%d..\n", ee->prop.window, alpha);
		//(void)ee;
		//(void)alpha;
   }
   else if (!strcmp(ee->driver, "opengl_gix")){
#ifdef BUILD_ECORE_EVAS_OPENGL_GIX
        Evas_Engine_Info_GL_GIX *einfo;

        einfo = (Evas_Engine_Info_GL_GIX *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        //if (!ecore_x_composite_query()) return;

        //ee->shaped = 0;
        ee->alpha = alpha;
        ecore_gix_window_free(ee->prop.window);
        ecore_event_window_unregister(ee->prop.window);
        ee->prop.window = 0;

        einfo->info.destination_alpha = alpha;

		unsigned int windowed = 1;

		ee->prop.window = ecore_evas_gl_gix_new(NULL,windowed,ee->req.x, ee->req.y,
								   ee->req.w, ee->req.h);

        
	  /*ee->prop.window =  _ecore_evas_x_gl_window_new(ee, ee->engine.x.win_root,
								   ee->req.x, ee->req.y,
								   ee->req.w, ee->req.h,
								   ee->prop.override, ee->alpha, NULL);
								   */

        if (!ee->prop.window) return;

        //ecore_x_window_attributes_get(ee->prop.window, &att);
        //einfo->info.visual = att.visual;
        //einfo->info.colormap = att.colormap;
        //einfo->info.depth = att.depth;

        einfo->info.drawable = ee->prop.window;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
//        ecore_x_window_shape_mask_set(ee->prop.window, 0);
        //ecore_x_input_multi_select(ee->prop.window);
        ecore_event_window_register(ee->prop.window, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
        if (ee->prop.borderless)
          ecore_gix_window_borderless_set(ee->prop.window, ee->prop.borderless);
        if (ee->visible) ecore_gix_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_gix_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_gix_window_title_set(ee->prop.window, ee->prop.title);
          }
        //if (ee->prop.name)
          //ecore_x_icccm_name_class_set(ee->prop.window,
            //                           ee->prop.name, ee->prop.clas);
        //_ecore_evas_x_hints_update(ee);
        //_ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        //_ecore_evas_x_protocols_set(ee);
        //_ecore_evas_x_sync_set(ee);
        //_ecore_evas_x_size_pos_hints_update(ee);
#endif /* BUILD_ECORE_EVAS_OPENGL_GIX */
   }
   else{
   }
}

#if 0
static void
_ecore_evas_object_cursor_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee)
     ee->prop.cursor.object = NULL;
}
#endif

static void
_ecore_evas_gix_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
	if (obj != _ecore_evas_default_cursor_image_get(ee))
     ecore_gix_window_cursor_show(ee->prop.window, 0);
   /*int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (!obj)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_gix_window_cursor_show(ee->prop.window, 1);
        return;

     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_gix_window_cursor_show(ee->prop.window, 0);

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,x - ee->prop.cursor.hot.x,y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
   */
}

static void
_ecore_evas_gix_fullscreen_set(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Software_Gix *einfo;
   int w;
   int h;
   int resized = 0;

   if (((ee->prop.fullscreen) && (on)) || ((!ee->prop.fullscreen) && (!on)))
     return;

   if (on)
     ecore_gix_window_fullscreen_set(ee->prop.window, 1);
   else
     ecore_gix_window_fullscreen_set(ee->prop.window, 0);
   /* set the new size of the evas */
   ecore_gix_window_size_get(ee->prop.window, &w, &h);
   if( (ee->w != w) || (ee->h != h))
     {
        resized = 1;
        ee->w = w;
        ee->h = h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;

        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
     }
   einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	   //fixme dpp

		//einfo->info.depth    = einfo->func.best_depth_get(einfo);
		//einfo->info.drawable = ee->prop.window.id;
		DBG("%s got line%d\n",__FUNCTION__,__LINE__);


        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
   ee->prop.fullscreen = on;
   if (resized)
     {
        if(ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void *
_ecore_evas_gix_window_get(const Ecore_Evas *ee)
{
   return (void*)ee->prop.window;
}


static void
_ecore_evas_gix_raise(Ecore_Evas *ee)
{
   DBG("ecore evas gix raise");

   if (!ee->prop.fullscreen)
     ecore_gix_window_raise((Ecore_Gix_Window )ee->prop.window);
   else
     ecore_gix_window_raise((Ecore_Gix_Window )ee->prop.window);
}

static void
_ecore_evas_gix_lower(Ecore_Evas *ee)
{
   DBG("ecore evas gix lower");

   if (!ee->prop.fullscreen)
     ecore_gix_window_lower((Ecore_Gix_Window )ee->prop.window);
   else
     ecore_gix_window_lower((Ecore_Gix_Window )ee->prop.window);
}


static void
_ecore_evas_gix_iconified_set(Ecore_Evas *ee, int on)
{
/*    if (((ee->prop.borderless) && (on)) || */
/*        ((!ee->prop.borderless) && (!on))) return; */
   ee->prop.iconified = on;
   ecore_gix_window_iconified_set((Ecore_Gix_Window )ee->prop.window,
                                    ee->prop.iconified);
}



static void
_ecore_evas_gix_borderless_set(Ecore_Evas *ee, int on)
{
   if (((ee->prop.borderless) && (on)) ||
       ((!ee->prop.borderless) && (!on))) return;
   ee->prop.borderless = on;
   ecore_gix_window_borderless_set((Ecore_Gix_Window )ee->prop.window,
                                     ee->prop.borderless);


}

static void
_ecore_evas_gix_activate(Ecore_Evas *ee __UNUSED__)
{
   DBG("ecore evas activate");

   //ecore_gix_window_focus_set((Ecore_Gix_Window )ee->prop.window);
}


static void
_ecore_evas_gix_callback_delete_request_set(Ecore_Evas *ee,
                                              void (*func) (Ecore_Evas *ee))
{
   ee->func.fn_delete_request = func;
}


static void
_ecore_evas_gix_title_set(Ecore_Evas *ee, const char *t)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (t) ee->prop.title = strdup(t);
   
   //ecore_gix_icccm_title_set(ee->prop.window, ee->prop.title);
   ecore_gix_window_title_set(ee->prop.window, ee->prop.title);
}

static void
_ecore_evas_gix_rotation_set_internal(Ecore_Evas *ee, int rotation, int resize,
                                    Evas_Engine_Info *einfo)
{
   int rot_dif;
   Ecore_Evas_Cursor *cursor;
   const Evas_Device *pointer;


   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   pointer = evas_default_device_get(ee->evas, EVAS_DEVICE_CLASS_MOUSE);
   pointer = evas_device_parent_get(pointer);

   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);

   if (rot_dif != 180)
     {
        int minw, minh, maxw, maxh, basew, baseh, stepw, steph;

        if (!evas_engine_info_set(ee->evas, einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }

        if (!resize)
          {
             if (!ee->prop.fullscreen)
               {
                  ecore_gix_window_resize(ee->prop.window, ee->req.h, ee->req.w);
                  ee->expecting_resize.w = ee->h;
                  ee->expecting_resize.h = ee->w;
               }
             else
               {
                  int w, h;
                  
                  ecore_gix_window_size_get(ee->prop.window, &w, &h);
                  ecore_gix_window_resize(ee->prop.window, h, w);
                  if ((rotation == 0) || (rotation == 180))
                    {
                       evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
                       evas_output_viewport_set(ee->evas, 0, 0, ee->req.w, ee->req.h);
                    }
                  else
                    {
                       evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                       evas_output_viewport_set(ee->evas, 0, 0, ee->req.h, ee->req.w);
                    }
                  if (ee->func.fn_resize) ee->func.fn_resize(ee);
               }
             if ((ee->rotation == 90) || (ee->rotation == 270))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.h, ee->req.w);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
          }
        else
          {
             int w, h;
             
             ecore_gix_window_size_get(ee->prop.window, &w, &h);
             if ((rotation == 0) || (rotation == 180))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
             if ((ee->rotation == 90) || (ee->rotation == 270))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        ecore_evas_size_min_get(ee, &minw, &minh);
        ecore_evas_size_max_get(ee, &maxw, &maxh);
        ecore_evas_size_base_get(ee, &basew, &baseh);
        ecore_evas_size_step_get(ee, &stepw, &steph);
        ee->rotation = rotation;
        ecore_evas_size_min_set(ee, minh, minw);
        ecore_evas_size_max_set(ee, maxh, maxw);
        ecore_evas_size_base_set(ee, baseh, basew);
        ecore_evas_size_step_set(ee, steph, stepw);
        _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                       ecore_gix_current_time_get());
     }
   else
     {
        if (!evas_engine_info_set(ee->evas, einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        ee->rotation = rotation;
        _ecore_evas_mouse_move_process(ee,cursor->pos_x, cursor->pos_y,
                                       ecore_gix_current_time_get());
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
        
        if ((ee->rotation == 90) || (ee->rotation == 270))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
}


static void
_ecore_evas_gix_move_resize(Ecore_Evas *ee, int x, int y, int width, int height)
{
   //INF("ecore evas resize (%dx%d %dx%d)", x, y, width, height);

   if ((ee->w != width) || (ee->h != height) || (x != ee->x) || (y != ee->y))
     {
        int change_size = 0;
        int change_pos = 0;

        if ((ee->w != width) || (ee->h != height)) change_size = 1;
        if ((x != ee->x) || (y != ee->y)) change_pos = 1;

        ee->x = x;
        ee->y = y;
        ee->w = width;
        ee->h = height;
        ecore_gix_window_move(ee->prop.window, x, y);
		ecore_gix_window_resize(ee->prop.window, width, height);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
/*         if ((ee->shaped) || (ee->alpha)) */
/*           _ecore_evas_gix_region_border_resize(ee); */
        if (change_pos)
          {
             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
        if (change_size)
          {
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
          }
     }
}

static void
_ecore_evas_gix_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
	Evas_Engine_Info_Software_Gix *einfo;
   DBG("ecore evas rotation: %s", rotation ? "yes" : "no");

   if (ee->rotation == rotation) return;

	einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);
	if (!einfo) return;
	einfo->info.rotation = rotation;
	if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
	  {
		 ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
	  }
	_ecore_evas_gix_rotation_set_internal
	  (ee, rotation, resize, (Evas_Engine_Info *)einfo);

}

/*
 * @param ee
 * @param layer If < 3, @a ee will be put below all other windows.
 *              If > 5, @a ee will be "always-on-top"
 *              If = 4, @a ee will be put in the default layer.
 *              Acceptable values range from 1 to 255 (0 reserved for
 *              desktop windows)
 */
static void
_ecore_evas_x_layer_update(Ecore_Evas *ee)
{
   if (ee->should_be_visible)
     {
     }
   else
     {
     }

	 if (ee->prop.layer == 4){
		 gi_set_window_stack(ee->prop.window,G_WINDOW_LAYER_NORMAL);
	 }
	 else if (ee->prop.layer < 3){
		 gi_set_window_stack(ee->prop.window,G_WINDOW_LAYER_BELOW);
	 }
	 else if (ee->prop.layer > 5){
		 gi_set_window_stack(ee->prop.window,G_WINDOW_LAYER_ONTOP);
	 }
   /* FIXME: Set gnome layer */
}

static void
_ecore_evas_gix_layer_set(Ecore_Evas *ee, int layer)
{
   if (ee->prop.layer == layer) return;

   /* FIXME: Should this logic be here? */
   if (layer < 1)
     layer = 1;
   else if (layer > 255)
     layer = 255;

   ee->prop.layer = layer;
   _ecore_evas_x_layer_update(ee);
}
#endif

#ifdef BUILD_ECORE_EVAS_GIX
static Ecore_Evas_Engine_Func _ecore_gix_engine_func =
{
   _ecore_evas_gix_free,        /* free an ecore_evas */
     NULL,                                /* cb resize */
     NULL,                                /* cb move */
     NULL,                                /* cb show */
     NULL,                                /* cb hide */
     _ecore_evas_gix_callback_delete_request_set, /* cb delete request */
     NULL,                                /* cb destroy */
     NULL,                                /* cb focus in */
     NULL,                                /* cb focus out */
     NULL,                              /* cb sticky */
     NULL,                              /* cb unsticky */
     NULL,                                /* cb mouse in */
     NULL,                                /* cb mouse out */
     NULL,                                /* cb pre render */
     NULL,                                /* cb post render */
     _ecore_evas_gix_move,                /* move */
     NULL,                              /* managed move */
     _ecore_evas_gix_resize,        /* resize */
     _ecore_evas_gix_move_resize,                                /* move resize */
     _ecore_evas_gix_rotation_set,/* rotation */
     _ecore_evas_gix_shaped_set,   /* shaped */
     _ecore_evas_gix_show,			/* show */
     _ecore_evas_gix_hide,			/* hide */
     _ecore_evas_gix_raise,			/* raise */
     _ecore_evas_gix_lower,			/* lower */
     _ecore_evas_gix_activate,		/* activate */
     _ecore_evas_gix_title_set,		/* title set */
     NULL,                                /* name class set */
     NULL,                                /* size min */
     NULL,                                /* size max */
     NULL,                                /* size base */
     NULL,                                /* size step */
     _ecore_evas_gix_object_cursor_set, /* set cursor to an evas object */
     _ecore_evas_gix_layer_set,                                /* layer set */
     _ecore_evas_gix_focus_set,        /* focus */
     _ecore_evas_gix_iconified_set,                                /* iconified */
     _ecore_evas_gix_borderless_set,                                /* borderless */
     NULL,                                /* override */
     NULL,                                /* maximized */
     _ecore_evas_gix_fullscreen_set,/* fullscreen */
     NULL,                                /* avoid damage */
     NULL,                                /* withdrawn */
     NULL,                                /* sticky */
     NULL,                              /* ignore events */
#if 1
     _ecore_evas_gix_alpha_set,         /* alpha */
#else
     NULL,         /* alpha */
#endif
     NULL, //transparent
     NULL, // profiles_set

     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,

     NULL, // render
     NULL, // screen_geometry_get
     NULL  // screen_dpi_get

};

#ifdef BUILD_ECORE_EVAS_OPENGL_GIX
EAPI void
ecore_evas_gl_gix_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e))
{
   Evas_Engine_Info_GL_GIX *einfo;

   if (!(!strcmp(ee->driver, "opengl_gix"))) return;

   einfo = (Evas_Engine_Info_GL_GIX *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->callback.pre_swap = pre_cb;
        einfo->callback.post_swap = post_cb;
        einfo->callback.data = data;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
}
#else
EAPI void
ecore_evas_gl_gix_pre_post_swap_callback_set(const Ecore_Evas *ee __UNUSED__, void *data __UNUSED__, void (*pre_cb) (void *data, Evas *e) __UNUSED__, void (*post_cb) (void *data, Evas *e) __UNUSED__)
{
   return;
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_GIX */



EAPI Ecore_Evas *
ecore_evas_gix_new_internal(int rmethod, const char *disp_name, int windowed, int x, int y, int w, int h,long gformat)
{
   Evas_Engine_Info_Software_Gix *einfo;
   Ecore_Evas *ee;
   Ecore_Gix_Window *window;
   long flags = 0;

   if (!ecore_gix_init(disp_name)) {
	   ERR( "ecore_evas_gix_new: failed for %s\n",disp_name);
	   return NULL;
   }
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);
   _ecore_evas_gix_init();
   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_gix_engine_func;

   ee->driver = "gix";   
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->rotation = 0;
   ee->visible = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.layer = 1;
   //ee->prop.layer = 4;
   ee->prop.fullscreen = 0;
   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->prop.window = 0;
   //ee->engine.gix.parent = NULL;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);

   if(!windowed)
	   flags |= GI_FLAGS_NORESIZE|GI_FLAGS_NON_FRAME;

   window = (Ecore_Gix_Window *)ecore_gix_window_new(
	   GI_DESKTOP_WINDOW_ID, flags,x,y,w,h);
   ee->prop.window = (unsigned)window;

   if (einfo)
     {
	   //long gformat = GI_RENDER_x8r8g8b8;

	   if (rmethod == evas_render_method_lookup("gl_gix"))
	   {
		    //gformat = gi_screen_format();
#ifdef BUILD_ECORE_EVAS_OPENGL_GIX
    Evas_Engine_Info_GL_GIX *elinfo;
	elinfo = (Evas_Engine_Info_GL_GIX *)einfo;

		    ee->driver = "gl_gix";
		    elinfo->info.drawable = (unsigned)window;
			elinfo->info.mask = 0;
			elinfo->info.gformat = gformat;
			elinfo->info.rotation = 0;
			elinfo->info.rotation = 0;

			/**/
			elinfo->render_mode = 0;
			elinfo->vsync = 0;
			elinfo->indirect = 0;

			elinfo->callback.data = 0;
			elinfo->callback.pre_swap = 0;
			elinfo->callback.post_swap = 0;

			if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)elinfo))
			  {
				 ERR("evas_engine_info_set() for engine '%s' failed.",
					 ee->driver);
				 ecore_evas_free(ee);
				 return NULL;
			  }
#endif
	   }
	   else{
		   
		   //gformat = gi_screen_format();
		   
			einfo->info.drawable = (unsigned)window;
			einfo->info.mask = 0;
			einfo->info.gformat = gformat;
			einfo->info.rotation = 0;
			if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
			  {
				 ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
				 ecore_evas_free(ee);
				 return NULL;
			  }

	   }
     }
	 else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   
   ee->engine.func->fn_render = _ecore_evas_gix_render;

   _ecore_evas_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);


   return ee;
}

static void
_ecore_evas_x_flush_post(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
}


static int
_ecore_evas_x_render(Ecore_Evas *ee)
{
   int rend = 0;

   if (ee->in_async_render)
     {
        EDBG("ee=%p is rendering asynchronously, skip.", ee);
        return 0;
     }

   rend = ecore_evas_render_prepare(ee);

   if (!ee->can_async_render)
     {
        Eina_List *updates = evas_render_updates(ee->evas);
        rend = _render_updates_process(ee, updates);
        evas_render_updates_free(updates);
     }
   else if (evas_render_async(ee->evas))
     {
        EDBG("ee=%p started asynchronous render.", ee);
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }
   else if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

static void
_ecore_evas_x_free(Ecore_Evas *ee)
{
}

static void
_ecore_evas_x_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_delete_request = func;
   _ecore_evas_x_protocols_set(ee);
}


static Ecore_Evas_Engine_Func _ecore_x_engine_func =
{
   _ecore_evas_x_free,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_x_callback_delete_request_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
#if 0
   _ecore_evas_x_move,
   _ecore_evas_x_managed_move,
   _ecore_evas_x_resize,
   _ecore_evas_x_move_resize,
   _ecore_evas_x_rotation_set,
   _ecore_evas_x_shaped_set,
   _ecore_evas_x_show,
   _ecore_evas_x_hide,
   _ecore_evas_x_raise,
   _ecore_evas_x_lower,
   _ecore_evas_x_activate,
   _ecore_evas_x_title_set,
   _ecore_evas_x_name_class_set,
   _ecore_evas_x_size_min_set,
   _ecore_evas_x_size_max_set,
   _ecore_evas_x_size_base_set,
   _ecore_evas_x_size_step_set,
   _ecore_evas_x_object_cursor_set,
   _ecore_evas_x_object_cursor_unset,
   _ecore_evas_x_layer_set,
   _ecore_evas_x_focus_set,
   _ecore_evas_x_iconified_set,
   _ecore_evas_x_borderless_set,
   _ecore_evas_x_override_set,
   _ecore_evas_x_maximized_set,
   _ecore_evas_x_fullscreen_set,
   _ecore_evas_x_avoid_damage_set,
   _ecore_evas_x_withdrawn_set,
   _ecore_evas_x_sticky_set,
   _ecore_evas_x_ignore_events_set,
   _ecore_evas_x_alpha_set,
   _ecore_evas_x_transparent_set,
   _ecore_evas_x_profiles_set,
   _ecore_evas_x_profile_set,

   _ecore_evas_x_window_group_set,
   _ecore_evas_x_aspect_set,
   _ecore_evas_x_urgent_set,
   _ecore_evas_x_modal_set,
   _ecore_evas_x_demand_attention_set,
   _ecore_evas_x_focus_skip_set,

   NULL, // render
   _ecore_evas_x_screen_geometry_get,
   _ecore_evas_x_screen_dpi_get,
   NULL,
   NULL, //fn_msg_send

   _ecore_evas_x_pointer_xy_get,
   _ecore_evas_x_pointer_warp,

   _ecore_evas_x_wm_rot_preferred_rotation_set,
   _ecore_evas_x_wm_rot_available_rotations_set,
   _ecore_evas_x_wm_rot_manual_rotation_done_set,
   _ecore_evas_x_wm_rot_manual_rotation_done,

   _ecore_evas_x_aux_hints_set,

   NULL, // fn_animator_register
   NULL, // fn_animator_unregister

   NULL, // fn_evas_changed
   NULL, //fn_focus_device_set
   NULL, //fn_callback_focus_device_in_set
   NULL, //fn_callback_focus_device_out_set
   NULL, //fn_callback_device_mouse_in_set
   NULL, //fn_callback_device_mouse_out_set
   NULL, //fn_pointer_device_xy_get
   NULL, //fn_prepare
   NULL, //fn_last_tick_get
   _ecore_evas_x_selection_claim, //fn_selection_claim
   _ecore_evas_x_selection_has_owner, //fn_selection_has_owner
   _ecore_evas_x_selection_request, //fn_selection_request
   _ecore_evas_x_dnd_start, //fn_dnd_start
   _ecore_evas_x_dnd_stop, //fn_dnd_stop
#endif
};


static void
_ecore_evas_x_render_updates(void *data, Evas *e EINA_UNUSED, void *event_info)
{

   Evas_Event_Render_Post *ev = event_info;
   Ecore_Evas *ee = data;

   if (!ev) return;

   EDBG("ee=%p finished asynchronous render.", ee);

   ee->in_async_render = EINA_FALSE;
#if 0
   _render_updates_process(ee, ev->updated_area);

   if (ee->delayed.resize_shape)
     {
        _resize_shape_do(ee);
        ee->delayed.resize_shape = EINA_FALSE;
     }
   if (ee->delayed.shaped_changed)
     {
        _shaped_do(ee, ee->delayed.shaped);
        ee->delayed.shaped_changed = EINA_FALSE;
     }
   if (ee->delayed.alpha_changed)
     {
        _alpha_do(ee, ee->delayed.alpha);
        ee->delayed.alpha_changed = EINA_FALSE;
     }
   if (ee->delayed.transparent_changed)
     {
        _transparent_do(ee, ee->delayed.transparent);
        ee->delayed.transparent_changed = EINA_FALSE;
     }
   if (ee->delayed.avoid_damage != ee->prop.avoid_damage)
     _avoid_damage_do(ee, ee->delayed.avoid_damage);
   if (ee->delayed.rotation_changed)
     {
        _rotation_do(ee, ee->delayed.rotation, ee->delayed.rotation_resize);
        ee->delayed.rotation_changed = EINA_FALSE;
     }
#endif
}


static void
_ecore_evas_x_group_leader_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   //leader_ref++;
   //if (leader_ref == 1)
     {
	 }
}

EAPI Ecore_Evas *
ecore_evas_software_gix_new_internal( Ecore_Gix_Window parent,
				     int x, int y, int w, int h)
{
   Evas_Engine_Info_Software_Gix *einfo;
   //Ecore_Evas_Interface_X11 *iface;
   //Ecore_Evas_Interface_Software_X11 *siface;
   Ecore_Evas_Engine_Data_X11 *edata;
   Ecore_Evas *ee;
   int argb = 0, rmethod;
   static int redraw_debug = -1;
   char *id = NULL;

   ERR("%s() start!~",__func__);

   rmethod = evas_render_method_lookup("software_gix");
   if (!rmethod) {
	   ERR("Can not found software_gix module.");
	   return NULL;
   }
   if (!ecore_gix_init(NULL)) {
	   ERR("%s() init failed.",__func__);
	   return NULL;
   }
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)  {
	   ERR("%s() No memory.",__func__);
	   return NULL;
   }
   edata = calloc(1, sizeof(Ecore_Evas_Engine_Data_X11));
   if (!edata)
     {
	   ERR("%s() fail at line %d.",__func__,__LINE__);
	free(ee);
	return NULL;
     }

   ee->engine.data = edata;
   //iface = _ecore_evas_x_interface_x11_new();
   //siface = _ecore_evas_x_interface_software_x11_new();

   //ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);
   //ee->engine.ifaces = eina_list_append(ee->engine.ifaces, siface);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_gix_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "software_gix";
   //if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = 0;
   ee->prop.withdrawn = EINA_TRUE;
   edata->state.sticky = 0;

   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   /* init evas here */
   if (!ecore_evas_evas_new(ee, w, h))
     {
        ERR("Can not create a Canvas.");
        ecore_evas_free(ee);
        return NULL;
     }

   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST,
                           _ecore_evas_x_flush_post, ee);
   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
			     _ecore_evas_x_render_updates, ee);
   evas_output_method_set(ee->evas, rmethod);

   edata->win_root = parent;
   edata->screen_num = 0;

   if (parent != 0)
     {
        edata->screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
	   #if 0
        if (ecore_x_window_argb_get(parent))
          {
             ee->prop.window = ecore_x_window_argb_new(parent, x, y, w, h);
             argb = 1;
          }
        else
		#endif
          ee->prop.window = ecore_gix_window_new(parent,0, x, y, w, h);
     }
   else
     ee->prop.window = ecore_gix_window_new(parent,0, x, y, w, h);
   //ecore_x_vsync_animator_tick_source_set(ee->prop.window);
   if ((id = getenv("DESKTOP_STARTUP_ID")))
     {
        //ecore_x_netwm_startup_id_set(ee->prop.window, id);
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
//        putenv((char*)"DESKTOP_STARTUP_ID=");
     }
   einfo = (Evas_Engine_Info_Software_Gix *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
       /* Ecore_X_Screen *screen;

        // FIXME: this is inefficient as its 1 or more round trips 
        screen = ecore_x_default_screen_get();
        if (ecore_x_screen_count_get() > 1)
          {
             Ecore_Gix_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  Ecore_Gix_Window root;

                  root = ecore_x_window_root_get(parent);
                  for (i = 0; i < num; i++)
                    {
                       if (root == roots[i])
                         {
                            screen = ecore_x_screen_get(i);
                            break;
                         }
                    }
                  free(roots);
               }
          }
		  */

        //einfo->info.destination_alpha = argb;

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }

       // einfo->info.backend = EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB;
        //einfo->info.connection = ecore_x_display_get();
       // einfo->info.screen = NULL;
        einfo->info.drawable = ee->prop.window;

        if (argb)
          {
             //Ecore_X_Window_Attributes at;

             //ecore_x_window_attributes_get(ee->prop.window, &at);
             //einfo->info.visual = at.visual;
             //einfo->info.colormap = at.colormap;
            // einfo->info.depth = at.depth;
             einfo->info.destination_alpha = 1;
          }
        else
          {
             //einfo->info.visual =
             //  ecore_x_default_visual_get(einfo->info.connection, screen);
             //einfo->info.colormap =
            //   ecore_x_default_colormap_get(einfo->info.connection, screen);
            // einfo->info.depth =
            //   ecore_x_default_depth_get(einfo->info.connection, screen);
             einfo->info.destination_alpha = 0;
          }

        einfo->info.rotation = 0;
        einfo->info.debug = redraw_debug;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }

   _ecore_evas_x_hints_update(ee);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
   //_ecore_evas_x_window_profile_protocol_set(ee);
  // _ecore_evas_x_wm_rotation_protocol_set(ee);
   //_ecore_evas_x_aux_hints_supported_update(ee);
  // _ecore_evas_x_aux_hints_update(ee);
   //_ecore_evas_x_selection_window_init(ee);

   ee->engine.func->fn_render = _ecore_evas_x_render;
   ee->draw_block = EINA_TRUE;
   //if (!wm_exists) edata->configured = 1;

  // ecore_x_input_multi_select(ee->prop.window);
   ecore_evas_done(ee, EINA_FALSE);

    ERR("%s() init OK.",__func__);

   return ee;
}

EAPI Ecore_Evas *
ecore_evas_gix_new(const char *disp_name, int windowed, int x, int y, int w, int h)
{
   int rmethod;
   long fmt = gi_screen_format();

   rmethod = evas_render_method_lookup("gix");
   if (!rmethod){
	   DBG("ecore_evas_gix_new: failed line %d\n",__LINE__);
	   return NULL;
   }
   if (!fmt)
   {
	   fmt = GI_RENDER_x8r8g8b8;
   }
   return ecore_evas_gix_new_internal(rmethod, disp_name,windowed,x,y,w,h,fmt);
}



EAPI Ecore_Evas *
ecore_evas_gix_rgba_new(const char *disp_name, int windowed, int x, int y, int w, int h)
{
   int rmethod;

   rmethod = evas_render_method_lookup("gix");
   if (!rmethod){
	   DBG("ecore_evas_gix_new: failed line %d\n",__LINE__);
	   return NULL;
   }
   return ecore_evas_gix_new_internal(rmethod, disp_name,windowed,
	   x,y,w,h,GI_RENDER_a8r8g8b8);
}


EAPI Ecore_Gix_Window  
ecore_evas_gix_window_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "gix")) && !(!strcmp(ee->driver, "gl_gix")))
	   return 0; //not tested
   return (Ecore_Gix_Window  ) _ecore_evas_gix_window_get(ee);
}


#else


EAPI Ecore_Evas *
ecore_evas_gix_new(const char *disp_name __UNUSED__, int windowed __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return NULL;
}

EAPI Ecore_Gix_Window  
ecore_evas_gix_window_get(const Ecore_Evas *ee __UNUSED__)
{
  return NULL;
}
#endif



#ifdef BUILD_ECORE_EVAS_OPENGL_GIX

EAPI Ecore_Evas*
ecore_evas_gl_gix_new(const char *disp_name, int windowed, int x, int y, int w, int h)
{
   Ecore_Evas          *ee;
   int                  rmethod;

   rmethod = evas_render_method_lookup("gl_gix");
   if (!rmethod) {
	   ERR("evas_render_method_lookup failed\n");
	   return NULL;
   }

   ee = ecore_evas_gix_new_internal(rmethod, disp_name,windowed,x,y,w,h,GI_RENDER_x8r8g8b8);
   //if (ee) ee->driver = "gl_gix";
   return ee;
}
#else
EAPI Ecore_Evas*
ecore_evas_gl_gix_new(const char* name __UNUSED__, int w __UNUSED__, int h __UNUSED__, int fullscreen __UNUSED__, int noframe __UNUSED__)
{
   ERR("OUTCH !");
   return NULL;
}

#endif

EAPI Ecore_Gix_Window
_ecore_evas_software_gix_window_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_gix"))) return 0;
   return (Ecore_Gix_Window) ecore_evas_window_get(ee);
}
