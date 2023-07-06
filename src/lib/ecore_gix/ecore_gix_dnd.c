#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>

#include "Ecore_Gix.h"
#include "ecore_gix_private.h"
#include "ecore_Gix_Keys.h"
#include "Ecore.h"
#include "ecore_private.h"
#include <gi/x11_port.h>



#define _ecore_x_disp ((void*)1)



EAPI int ECORE_GIX_EVENT_XDND_ENTER = 0;
EAPI int ECORE_GIX_EVENT_XDND_POSITION = 0;
EAPI int ECORE_GIX_EVENT_XDND_STATUS = 0;
EAPI int ECORE_GIX_EVENT_XDND_LEAVE = 0;
EAPI int ECORE_GIX_EVENT_XDND_DROP = 0;
EAPI int ECORE_GIX_EVENT_XDND_FINISHED = 0;

static Ecore_Gix_DND_Source *_source = NULL;
static Ecore_Gix_DND_Target *_target = NULL;
static int _ecore_x_dnd_init_count = 0;

typedef struct _Version_Cache_Item
{
   Ecore_Gix_Window win;
   int            ver;
} Version_Cache_Item;
static Version_Cache_Item *_version_cache = NULL;
static int _version_cache_num = 0, _version_cache_alloc = 0;
static void (*_posupdatecb)(void *,
                            Ecore_Gix_Xdnd_Position *);
static void *_posupdatedata;

void
_ecore_x_dnd_init(void)
{
   if (!_ecore_x_dnd_init_count)
     {
        _source = calloc(1, sizeof(Ecore_Gix_DND_Source));
        if (!_source) return;
        _source->version = ECORE_GIX_DND_VERSION;
        _source->win = None;
        _source->dest = None;
        _source->state = ECORE_GIX_DND_SOURCE_IDLE;
        _source->prev.window = 0;

        _target = calloc(1, sizeof(Ecore_Gix_DND_Target));
        if (!_target)
          {
             free(_source);
             _source = NULL;
             return;
          }
        _target->win = None;
        _target->source = None;
        _target->state = ECORE_GIX_DND_TARGET_IDLE;

        ECORE_GIX_EVENT_XDND_ENTER = ecore_event_type_new();
        ECORE_GIX_EVENT_XDND_POSITION = ecore_event_type_new();
        ECORE_GIX_EVENT_XDND_STATUS = ecore_event_type_new();
        ECORE_GIX_EVENT_XDND_LEAVE = ecore_event_type_new();
        ECORE_GIX_EVENT_XDND_DROP = ecore_event_type_new();
        ECORE_GIX_EVENT_XDND_FINISHED = ecore_event_type_new();
     }

   _ecore_x_dnd_init_count++;
}

void
_ecore_x_dnd_shutdown(void)
{
   _ecore_x_dnd_init_count--;
   if (_ecore_x_dnd_init_count > 0)
     return;

   if (_source)
     free(_source);

   _source = NULL;

   if (_target)
     free(_target);

   _target = NULL;

   _ecore_x_dnd_init_count = 0;
}

static Eina_Bool
_ecore_x_dnd_converter_copy(char *target __UNUSED__,
                            void *data,
                            int size,
                            void **data_ret,
                            int *size_ret,
                            Ecore_Gix_Atom *tprop __UNUSED__,
                            int *count __UNUSED__)
{
#if 0
   XTextProperty text_prop;
   char *mystr;
   XICCEncodingStyle style = XTextStyle;

   if (!data || !size)
     return EINA_FALSE;

   mystr = calloc(1, size + 1);
   if (!mystr)
     return EINA_FALSE;

   memcpy(mystr, data, size);

   if (XmbTextListToTextProperty( &mystr, 1, style,
                                 &text_prop) == Success)
     {
        int bufsize = strlen((char *)text_prop.value) + 1;
        *data_ret = malloc(bufsize);
        if (!*data_ret)
          {
             free(mystr);
             return EINA_FALSE;
          }
        memcpy(*data_ret, text_prop.value, bufsize);
        *size_ret = bufsize;
        XFree(text_prop.value);
        free(mystr);
        return EINA_TRUE;
     }
   else
     {
        free(mystr);
        return EINA_FALSE;
     }
#else
	*data_ret = malloc(size);
	memcpy(*data_ret, data, size);
	*size_ret = size;
	fprintf(stderr, "%s() not work now, Fixme!!!\n",__func__);
	return EINA_TRUE;
#endif
}

