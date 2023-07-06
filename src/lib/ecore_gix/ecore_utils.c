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
#include <gi/x11_port.h>




/*** atom *****************************************************/
EAPI gi_atom_id_t ecore_x_atom_get(const char *name)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XInternAtom(_ecore_x_disp, name, False);
}
/*** netwm *****************************************************/

EAPI void ecore_x_icccm_transient_for_set(Ecore_Gix_Window win,
                                Ecore_Gix_Window forwin)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_set_transient_for_hint( win, forwin);
}


#define _ATOM_SET_CARD32(win, atom, p_val, cnt)                               \
  XChangeProperty(_ecore_x_disp, win, atom, GA_CARDINAL, 32, PropModeReplace, \
                  (unsigned char *)p_val, cnt)

/*
 * Set CARD32 (array) property
 */
EAPI void ecore_x_window_prop_card32_set(Ecore_Gix_Window win,
                               gi_atom_id_t atom,
                               unsigned int *val,
                               unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   _ATOM_SET_CARD32(win, atom, val, num);
#else /* if SIZEOF_INT == SIZEOF_LONG */
   long *v2;
   unsigned int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   v2 = malloc(num * sizeof(long));
   if (!v2)
     return;

   for (i = 0; i < num; i++)
     v2[i] = val[i];
   _ATOM_SET_CARD32(win, atom, v2, num);
   free(v2);
#endif /* if SIZEOF_INT == SIZEOF_LONG */
}

/*
 * Get CARD32 (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_card32_get(Ecore_Gix_Window win,
                               gi_atom_id_t atom,
                               unsigned int *val,
                               unsigned int len)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   unsigned int i;
   int num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          GA_CARDINAL, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if (type_ret != GA_CARDINAL || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else
     {
        if (num_ret < len)
          len = num_ret;

        for (i = 0; i < len; i++)
          val[i] = ((unsigned long *)prop_ret)[i];
        num = len;
     }

   if (prop_ret)
     XFree(prop_ret);

   return num;
}

/*
 * Get CARD32 (array) property of any length
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_card32_list_get(Ecore_Gix_Window win,
                                    gi_atom_id_t atom,
                                    unsigned int **plst)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   unsigned int i, *val;
   int num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   *plst = NULL;
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          GA_CARDINAL, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if ((type_ret != GA_CARDINAL) || (format_ret != 32))
     num = -1;
   else if ((num_ret == 0) || (!prop_ret))
     num = 0;
   else
     {
        val = malloc(num_ret * sizeof(unsigned int));
        if (!val)
          {
             if (prop_ret) XFree(prop_ret);
             return -1;
          }
        for (i = 0; i < num_ret; i++)
          val[i] = ((unsigned long *)prop_ret)[i];
        num = num_ret;
        *plst = val;
     }

   if (prop_ret)
     XFree(prop_ret);

   return num;
}

/*
 * Set X ID (array) property
 */
EAPI void ecore_x_window_prop_xid_set(Ecore_Gix_Window win,
                            gi_atom_id_t atom,
                            gi_atom_id_t type,
                            gi_id_t *lst,
                            unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
                   (unsigned char *)lst, num);
#else /* if SIZEOF_INT == SIZEOF_LONG */
   unsigned long *pl;
   unsigned int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   pl = malloc(num * sizeof(long));
   if (!pl)
     return;

   for (i = 0; i < num; i++)
     pl[i] = lst[i];
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
                   (unsigned char *)pl, num);
   free(pl);
#endif /* if SIZEOF_INT == SIZEOF_LONG */
}