EAPI void
ecore_x_dnd_aware_set(Ecore_Gix_Window win,
                      Eina_Bool on)
{
   Ecore_Gix_Atom prop_data = ECORE_GIX_DND_VERSION;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (on)
     ecore_x_window_prop_property_set(win, ECORE_GIX_ATOM_XDND_AWARE,
                                      XA_ATOM, 32, &prop_data, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_GIX_ATOM_XDND_AWARE);
}

EAPI int
ecore_x_dnd_version_get(Ecore_Gix_Window win)
{
   unsigned char *prop_data;
   int num;
   Version_Cache_Item *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   // this looks hacky - and it is, but we need a way of caching info about
   // a window while dragging, because we literally query this every mouse
   // move and going to and from x multiple times per move is EXPENSIVE
   // and slows things down, puts lots of load on x etc.
   if (_source->state == ECORE_GIX_DND_SOURCE_DRAGGING)
     if (_version_cache)
       {
          int i;

          for (i = 0; i < _version_cache_num; i++)
            {
               if (_version_cache[i].win == win)
                 return _version_cache[i].ver;
            }
       }

   if (ecore_x_window_prop_property_get(win, ECORE_GIX_ATOM_XDND_AWARE,
                                        XA_ATOM, 32, &prop_data, &num))
     {
        int version = (int)*prop_data;
        free(prop_data);
        if (_source->state == ECORE_GIX_DND_SOURCE_DRAGGING)
          {
             _version_cache_num++;
             if (_version_cache_num > _version_cache_alloc)
               _version_cache_alloc += 16;

             t = realloc(_version_cache,
                         _version_cache_alloc *
                         sizeof(Version_Cache_Item));
             if (!t) return 0;
             _version_cache = t;
             _version_cache[_version_cache_num - 1].win = win;
             _version_cache[_version_cache_num - 1].ver = version;
          }

        return version;
     }

   if (_source->state == ECORE_GIX_DND_SOURCE_DRAGGING)
     {
        _version_cache_num++;
        if (_version_cache_num > _version_cache_alloc)
          _version_cache_alloc += 16;

        t = realloc(_version_cache, _version_cache_alloc *
                    sizeof(Version_Cache_Item));
        if (!t) return 0;
        _version_cache = t;
        _version_cache[_version_cache_num - 1].win = win;
        _version_cache[_version_cache_num - 1].ver = 0;
     }

   return 0;
}

EAPI Eina_Bool
ecore_x_dnd_type_isset(Ecore_Gix_Window win,
                       const char *type)
{
   int num, i, ret = EINA_FALSE;
   unsigned char *data;
   Ecore_Gix_Atom *atoms, atom;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!ecore_x_window_prop_property_get(win, ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, &data, &num))
     return ret;

   atom = ecore_x_atom_get(type);
   atoms = (Ecore_Gix_Atom *)data;

   for (i = 0; i < num; ++i)
     {
        if (atom == atoms[i])
          {
             ret = EINA_TRUE;
             break;
          }
     }

   XFree(data);
   return ret;
}

EAPI void
ecore_x_dnd_type_set(Ecore_Gix_Window win,
                     const char *type,
                     Eina_Bool on)
{
   Ecore_Gix_Atom atom;
   Ecore_Gix_Atom *oldset = NULL, *newset = NULL;
   int i, j = 0, num = 0;
   unsigned char *data = NULL;
   unsigned char *old_data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atom = ecore_x_atom_get(type);
   ecore_x_window_prop_property_get(win, ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_Gix_Atom *)old_data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (on)
     {
        if (ecore_x_dnd_type_isset(win, type))
          {
             XFree(old_data);
             return;
          }

        newset = calloc(num + 1, sizeof(Ecore_Gix_Atom));
        if (!newset)
          return;

        data = (unsigned char *)newset;

        for (i = 0; i < num; i++)
          newset[i + 1] = oldset[i];
        /* prepend the new type */
        newset[0] = atom;

        ecore_x_window_prop_property_set(win, ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num + 1);
     }
   else
     {
        if (!ecore_x_dnd_type_isset(win, type))
          {
             XFree(old_data);
             return;
          }

        newset = calloc(num - 1, sizeof(Ecore_Gix_Atom));
        if (!newset)
          {
             XFree(old_data);
             return;
          }

        data = (unsigned char *)newset;
        for (i = 0; i < num; i++)
          if (oldset[i] != atom)
            newset[j++] = oldset[i];

        ecore_x_window_prop_property_set(win, ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num - 1);
     }

   XFree(oldset);
   free(newset);
}

EAPI void
ecore_x_dnd_types_set(Ecore_Gix_Window win,
                      const char **types,
                      unsigned int num_types)
{
   Ecore_Gix_Atom *newset = NULL;
   unsigned int i;
   unsigned char *data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!num_types)
     ecore_x_window_prop_property_del(win, ECORE_GIX_ATOM_XDND_TYPE_LIST);
   else
     {
        newset = calloc(num_types, sizeof(Ecore_Gix_Atom));
        if (!newset)
          return;

        data = (unsigned char *)newset;
        for (i = 0; i < num_types; i++)
          {
             newset[i] = ecore_x_atom_get(types[i]);
             ecore_x_selection_converter_atom_add(newset[i],
                                                  _ecore_x_dnd_converter_copy);
          }
        ecore_x_window_prop_property_set(win, ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                         XA_ATOM, 32, data, num_types);
        free(newset);
     }
}

EAPI void
ecore_x_dnd_actions_set(Ecore_Gix_Window win,
                        Ecore_Gix_Atom *actions,
                        unsigned int num_actions)
{
   unsigned int i;
   unsigned char *data = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!num_actions)
     ecore_x_window_prop_property_del(win, ECORE_GIX_ATOM_XDND_ACTION_LIST);
   else
     {
        data = (unsigned char *)actions;
        for (i = 0; i < num_actions; i++)
          {
             ecore_x_selection_converter_atom_add(actions[i],
                                                  _ecore_x_dnd_converter_copy);
          }
        ecore_x_window_prop_property_set(win, ECORE_GIX_ATOM_XDND_ACTION_LIST,
                                         XA_ATOM, 32, data, num_actions);
     }
}

/**
 * The DND position update cb is called Ecore_X sends a DND position to a
 * client.
 *
 * It essentially mirrors some of the data sent in the position message.
 * Generally this cb should be set just before position update is called.
 * Please note well you need to look after your own data pointer if someone
 * trashes you position update cb set.
 *
 * It is considered good form to clear this when the dnd event finishes.
 *
 * @param cb Callback to updated each time ecore_x sends a position update.
 * @param data User data.
 */
EAPI void
ecore_x_dnd_callback_pos_update_set(
  void (*cb)(void *,
             Ecore_Gix_Xdnd_Position *data),
  const void *data)
{
   _posupdatecb = cb;
   _posupdatedata = (void *)data; /* Discard the const early */
}

Ecore_Gix_DND_Source *
_ecore_x_dnd_source_get(void)
{
   return _source;
}

Ecore_Gix_DND_Target *
_ecore_x_dnd_target_get(void)
{
   return _target;
}