/*
 * Get X ID (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_xid_get(Ecore_Gix_Window win,
                            gi_atom_id_t atom,
                            gi_atom_id_t type,
                            gi_id_t *lst,
                            unsigned int len)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   int num;
   unsigned i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          type, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if (type_ret != type || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else
     {
        if (num_ret < len)
          len = num_ret;

        for (i = 0; i < len; i++)
          lst[i] = ((unsigned long *)prop_ret)[i];
        num = len;
     }

   if (prop_ret)
     XFree(prop_ret);

   return num;
}

/*
 * Get X ID (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_xid_list_get(Ecore_Gix_Window win,
                                 gi_atom_id_t atom,
                                 gi_atom_id_t type,
                                 gi_id_t **val)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   gi_atom_id_t *alst;
   int num;
   unsigned i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   *val = NULL;
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          type, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if (type_ret != type || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else
     {
        alst = malloc(num_ret * sizeof(gi_id_t));
        for (i = 0; i < num_ret; i++)
          alst[i] = ((unsigned long *)prop_ret)[i];
        num = num_ret;
        *val = alst;
     }

   if (prop_ret)
     XFree(prop_ret);

   return num;
}

/*
 * Remove/add/toggle X ID list item.
 */
EAPI void ecore_x_window_prop_xid_list_change(Ecore_Gix_Window win,
                                    gi_atom_id_t atom,
                                    gi_atom_id_t type,
                                    gi_id_t item,
                                    int op)
{
   gi_id_t *lst;
   int i, num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   num = ecore_x_window_prop_xid_list_get(win, atom, type, &lst);
   if (num < 0)
     {
        return; /* Error - assuming invalid window */
     }

   /* Is it there? */
   for (i = 0; i < num; i++)
     {
        if (lst[i] == item)
          break;
     }

   if (i < num)
     {
        /* Was in list */
        if (op == ECORE_GIX_PROP_LIST_ADD)
          goto done;  /* Remove it */

        num--;
        for (; i < num; i++)
          lst[i] = lst[i + 1];
     }
   else
     {
        /* Was not in list */
        if (op == ECORE_GIX_PROP_LIST_REMOVE)
          goto done;  /* Add it */

        num++;
        lst = realloc(lst, num * sizeof(gi_id_t));
        lst[i] = item;
     }

   ecore_x_window_prop_xid_set(win, atom, type, lst, num);

done:
   if (lst)
     free(lst);
}

/*
 * Set Atom (array) property
 */
EAPI void ecore_x_window_prop_atom_set(Ecore_Gix_Window win,
                             gi_atom_id_t atom,
                             gi_atom_id_t *lst,
                             unsigned int num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_xid_set(win, atom, GA_ATOM, lst, num);
}

/*
 * Get Atom (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_atom_get(Ecore_Gix_Window win,
                             gi_atom_id_t atom,
                             gi_atom_id_t *lst,
                             unsigned int len)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ecore_x_window_prop_xid_get(win, atom, GA_ATOM, lst, len);
}

/*
 * Get Atom (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_atom_list_get(Ecore_Gix_Window win,
                                  gi_atom_id_t atom,
                                  gi_atom_id_t **plst)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ecore_x_window_prop_xid_list_get(win, atom, GA_ATOM, plst);
}

/*
 * Remove/add/toggle atom list item.
 */
EAPI void ecore_x_window_prop_atom_list_change(Ecore_Gix_Window win,
                                     gi_atom_id_t atom,
                                     gi_atom_id_t item,
                                     int op)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_xid_list_change(win, atom, GA_ATOM, item, op);
}

/*
 * Set Window (array) property
 */
EAPI void ecore_x_window_prop_window_set(Ecore_Gix_Window win,
                               gi_atom_id_t atom,
                               Ecore_Gix_Window *lst,
                               unsigned int num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_xid_set(win, atom, GA_WINDOW, lst, num);
}

/*
 * Get Window (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_window_get(Ecore_Gix_Window win,
                               gi_atom_id_t atom,
                               Ecore_Gix_Window *lst,
                               unsigned int len)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ecore_x_window_prop_xid_get(win, atom, GA_WINDOW, lst, len);
}

/*
 * Get Window (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int ecore_x_window_prop_window_list_get(Ecore_Gix_Window win,
                                    gi_atom_id_t atom,
                                    Ecore_Gix_Window **plst)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ecore_x_window_prop_xid_list_get(win, atom, GA_WINDOW, plst);
}

EAPI gi_atom_id_t ecore_x_window_prop_any_type(void)
{
   return AnyPropertyType;
}

/**
 * @brief Set a property of Ecore_Gix_Window.
 * @param win The window for which the property will be set.
 * @param property The property of the window to be set.
 * @param type The type of the property that will be set.
 * @param size The size of the property that will be set.
 * @param data The data of the property that will be set.
 * @param number The size of data.
 */