EAPI Eina_Bool
ecore_x_dnd_begin(Ecore_Gix_Window source,
                  unsigned char *data,
                  int size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!ecore_x_dnd_version_get(source))
     return EINA_FALSE;

   /* Take ownership of XdndSelection */
   if (!ecore_x_selection_xdnd_set(source, data, size))
     return EINA_FALSE;

   if (_version_cache)
     {
        free(_version_cache);
        _version_cache = NULL;
        _version_cache_num = 0;
        _version_cache_alloc = 0;
     }

   ecore_x_window_shadow_tree_flush();

   _source->win = source;
   ecore_x_window_ignore_set(_source->win, 1);
   _source->state = ECORE_GIX_DND_SOURCE_DRAGGING;
   //_source->time = _ecore_x_event_last_time;
   _source->prev.window = 0;

   /* Default Accepted Action: move */
   _source->action = ECORE_GIX_ATOM_XDND_ACTION_MOVE;
   _source->accepted_action = None;
   _source->dest = None;

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_x_dnd_drop(void)
{
   XEvent xev;
   int status = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (_source->dest)
     {
        xev.type = GI_MSG_CLIENT_MSG;

        xev.body.client.client_format = 32;
        xev.ev_window = _source->dest;

        if (_source->will_accept)
          {
             xev.body.client.client_type = ECORE_GIX_ATOM_XDND_DROP;
             xev.params[0] = _source->win;
             xev.params[1] = 0;
             xev.params[2] = _source->time;
             gi_send_event( _source->dest, FALSE, 0, &xev);
             _source->state = ECORE_GIX_DND_SOURCE_DROPPED;
             status = EINA_TRUE;
          }
        else
          {
             xev.body.client.client_type = ECORE_GIX_ATOM_XDND_LEAVE;
             xev.params[0] = _source->win;
             xev.params[1] = 0;
             gi_send_event( _source->dest, FALSE, 0, &xev);
             _source->state = ECORE_GIX_DND_SOURCE_IDLE;
          }
     }
   else
     {
        /* Dropping on nothing */
        ecore_x_selection_xdnd_clear();
        _source->state = ECORE_GIX_DND_SOURCE_IDLE;
     }

   ecore_x_window_ignore_set(_source->win, 0);

   _source->prev.window = 0;

   return status;
}

EAPI void
ecore_x_dnd_send_status(Eina_Bool will_accept,
                        Eina_Bool suppress,
                        Ecore_Gix_Rectangle rectangle,
                        Ecore_Gix_Atom action)
{
   XEvent xev;

   if (_target->state == ECORE_GIX_DND_TARGET_IDLE)
     return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   memset(&xev, 0, sizeof(XEvent));

   _target->will_accept = will_accept;

   xev.type = GI_MSG_CLIENT_MSG;

   xev.body.client.client_type = ECORE_GIX_ATOM_XDND_STATUS;
   xev.body.client.client_format = 32;
   xev.ev_window = _target->source;

   xev.params[0] = _target->win;
   xev.params[1] = 0;
   if (will_accept)
     xev.params[1] |= 0x1UL;

   if (!suppress)
     xev.params[1] |= 0x2UL;

   /* Set rectangle information */
   xev.params[2] = rectangle.x;
   xev.params[2] <<= 16;
   xev.params[2] |= rectangle.y;
   xev.params[3] = rectangle.w;
   xev.params[3] <<= 16;
   xev.params[3] |= rectangle.h;

   if (will_accept)
     {
        xev.params[4] = action;
        _target->accepted_action = action;
     }
   else
     {
        xev.params[4] = None;
        _target->accepted_action = action;
     }

   gi_send_event( _target->source, FALSE, 0, &xev);
}

EAPI void
ecore_x_dnd_send_finished(void)
{
   XEvent xev;

   if (_target->state == ECORE_GIX_DND_TARGET_IDLE)
     return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xev.type = GI_MSG_CLIENT_MSG;

   xev.body.client.client_type = ECORE_GIX_ATOM_XDND_FINISHED;
   xev.body.client.client_format = 32;
   xev.ev_window = _target->source;

   xev.params[0] = _target->win;
   xev.params[1] = 0;
   xev.params[2] = 0;
   if (_target->will_accept)
     {
        xev.params[1] |= 0x1UL;
        xev.params[2] = _target->accepted_action;
     }

   gi_send_event( _target->source, FALSE, 0, &xev);

   _target->state = ECORE_GIX_DND_TARGET_IDLE;
}

EAPI void
ecore_x_dnd_source_action_set(Ecore_Gix_Atom action)
{
   _source->action = action;
   if (_source->prev.window)
     _ecore_x_dnd_drag(_source->prev.window, _source->prev.x, _source->prev.y);
}

EAPI Ecore_Gix_Atom
ecore_x_dnd_source_action_get(void)
{
   return _source->action;
}