EAPI void ecore_x_window_prop_property_set(Ecore_Gix_Window win,
                                 gi_atom_id_t property,
                                 gi_atom_id_t type,
                                 int size,
                                 void *data,
                                 int number)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (win == 0)
     win = DefaultRootWindow(_ecore_x_disp);

   if (size != 32)
     XChangeProperty(_ecore_x_disp,
                     win,
                     property,
                     type,
                     size,
                     PropModeReplace,
                     (unsigned char *)data,
                     number);
   else
     {
        unsigned long *dat;
        int i, *ptr;

        dat = malloc(sizeof(unsigned long) * number);
        if (dat)
          {
             for (ptr = (int *)data, i = 0; i < number; i++)
               dat[i] = ptr[i];
             XChangeProperty(_ecore_x_disp, win, property, type, size,
                             PropModeReplace, (unsigned char *)dat, number);
             free(dat);
          }
     }
}

/**
 * @brief Get a property of Ecore_Gix_Window.
 * @note If there aren't any data to be got the function return NULL.
 *       If the function can't allocate the memory then 0 is returned.
 * @param win The window for which the property will be got.
 * @param property The property of the window that will be gotten.
 * @param type The type of the property that will be gotten.
 * @param size This parameter isn't in use.
 * @param data The data of the property that will be gotten.
 * @param num The size of property.
 * @return size_ret The size of array that contains the property.
 */
EAPI int ecore_x_window_prop_property_get(Ecore_Gix_Window win,
                                 gi_atom_id_t property,
                                 gi_atom_id_t type,
                                 int size __UNUSED__,
                                 unsigned char **data,
                                 int *num)
{
   Atom type_ret = 0;
   int ret, size_ret = 0;
   unsigned long num_ret = 0, bytes = 0, i;
   unsigned char *prop_ret = NULL;

   /* make sure these are initialized */
   if (num)
     *num = 0;

   if (data)
     *data = NULL;
   else /* we can't store the retrieved data, so just return */
     return 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!win)
     win = DefaultRootWindow(_ecore_x_disp);

   ret = XGetWindowProperty(_ecore_x_disp, win, property, 0, LONG_MAX,
                            False, type, &type_ret, &size_ret,
                            &num_ret, &bytes, &prop_ret);

   if (ret != Success)
     return 0;

   if (!num_ret)
     {
        XFree(prop_ret);
        return 0;
     }

   if (!(*data = malloc(num_ret * size_ret / 8)))
     {
        XFree(prop_ret);
        return 0;
     }

   switch (size_ret) {
      case 8:
        for (i = 0; i < num_ret; i++)
          (*data)[i] = prop_ret[i];
        break;

      case 16:
        for (i = 0; i < num_ret; i++)
          ((unsigned short *)*data)[i] = ((unsigned short *)prop_ret)[i];
        break;

      case 32:
        for (i = 0; i < num_ret; i++)
          ((unsigned int *)*data)[i] = ((unsigned long *)prop_ret)[i];
        break;
     }

   XFree(prop_ret);

   if (num)
     *num = num_ret;

   return size_ret;
}

EAPI void ecore_x_window_prop_property_del(Ecore_Gix_Window win,
                                 gi_atom_id_t property)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XDeleteProperty(_ecore_x_disp, win, property);
}


/**
 * Set a window string property.
 * @param win The window
 * @param type The property
 * @param str The string
 *
 * Set a window string property
 */