void
_ecore_x_dnd_drag(Ecore_Gix_Window root,
                  int x,
                  int y)
{
   XEvent xev;
   Ecore_Gix_Window win;
   Ecore_Gix_Window *skip;
   Ecore_Gix_Xdnd_Position pos;
   int num;

   if (_source->state != ECORE_GIX_DND_SOURCE_DRAGGING)
     return;

   /* Preinitialize XEvent struct */
   memset(&xev, 0, sizeof(XEvent));
   xev.type = GI_MSG_CLIENT_MSG;

   xev.body.client.client_format = 32;

   /* Attempt to find a DND-capable window under the cursor */
   skip = ecore_x_window_ignore_list(&num);
// WARNING - this function is HEAVY. it goes to and from x a LOT walking the
// window tree - use the SHADOW version - makes a 1-off tree copy, then uses
// that instead.
//   win = ecore_x_window_at_xy_with_skip_get(x, y, skip, num);
   win = ecore_x_window_shadow_tree_at_xy_with_skip_get(root, x, y, skip, num);

// NOTE: This now uses the shadow version to find parent windows
//   while ((win) && !(ecore_x_dnd_version_get(win)))
//     win = ecore_x_window_parent_get(win);
   while ((win) && !(ecore_x_dnd_version_get(win)))
     win = ecore_x_window_shadow_parent_get(root, win);

   /* Send XdndLeave to current destination window if we have left it */
   if ((_source->dest) && (win != _source->dest))
     {
        xev.ev_window = _source->dest;
        xev.body.client.client_type = ECORE_GIX_ATOM_XDND_LEAVE;
        xev.params[0] = _source->win;
        xev.params[1] = 0;

        gi_send_event( _source->dest, FALSE, 0, &xev);
        _source->suppress = 0;
     }

   if (win)
     {
        int x1, x2, y1, y2;

        _source->version = MIN(ECORE_GIX_DND_VERSION,
                               ecore_x_dnd_version_get(win));
        if (win != _source->dest)
          {
             int i;
             unsigned char *data;
             Ecore_Gix_Atom *types;

             ecore_x_window_prop_property_get(_source->win,
                                              ECORE_GIX_ATOM_XDND_TYPE_LIST,
                                              XA_ATOM,
                                              32,
                                              &data,
                                              &num);
             types = (Ecore_Gix_Atom *)data;

             /* Entered new window, send XdndEnter */
             xev.ev_window = win;
             xev.body.client.client_type = ECORE_GIX_ATOM_XDND_ENTER;
             xev.params[0] = _source->win;
             xev.params[1] = 0;
             if (num > 3)
               xev.params[1] |= 0x1UL;
             else
               xev.params[1] &= 0xfffffffeUL;

             xev.params[1] |= ((unsigned long)_source->version) << 24;

             for (i = 2; i < 5; i++)
               xev.params[i] = 0;
             for (i = 0; i < MIN(num, 3); ++i)
               xev.params[i + 2] = types[i];
             XFree(data);
             gi_send_event( win, FALSE, 0, &xev);
             _source->await_status = 0;
             _source->will_accept = 0;
          }

        /* Determine if we're still in the rectangle from the last status */
        x1 = _source->rectangle.x;
        x2 = _source->rectangle.x + _source->rectangle.width;
        y1 = _source->rectangle.y;
        y2 = _source->rectangle.y + _source->rectangle.height;

        if ((!_source->await_status) ||
            (!_source->suppress) ||
            ((x < x1) || (x > x2) || (y < y1) || (y > y2)))
          {
             xev.ev_window = win;
             xev.body.client.client_type = ECORE_GIX_ATOM_XDND_POSITION;
             xev.params[0] = _source->win;
             xev.params[1] = 0; /* Reserved */
             xev.params[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
             xev.params[3] = _source->time; /* Version 1 */
             xev.params[4] = _source->action; /* Version 2, Needs to be pre-set */
             gi_send_event( win, FALSE, 0, &xev);

             _source->await_status = 1;
          }
     }

   if (_posupdatecb)
     {
        pos.position.x = x;
        pos.position.y = y;
        pos.win = win;
        pos.prev = _source->dest;
        _posupdatecb(_posupdatedata, &pos);
     }

   _source->prev.x = x;
   _source->prev.y = y;
   _source->prev.window = root;
   _source->dest = win;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