EAPI void ecore_x_window_prop_string_set(Ecore_Gix_Window win,
                               gi_atom_id_t type,
                               const char *title)
{
   //
   gi_change_property( win, type, GA_UTF8_STRING,
                             8, G_PROP_MODE_Replace, (unsigned char *)title,
                             title ? strlen(title)+1 : 0);
}

static void *
get_prop_data (gi_window_id_t win, gi_atom_id_t prop,
               gi_atom_id_t type, int *items)
{
  gi_atom_id_t type_ret=0;
  int format_ret=0;
  unsigned long items_ret=0;
  unsigned long after_ret=0;
  unsigned char *prop_data = NULL;
  int err;

  prop_data = 0;
  err = gi_get_window_property ( win, prop, 0, 0x7fffffff, FALSE,
                                 type, &type_ret, &format_ret, &items_ret,
                                 &after_ret, &prop_data);
  if (items)
    *items = items_ret;
  return prop_data;
}

/**
 * Get a window string property.
 * @param win The window
 * @param type The property
 * @return Window string property of a window. String must be free'd when done.
 */
EAPI char *ecore_x_window_prop_string_get(Ecore_Gix_Window win,
                               gi_atom_id_t type)
{
	char *str = NULL;

	str = get_prop_data (win, type, GA_UTF8_STRING, NULL);
   

   return str;
}




/*** window *****************************************************/

EAPI Ecore_Gix_Window ecore_x_window_root_first_get(void)
{
	return GI_DESKTOP_WINDOW_ID;
}

EAPI void *ecore_x_display_get(void)
{
   return _ecore_x_disp;
}

EAPI void ecore_x_drawable_geometry_get(Ecore_Gix_Window d,
                              int *x,
                              int *y,
                              int *w,
                              int *h)
{
   int ret_x, ret_y;
   unsigned int ret_w, ret_h;
   gi_window_info_t info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (gi_get_window_info( d, &info) < 0)
     {
        ret_x = 0;
        ret_y = 0;
        ret_w = 0;
        ret_h = 0;
     }
	 else{
		 ret_x = info.x;
        ret_y = info.y;
        ret_w = info.width;
        ret_h = info.height;
	 }

   if (x)
     *x = ret_x;

   if (y)
     *y = ret_y;

   if (w)
     *w = (int)ret_w;

   if (h)
     *h = (int)ret_h;
}

EAPI void ecore_x_window_size_get(Ecore_Gix_Window win,
                        int *w,
                        int *h)
{
   int dummy_x, dummy_y;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (win == 0)
     win = GI_DESKTOP_WINDOW_ID;

   ecore_x_drawable_geometry_get(win, &dummy_x, &dummy_y, w, h);
}

EAPI Ecore_Gix_Window ecore_x_window_parent_get(Ecore_Gix_Window win)
{
   gi_window_id_t  parent = 0;
   gi_window_info_t info;

   if (gi_get_window_info(win,&info) < 0) {
	   return 0;
   }

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   parent = info.parent;
   return parent;
}

EAPI void ecore_x_window_pixmap_set(Ecore_Gix_Window win,
                          Ecore_Gix_Pixmap pmap)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_set_window_pixmap( win, pmap,0);
}

EAPI void ecore_x_window_area_clear(Ecore_Gix_Window win,
                          int x,
                          int y,
                          int w,
                          int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_clear_area( win, x, y, w, h,	FALSE);
}

EAPI void ecore_x_window_area_expose(Ecore_Gix_Window win,
                           int x,
                           int y,
                           int w,
                           int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_clear_area( win, x, y, w, h,	TRUE);
}

EAPI void ecore_x_window_delete_request_send(Ecore_Gix_Window win)
{
	gi_wm_close_window(win);
}

EAPI Eina_Bool ecore_x_bell(int percent)
{
   //int ret;

   //ret = XBell(_ecore_x_disp, percent);
   //if (ret == BadValue)
     return EINA_FALSE;

   //return EINA_TRUE;
}



EAPI void ecore_x_window_client_manage(Ecore_Gix_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSelectInput(_ecore_x_disp, win,
                PropertyChangeMask |
//		ResizeRedirectMask |
                FocusChangeMask |
                //ColormapChangeMask |
                //VisibilityChangeMask |
                StructureNotifyMask |
                SubstructureNotifyMask
                );
   //XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
}


/*** atom *****************************************************/


EAPI void ecore_x_netwm_pid_set(Ecore_Gix_Window win, int pid)
{
	gi_set_window_pid(win, pid);
}

EAPI Eina_Bool ecore_x_netwm_pid_get(Ecore_Gix_Window win, int *pid)
{
   int ret;
   gi_id_t tmp;

   ret = gi_get_window_pid(win, &tmp);
   if (!ret)
   {
	   return EINA_FALSE;
   }

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (pid)
     *pid = tmp;
   return EINA_TRUE ;
}

EAPI void ecore_x_window_client_sniff(Ecore_Gix_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_set_events_mask( win,
                GI_MASK_PROPERTYNOTIFY |
                GI_MASK_FOCUS_IN | GI_MASK_FOCUS_OUT |GI_MASK_CREATENOTIFY|
                GI_MASK_WINDOW_DESTROY | GI_MASK_PAINT_AREA| \
		GI_MASK_CONFIGURENOTIFY|GI_MASK_WINDOW_SHOW |GI_MASK_WINDOW_HIDE );
   //XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
}

EAPI void
ecore_x_netwm_name_set(Ecore_Gix_Window win,
                       const char *name)
{
	gi_set_window_utf8_caption(win, name);
}

EAPI void
ecore_x_icccm_title_set(Ecore_Gix_Window win,
                        const char *t)
{
	gi_set_window_utf8_caption(win, t);
}

EAPI Ecore_Gix_Window ecore_x_window_focus_get(void)
{
   return gi_get_focus_window(0);
}


EAPI Ecore_Gix_Window ecore_x_window_root_get(Ecore_Gix_Window win)
{
	return GI_DESKTOP_WINDOW_ID;
}

EAPI void ecore_gix_set_window_type(Ecore_Gix_Window win, const char* type)
{
	gi_atom_id_t wmtype[1];

	wmtype[0] = gi_intern_atom(type,FALSE); //"_NET_WM_WINDOW_TYPE_NORMAL"

	gi_set_wm_window_type(win, wmtype, 1);  
}

EAPI void ecore_x_icccm_name_class_set(Ecore_Gix_Window win, const char *n, const char *c)
{
}

#define ECORE_GIX_EVENT_MASK_WINDOW_CONFIGURE 0
EAPI Eina_Bool ecore_x_client_message32_send(Ecore_Gix_Window win,
                              gi_atom_id_t type,
                              unsigned mask,
                              long d0,
                              long d1,
                              long d2,
                              long d3,
                              long d4)
{

  gi_msg_t e;
  int ret;
  //gi_atom_id_t wm_state = gi_intern_atom( "_NET_ACTIVE_WINDOW", FALSE);

  e.type = GI_MSG_CLIENT_MSG;
  e.ev_window = win;
  e.effect_window = 0;
  e.body.client.client_type = type;
  e.body.client.client_format = 32;
  e.params[0] = d0;
  e.params[1] = d1;
  e.params[2] = d2;
  e.params[3] = d3;
  //e.body.client.message[0] = d4;

  ret = gi_send_event(win,FALSE,GI_MASK_CLIENT_MSG,&e);
  if (ret < 0)
  {
	  return EINA_FALSE;
  }
  return EINA_TRUE;
}


EAPI void ecore_x_atoms_get(const char **names,
	int num, Ecore_Gix_Atom *atoms)
{
   int i;

   for (i = 0; i < num; i++){
     atoms[i] = gi_intern_atom(names[i], FALSE);
   }
}

EAPI void ecore_x_netwm_opacity_set(Ecore_Gix_Window win, unsigned int opacity)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_set_window_opacity(win, opacity);
}

EAPI void ecore_x_icccm_command_set(Ecore_Gix_Window win,
	int argc, char **argv)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   gi_set_command( win, argv, argc);
}

EAPI void ecore_x_window_defaults_set(Ecore_Gix_Window win)
{
   long pid;
   int argc;
   char **argv;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   /*
    * Set WM_CLIENT_MACHINE.
    */
#if 0
   char *hostname[1];
   gethostname(buf, MAXHOSTNAMELEN);
   buf[MAXHOSTNAMELEN - 1] = '\0';
   hostname[0] = buf;
   /* The ecore function uses UTF8 which Xlib may not like (especially
    * with older clients) */
   /* ecore_x_window_prop_string_set(win, ECORE_GIX_ATOM_WM_CLIENT_MACHINE,
                                  (char *)buf); */
   char buf[MAXHOSTNAMELEN];
   XTextProperty xprop;
   if (XStringListToTextProperty(hostname, 1, &xprop))
     {
        XSetWMClientMachine(_ecore_x_disp, win, &xprop);
        XFree(xprop.value);
     }
#endif
   /*
    * Set _NET_WM_PID
    */
   pid = getpid();
   ecore_x_netwm_pid_set(win, pid);

   ecore_gix_set_window_type(win, "_NET_WM_WINDOW_TYPE_NORMAL");

   ecore_app_args_get(&argc, &argv);
   ecore_x_icccm_command_set(win, argc, argv);
}

/*** atom *****************************************************/

static Ecore_Gix_Window_Type
_ecore_x_netwm_window_type_type_get(Ecore_Gix_Atom atom)
{
   if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DESKTOP)
     return ECORE_GIX_WINDOW_TYPE_DESKTOP;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DOCK)
     return ECORE_GIX_WINDOW_TYPE_DOCK;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR)
     return ECORE_GIX_WINDOW_TYPE_TOOLBAR;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_MENU)
     return ECORE_GIX_WINDOW_TYPE_MENU;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_UTILITY)
     return ECORE_GIX_WINDOW_TYPE_UTILITY;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_SPLASH)
     return ECORE_GIX_WINDOW_TYPE_SPLASH;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DIALOG)
     return ECORE_GIX_WINDOW_TYPE_DIALOG;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NORMAL)
     return ECORE_GIX_WINDOW_TYPE_NORMAL;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU)
     return ECORE_GIX_WINDOW_TYPE_DROPDOWN_MENU;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU)
     return ECORE_GIX_WINDOW_TYPE_POPUP_MENU;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP)
     return ECORE_GIX_WINDOW_TYPE_TOOLTIP;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION)
     return ECORE_GIX_WINDOW_TYPE_NOTIFICATION;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_COMBO)
     return ECORE_GIX_WINDOW_TYPE_COMBO;
   else if (atom == ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DND)
     return ECORE_GIX_WINDOW_TYPE_DND;
   else
     return ECORE_GIX_WINDOW_TYPE_UNKNOWN;
}

static Ecore_Gix_Atom
_ecore_x_netwm_window_type_atom_get(Ecore_Gix_Window_Type type)
{
   switch (type)
     {
      case ECORE_GIX_WINDOW_TYPE_DESKTOP:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;

      case ECORE_GIX_WINDOW_TYPE_DOCK:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DOCK;

      case ECORE_GIX_WINDOW_TYPE_TOOLBAR:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;

      case ECORE_GIX_WINDOW_TYPE_MENU:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_MENU;

      case ECORE_GIX_WINDOW_TYPE_UTILITY:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_UTILITY;

      case ECORE_GIX_WINDOW_TYPE_SPLASH:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_SPLASH;

      case ECORE_GIX_WINDOW_TYPE_DIALOG:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DIALOG;

      case ECORE_GIX_WINDOW_TYPE_NORMAL:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NORMAL;

      case ECORE_GIX_WINDOW_TYPE_DROPDOWN_MENU:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU;

      case ECORE_GIX_WINDOW_TYPE_POPUP_MENU:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU;

      case ECORE_GIX_WINDOW_TYPE_TOOLTIP:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP;

      case ECORE_GIX_WINDOW_TYPE_NOTIFICATION:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION;

      case ECORE_GIX_WINDOW_TYPE_COMBO:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_COMBO;

      case ECORE_GIX_WINDOW_TYPE_DND:
        return ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DND;

      default:
        return 0;
     }
}

/*
 * FIXME: We should set WM_TRANSIENT_FOR if type is ECORE_GIX_WINDOW_TYPE_TOOLBAR
 * , ECORE_GIX_WINDOW_TYPE_MENU or ECORE_GIX_WINDOW_TYPE_DIALOG
 */
EAPI void
ecore_x_netwm_window_type_set(Ecore_Gix_Window win,
                              Ecore_Gix_Window_Type type)
{
   Ecore_Gix_Atom atom;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atom = _ecore_x_netwm_window_type_atom_get(type);
   ecore_x_window_prop_atom_set(win, ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE,
                                &atom, 1);
}

/* FIXME: Maybe return 0 on some conditions? */
EAPI Eina_Bool
ecore_x_netwm_window_type_get(Ecore_Gix_Window win,
                              Ecore_Gix_Window_Type *type)
{
   int num;
   Ecore_Gix_Atom *atoms = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (type)
     *type = ECORE_GIX_WINDOW_TYPE_NORMAL;

   num = ecore_x_window_prop_atom_list_get(win,
                                           ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE,
                                           &atoms);
   if ((type) && (num >= 1) && (atoms))
     *type = _ecore_x_netwm_window_type_type_get(atoms[0]);

   free(atoms);
   if (num >= 1)
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI int
ecore_x_netwm_window_types_get(Ecore_Gix_Window win,
                               Ecore_Gix_Window_Type **types)
{
   int num, i;
   Ecore_Gix_Atom *atoms = NULL;
   Ecore_Gix_Window_Type *atoms2 = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (types)
     *types = NULL;

   num = ecore_x_window_prop_atom_list_get(win,
                                           ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE,
                                           &atoms);
   if ((num <= 0) || (!atoms))
     {
        if (atoms)
          free(atoms);

        return 0;
     }

   atoms2 = malloc(num * sizeof(Ecore_Gix_Window_Type));
   if (!atoms2)
     return 0;

   for (i = 0; i < num; i++)
     atoms2[i] = _ecore_x_netwm_window_type_type_get(atoms[i]);
   free(atoms);
   if (types)
     *types = atoms2;
   else
     free(atoms2);

   return num;
}

EAPI char                     *ecore_x_icccm_icon_name_get(Ecore_Gix_Window win);
EAPI void                      ecore_x_icccm_icon_name_set(Ecore_Gix_Window win, const char *t);

EAPI void
ecore_x_icccm_icon_name_set(Ecore_Gix_Window win,
                            const char *t)
{
	gi_set_window_icon_name(win,t);
}


EAPI void
ecore_x_netwm_icon_name_set(Ecore_Gix_Window win,
                            const char *name)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
  // _ecore_x_window_prop_string_utf8_set(win, ECORE_GIX_ATOM_NET_WM_ICON_NAME,
   //                                     name);
   gi_set_window_icon_name(win,name);
}


EAPI char *
ecore_x_icccm_icon_name_get(Ecore_Gix_Window win)
{
	return ecore_x_window_prop_string_get(win, GA_WM_ICON_NAME);
}

EAPI void
ecore_x_icccm_window_role_set(Ecore_Gix_Window win,
                              const char *role)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_string_set(win, ECORE_GIX_ATOM_WM_WINDOW_ROLE,
                                  (char *)role);
}

EAPI char *
ecore_x_icccm_window_role_get(Ecore_Gix_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ecore_x_window_prop_string_get(win, ECORE_GIX_ATOM_WM_WINDOW_ROLE);
}

EAPI void
ecore_x_pointer_xy_get(Ecore_Gix_Window win,
                       int *x,
                       int *y)
{
   gi_window_id_t rwin, cwin;
   int rx, ry, wx, wy, ret;
   unsigned int mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = gi_query_pointer( win,  &cwin,
                       &rx, &ry, &wx, &wy, &mask);
   if (!ret)
     wx = wy = -1;

   if (x) *x = wx;
   if (y) *y = wy;
}

EAPI void
ecore_x_pointer_root_xy_get(int *x, int *y)
{
	ecore_x_pointer_xy_get(GI_DESKTOP_WINDOW_ID,x,y);
}

static int ignore_num = 0;
static Ecore_Gix_Window *ignore_list = NULL;

EAPI void
ecore_x_window_ignore_set(Ecore_Gix_Window win,
                          int ignore)
{
   int i, j, cnt;
   Ecore_Gix_Window *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (ignore)
     {
        if (ignore_list)
          {
             for (i = 0; i < ignore_num; i++)
               {
                  if (win == ignore_list[i])
                    return;
               }
             t = realloc(ignore_list, (ignore_num + 1) * sizeof(Ecore_Gix_Window));
             if (!t) return;
             ignore_list = t;
             ignore_list[ignore_num++] = win;
          }
        else
          {
             ignore_num = 0;
             ignore_list = malloc(sizeof(Ecore_Gix_Window));
             if (ignore_list)
               ignore_list[ignore_num++] = win;
          }
     }
   else
     {
        if (!ignore_list)
          return;

        for (cnt = ignore_num, i = 0, j = 0; i < cnt; i++)
          {
             if (win != ignore_list[i])
               ignore_list[j++] = ignore_list[i];
             else
               ignore_num--;
          }

        if (ignore_num <= 0)
          {
             free(ignore_list);
             ignore_list = NULL;
             return;
          }
        t = realloc(ignore_list, ignore_num * sizeof(Ecore_Gix_Window));
        if (t) ignore_list = t;
     }
}


EAPI Ecore_Gix_Window *
ecore_x_window_ignore_list(int *num)
{
   if (num)
     *num = ignore_num;

   return ignore_list;
}


EAPI void
ecore_x_window_shape_input_rectangle_set(Ecore_Gix_Window win,
                                         int x,
                                         int y,
                                         int w,
                                         int h)
{
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
}

EAPI void
ecore_x_icccm_hints_set(Ecore_Gix_Window win,
                        Eina_Bool accepts_focus,
                        Ecore_Gix_Window_State_Hint initial_state,
                        Ecore_Gix_Pixmap icon_pixmap,
                        Ecore_Gix_Pixmap icon_mask,
                        Ecore_Gix_Window icon_window,
                        Ecore_Gix_Window window_group,
                        Eina_Bool is_urgent)
{
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
}

EAPI Ecore_Gix_Window
ecore_x_window_shadow_parent_get(Ecore_Gix_Window root __UNUSED__,
                                 Ecore_Gix_Window win)
{
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
}


EAPI void
ecore_x_netwm_window_state_set(Ecore_Gix_Window win,
                               Ecore_Gix_Window_State *state,
                               unsigned int num)
{
#if 0
   Ecore_Gix_Atom *set;
   unsigned int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!num)
     {
        ecore_x_window_prop_property_del(win, ECORE_GIX_ATOM_NET_WM_STATE);
        return;
     }

   set = malloc(num * sizeof(Ecore_Gix_Atom));
   if (!set)
     return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_state_atom_get(state[i]);

   ecore_x_window_prop_atom_set(win, ECORE_GIX_ATOM_NET_WM_STATE, set, num);

   free(set);
#else
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
#endif
}



EAPI Ecore_Gix_Window
ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_Gix_Window base,
                                               int x,
                                               int y,
                                               Ecore_Gix_Window *skip,
                                               int skip_num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
   return 0;
}

EAPI void
ecore_x_window_shadow_tree_flush(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
	fprintf(stderr, "%s() not impl, Fixme!!\n",__func__);
}


