#ifndef _ECORE_GIX_H
#define _ECORE_GIX_H


#include <gi/gi.h>
#include <gi/property.h>
#include <gi/region.h>
#include <gi/gi_msg_body.h>

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else /* if __GNUC__ >= 4 */
#  define EAPI
# endif /* if __GNUC__ >= 4 */
#else /* ifdef __GNUC__ */
# define EAPI
#endif /* ifdef __GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */


/* Property list operations */
#define ECORE_GIX_PROP_LIST_REMOVE    0
#define ECORE_GIX_PROP_LIST_ADD       1
#define ECORE_GIX_PROP_LIST_TOGGLE    2
#define ECORE_GIX_ATOM_ATOM GA_ATOM

EAPI extern int ECORE_GIX_EVENT_WINDOW_PROPERTY;
EAPI extern int ECORE_GIX_EVENT_CLIENT_MESSAGE;
EAPI extern int ECORE_GIX_EVENT_GOT_FOCUS;
EAPI extern int ECORE_GIX_EVENT_LOST_FOCUS;

EAPI extern int ECORE_GIX_EVENT_MOUSE_IN ;
EAPI extern int ECORE_GIX_EVENT_MOUSE_OUT ;

EAPI extern int ECORE_GIX_EVENT_WINDOW_DAMAGE ;
EAPI extern int ECORE_GIX_EVENT_WINDOW_DELETE_REQUEST ;
EAPI extern int ECORE_GIX_EVENT_WINDOW_CONFIGURE ;

EAPI extern int ECORE_GIX_EVENT_WINDOW_CREATE ;
EAPI extern int ECORE_GIX_EVENT_DESTROYED;
EAPI extern int ECORE_GIX_EVENT_GESTURE_TOUCH;
EAPI extern int ECORE_GIX_EVENT_WINDOW_HIDE ;
EAPI extern int ECORE_GIX_EVENT_WINDOW_SHOW ;
EAPI extern int ECORE_GIX_EVENT_WINDOW_REPARENT ;


EAPI extern int ECORE_GIX_EVENT_SELECTION_CLEAR ;
EAPI extern int ECORE_GIX_EVENT_SELECTION_REQUEST ;
EAPI extern int ECORE_GIX_EVENT_SELECTION_NOTIFY ;
EAPI extern int ECORE_GIX_EVENT_CLIENT_MESSAGE ;



#ifndef _ECORE_GIX_WINDOW_PREDEF
typedef unsigned long Ecore_Gix_Window;
#define ECORE_GIX_WINDOW_ID(n) (n)
#define _ECORE_GIX_WINDOW_PREDEF
#endif /* ifndef _ECORE_GIX_WINDOW_PREDEF */

typedef gi_atom_id_t Ecore_Gix_Atom;
typedef gi_id_t Ecore_Gix_Pixmap;
typedef gi_rectangle_t Ecore_Gix_Rectangle;

typedef struct _Ecore_Gix_Cursor              Ecore_Gix_Cursor;

typedef time_t Ecore_Gix_Time;


typedef enum _Ecore_Gix_Event_Mode
{
   ECORE_GIX_EVENT_MODE_NORMAL,
   ECORE_GIX_EVENT_MODE_WHILE_GRABBED,
   ECORE_GIX_EVENT_MODE_GRAB,
   ECORE_GIX_EVENT_MODE_UNGRAB
} Ecore_Gix_Event_Mode;

struct _Ecore_Gix_Cursor
{
   gi_image_t *img;
   int               hot_x;
   int               hot_y;
};


struct _Ecore_Gix_Event_Mouse_In
{
   int                  modifiers;
   int                  x, y;
   //Eina_Bool            same_screen : 1;
   struct
   {
      int x, y;
   } root;
   Ecore_Gix_Window       win;
   Ecore_Gix_Window       event_win;
   //Ecore_Gix_Event_Mode   mode;
   int detail;
   Ecore_Gix_Time         time;
};

struct _Ecore_Gix_Event_Mouse_Out
{
   int                  modifiers;
   int                  x, y;
   //int                  same_screen;
   struct
   {
      int x, y;
   } root;
   Ecore_Gix_Window       win;
   Ecore_Gix_Window       event_win;
   //Ecore_Gix_Event_Mode   mode;
   int detail;
   Ecore_Gix_Time         time;
};

/* this struct is to keep windows data (id, window itself and surface) in memory as every call
 * to Gix for this values (e.g window->GetSurface(window,&surface)) will increment the
 * reference count, then we will have to release N times the data, so better we just ask for
   them once */
//struct _Ecore_Gix_Window
//{
//   gi_window_id_t            id;
//   Ecore_Gix_Cursor *cursor;
//};

typedef struct _Ecore_Gix_Event_Mouse_In                     Ecore_Gix_Event_Mouse_In;
typedef struct _Ecore_Gix_Event_Mouse_Out                    Ecore_Gix_Event_Mouse_Out;

typedef struct _Ecore_Gix_Event_Got_Focus     Ecore_Gix_Event_Got_Focus;
typedef struct _Ecore_Gix_Event_Lost_Focus    Ecore_Gix_Event_Lost_Focus;
typedef struct _Ecore_Gix_Event_Window_Damage       Ecore_Gix_Event_Window_Damage;

typedef struct _Ecore_Gix_Event_Selection_Clear     Ecore_Gix_Event_Selection_Clear;
typedef struct _Ecore_Gix_Event_Selection_Request   Ecore_Gix_Event_Selection_Request;
typedef struct _Ecore_Gix_Event_Selection_Notify    Ecore_Gix_Event_Selection_Notify;
typedef struct _Ecore_Gix_Selection_Data            Ecore_Gix_Selection_Data;
typedef struct _Ecore_Gix_Selection_Data_Files      Ecore_Gix_Selection_Data_Files;
typedef struct _Ecore_Gix_Selection_Data_Text       Ecore_Gix_Selection_Data_Text;
typedef struct _Ecore_Gix_Selection_Data_Targets    Ecore_Gix_Selection_Data_Targets;
typedef struct _Ecore_Gix_Event_Xdnd_Enter          Ecore_Gix_Event_Xdnd_Enter;
typedef struct _Ecore_Gix_Event_Xdnd_Position       Ecore_Gix_Event_Xdnd_Position;
typedef struct _Ecore_Gix_Event_Xdnd_Status         Ecore_Gix_Event_Xdnd_Status;
typedef struct _Ecore_Gix_Event_Xdnd_Leave          Ecore_Gix_Event_Xdnd_Leave;
typedef struct _Ecore_Gix_Event_Xdnd_Drop           Ecore_Gix_Event_Xdnd_Drop;
typedef struct _Ecore_Gix_Event_Xdnd_Finished       Ecore_Gix_Event_Xdnd_Finished;
typedef struct _Ecore_Gix_Event_Client_Message      Ecore_Gix_Event_Client_Message;

typedef struct _Ecore_Gix_Xdnd_Position                      Ecore_Gix_Xdnd_Position;


typedef struct _Ecore_Gix_Event_Window_Property     Ecore_Gix_Event_Window_Property;

typedef struct _Ecore_Gix_Event_Window_Create       Ecore_Gix_Event_Window_Create;
typedef struct _Ecore_Gix_Event_Window_Destroy      Ecore_Gix_Event_Window_Destroy;
typedef struct _Ecore_Gix_Event_Gesture_Touch      Ecore_Gix_Event_Gesture_Touch;
typedef struct _Ecore_Gix_Event_Window_Hide         Ecore_Gix_Event_Window_Hide;
typedef struct _Ecore_Gix_Event_Window_Show         Ecore_Gix_Event_Window_Show;
typedef struct _Ecore_Gix_Event_Window_Show_Request Ecore_Gix_Event_Window_Show_Request;
typedef struct _Ecore_Gix_Event_Window_Reparent     Ecore_Gix_Event_Window_Reparent;
typedef struct _Ecore_Gix_Event_Window_Configure    Ecore_Gix_Event_Window_Configure;

typedef struct _Ecore_Gix_Event_Window_Delete_Request      Ecore_Gix_Event_Window_Delete_Request;
typedef struct _Ecore_Gix_Event_Window_Move_Resize_Request Ecore_Gix_Event_Window_Move_Resize_Request;


typedef enum _Ecore_Gix_Selection {
   ECORE_GIX_SELECTION_PRIMARY,
   ECORE_GIX_SELECTION_SECONDARY,
   ECORE_GIX_SELECTION_XDND,
   ECORE_GIX_SELECTION_CLIPBOARD,
   ECORE_GIX_SELECTION_OTHER
} Ecore_Gix_Selection;


struct _Ecore_Gix_Event_Window_Create
{
   gi_window_id_t win;
   Ecore_Gix_Window parent;
   int            override;
   int            x, y, w, h;
   int            border;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Destroy
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Gesture_Touch
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   unsigned int   time;
   uint32_t params[4];
   int subtype;
   int x,y;
};

struct _Ecore_Gix_Event_Window_Hide
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Show
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Show_Request
{
   gi_window_id_t win;
   Ecore_Gix_Window parent;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Reparent
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   Ecore_Gix_Window parent;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Configure
{
   gi_window_id_t win;
   Ecore_Gix_Window event_win;
   Ecore_Gix_Window abovewin;
   int            x, y, w, h;
   int            border;
   Eina_Bool      override : 1;
   Eina_Bool      from_wm : 1;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Property
{
   Ecore_Gix_Window win;
   gi_atom_id_t   atom;
   unsigned int   time;
};


struct _Ecore_Gix_Event_Selection_Clear
{
   Ecore_Gix_Window    win;
   Ecore_Gix_Selection selection;
   gi_atom_id_t      atom;
   unsigned int      time;
};

struct _Ecore_Gix_Event_Selection_Request
{
   Ecore_Gix_Window owner;
   Ecore_Gix_Window requestor;
   unsigned int   time;
   gi_atom_id_t   selection;
   gi_atom_id_t   target;
   gi_atom_id_t   property;
};

struct _Ecore_Gix_Event_Selection_Notify
{
   Ecore_Gix_Window    win;
   unsigned int      time;
   Ecore_Gix_Selection selection;
   gi_atom_id_t      atom;
   char             *target;
   void             *data;
};

struct _Ecore_Gix_Selection_Data
{
   enum {
      ECORE_GIX_SELECTION_CONTENT_NONE,
      ECORE_GIX_SELECTION_CONTENT_TEXT,
      ECORE_GIX_SELECTION_CONTENT_FILES,
      ECORE_GIX_SELECTION_CONTENT_TARGETS,
      ECORE_GIX_SELECTION_CONTENT_CUSTOM
   } content;
   unsigned char *data;
   int            length;
   int            format;
   int            (*free)(void *data);
};

struct _Ecore_Gix_Selection_Data_Files
{
   Ecore_Gix_Selection_Data data;
   char                 **files;
   int                    num_files;
};

struct _Ecore_Gix_Selection_Data_Text
{
   Ecore_Gix_Selection_Data data;
   char                  *text;
};

struct _Ecore_Gix_Selection_Data_Targets
{
   Ecore_Gix_Selection_Data data;
   char                 **targets;
   int                    num_targets;
};

struct _Ecore_Gix_Event_Xdnd_Enter
{
   gi_window_id_t win, source;

   char         **types;
   int            num_types;
};

struct _Ecore_Gix_Event_Xdnd_Position
{
   gi_window_id_t win, source;
   struct {
     int          x, y;
   } position;
   gi_atom_id_t   action;
};

struct _Ecore_Gix_Xdnd_Position
{
   gi_window_id_t win, prev;
   struct {
     int          x, y;
   } position;
};

struct _Ecore_Gix_Event_Xdnd_Status
{
   Ecore_Gix_Window    win, target;
   Eina_Bool         will_accept : 1;
   gi_cliprect_t rectangle;
   gi_atom_id_t      action;
};

struct _Ecore_Gix_Event_Xdnd_Leave
{
   gi_window_id_t win, source;
};

struct _Ecore_Gix_Event_Xdnd_Drop
{
   gi_window_id_t win, source;
   gi_atom_id_t   action;
   struct {
      int         x, y;
   } position;
};

struct _Ecore_Gix_Event_Xdnd_Finished
{
   gi_window_id_t win, target;
   Eina_Bool           completed : 1;
   gi_atom_id_t   action;
};

struct _Ecore_Gix_Event_Client_Message
{
   gi_window_id_t win;
   gi_atom_id_t   message_type;
   int            format;
   union {
      char        b[20];
      short       s[10];
      long        l[5];
   } data;
   unsigned int   time;
};

#define ECORE_GIX_DND_VERSION                    5


struct _Ecore_Gix_Event_Window_Delete_Request
{
   gi_window_id_t win;
   unsigned int   time;
};

struct _Ecore_Gix_Event_Window_Damage
{
   gi_window_id_t win;
   int            x, y, w, h;
   int            count;
   unsigned int   time;
};




struct _Ecore_Gix_Event_Got_Focus
{
   unsigned int time;
   gi_window_id_t  win;
};

struct _Ecore_Gix_Event_Lost_Focus
{
   unsigned int time;
   gi_window_id_t  win;
};

/* main functions */
EAPI int                    ecore_gix_init(const char *name);
EAPI int                    ecore_gix_shutdown(void);
//EAPI IGix *            ecore_gix_interface_get(void);
void ecore_gix_set_default_event_filter(int (*func)(gi_msg_t *msg));

/* window operations */
EAPI Ecore_Gix_Window  ecore_gix_window_new(gi_window_id_t parent,unsigned long window_falgs, int x, int y, int w, int h);
EAPI void                   ecore_gix_window_free(Ecore_Gix_Window  window);
EAPI void                   ecore_gix_window_move(Ecore_Gix_Window  window, int x, int y);
EAPI void                   ecore_gix_window_resize(Ecore_Gix_Window  window, int w, int h);
EAPI void                   ecore_gix_window_focus(Ecore_Gix_Window  window);
EAPI void                   ecore_gix_window_show(Ecore_Gix_Window  window);
EAPI void                   ecore_gix_window_hide(Ecore_Gix_Window  window);
EAPI void                   ecore_gix_window_shaped_set(Ecore_Gix_Window  window, Eina_Bool set);
EAPI void                   ecore_gix_window_fullscreen_set(Ecore_Gix_Window  window, Eina_Bool set);
EAPI void                   ecore_gix_window_size_get(Ecore_Gix_Window  window, int *w, int *h);
EAPI void                   ecore_gix_window_cursor_show(Ecore_Gix_Window  window, Eina_Bool show);

EAPI void ecore_gix_window_borderless_set(Ecore_Gix_Window  window, Eina_Bool on);
EAPI void ecore_gix_window_iconified_set(Ecore_Gix_Window  ecore_window, Eina_Bool on);
EAPI void ecore_gix_window_lower(Ecore_Gix_Window  ecore_window);
EAPI void ecore_gix_window_raise(Ecore_Gix_Window  ecore_window);
EAPI void ecore_gix_window_title_set(Ecore_Gix_Window  window, const char *title);
EAPI time_t ecore_gix_current_time_get(void);

EAPI Ecore_Gix_Atom ecore_x_atom_get(const char *name);
EAPI void ecore_x_icccm_transient_for_set(Ecore_Gix_Window win,Ecore_Gix_Window forwin);
EAPI void ecore_x_window_prop_card32_set(Ecore_Gix_Window win,
                               Ecore_Gix_Atom atom,
                               unsigned int *val,
                               unsigned int num);
EAPI int ecore_x_window_prop_card32_get(Ecore_Gix_Window win,
                               Ecore_Gix_Atom atom,
                               unsigned int *val,
                               unsigned int len);
EAPI int ecore_x_window_prop_card32_list_get(Ecore_Gix_Window win,
                                    Ecore_Gix_Atom atom,
                                    unsigned int **plst);

EAPI void ecore_x_window_prop_xid_set(Ecore_Gix_Window win,
                            Ecore_Gix_Atom atom,
                            Ecore_Gix_Atom type,
                            gi_id_t *lst,
                            unsigned int num);
EAPI int ecore_x_window_prop_xid_get(Ecore_Gix_Window win,
                            Ecore_Gix_Atom atom,
                            Ecore_Gix_Atom type,
                            gi_id_t *lst,
                            unsigned int len);
EAPI int ecore_x_window_prop_xid_list_get(Ecore_Gix_Window win,
                                 Ecore_Gix_Atom atom,
                                 Ecore_Gix_Atom type,
                                 gi_id_t **val);

EAPI void ecore_x_window_prop_xid_list_change(Ecore_Gix_Window win,
                                    Ecore_Gix_Atom atom,
                                    Ecore_Gix_Atom type,
                                    gi_id_t item,
                                    int op);


EAPI void ecore_x_window_prop_atom_set(Ecore_Gix_Window win,
                             Ecore_Gix_Atom atom,
                             Ecore_Gix_Atom *lst,
                             unsigned int num);

EAPI int ecore_x_window_prop_atom_get(Ecore_Gix_Window win,
                             Ecore_Gix_Atom atom,
                             Ecore_Gix_Atom *lst,
                             unsigned int len);

EAPI int ecore_x_window_prop_atom_list_get(Ecore_Gix_Window win,
                                  Ecore_Gix_Atom atom,
                                  Ecore_Gix_Atom **plst);

EAPI void ecore_x_window_prop_atom_list_change(Ecore_Gix_Window win,
                                     Ecore_Gix_Atom atom,
                                     Ecore_Gix_Atom item,
                                     int op);


EAPI void ecore_x_window_prop_window_set(Ecore_Gix_Window win,
                               Ecore_Gix_Atom atom,
                               Ecore_Gix_Window *lst,
                               unsigned int num);

EAPI int ecore_x_window_prop_window_get(Ecore_Gix_Window win,
                               Ecore_Gix_Atom atom,
                               Ecore_Gix_Window *lst,
                               unsigned int len);
EAPI int ecore_x_window_prop_window_list_get(Ecore_Gix_Window win,
                                    Ecore_Gix_Atom atom,
                                    Ecore_Gix_Window **plst);
EAPI Ecore_Gix_Atom ecore_x_window_prop_any_type(void);

EAPI void ecore_x_window_prop_property_set(Ecore_Gix_Window win,
                                 Ecore_Gix_Atom property,
                                 Ecore_Gix_Atom type,
                                 int size,
                                 void *data,
                                 int number);

EAPI int ecore_x_window_prop_property_get(Ecore_Gix_Window win,
                                 Ecore_Gix_Atom property,
                                 Ecore_Gix_Atom type,
                                 int size ,
                                 unsigned char **data,
                                 int *num);


EAPI void ecore_x_window_prop_property_del(Ecore_Gix_Window win,
                                 Ecore_Gix_Atom property);

EAPI void ecore_x_window_prop_string_set(Ecore_Gix_Window win,
                               Ecore_Gix_Atom type,
                               const char *title);
EAPI char *ecore_x_window_prop_string_get(Ecore_Gix_Window win,
                               Ecore_Gix_Atom type);

EAPI Ecore_Gix_Window ecore_x_window_root_first_get(void);
EAPI void *ecore_x_display_get(void);
EAPI void ecore_x_drawable_geometry_get(Ecore_Gix_Window d, int *x, int *y, int *w, int *h);
#define ecore_x_window_geometry_get ecore_x_drawable_geometry_get
EAPI Eina_Bool ecore_x_bell(int percent);
EAPI void ecore_x_window_client_manage(Ecore_Gix_Window win);
EAPI void ecore_x_window_size_get(Ecore_Gix_Window win, int *w, int *h);
EAPI Ecore_Gix_Window ecore_x_window_parent_get(Ecore_Gix_Window win);
EAPI void ecore_x_window_pixmap_set(Ecore_Gix_Window win,
                          Ecore_Gix_Pixmap pmap);

EAPI Ecore_Gix_Window ecore_x_window_focus_get(void);
EAPI Ecore_Gix_Window ecore_x_window_root_get(Ecore_Gix_Window win);
EAPI void ecore_x_netwm_opacity_set(Ecore_Gix_Window win, unsigned int opacity);

EAPI void ecore_gix_set_window_type(Ecore_Gix_Window win, const char* type);
EAPI void ecore_x_icccm_name_class_set(Ecore_Gix_Window win, const char *n, const char *c);
EAPI Eina_Bool ecore_x_client_message32_send(Ecore_Gix_Window win,
                              gi_atom_id_t type,
                              unsigned mask,
                              long d0,
                              long d1,
                              long d2,
                              long d3,
                              long d4);

typedef enum _Ecore_Gix_Illume_Mode
{
   ECORE_GIX_ILLUME_MODE_UNKNOWN = 0,
   ECORE_GIX_ILLUME_MODE_SINGLE,
   ECORE_GIX_ILLUME_MODE_DUAL_TOP,
   ECORE_GIX_ILLUME_MODE_DUAL_LEFT
} Ecore_Gix_Illume_Mode;

typedef enum _Ecore_Gix_Illume_Quickpanel_State
{
   ECORE_GIX_ILLUME_QUICKPANEL_STATE_UNKNOWN = 0,
   ECORE_GIX_ILLUME_QUICKPANEL_STATE_OFF,
   ECORE_GIX_ILLUME_QUICKPANEL_STATE_ON
} Ecore_Gix_Illume_Quickpanel_State;

typedef enum _Ecore_Gix_Illume_Indicator_State
{
   ECORE_GIX_ILLUME_INDICATOR_STATE_UNKNOWN = 0,
   ECORE_GIX_ILLUME_INDICATOR_STATE_OFF,
   ECORE_GIX_ILLUME_INDICATOR_STATE_ON
} Ecore_Gix_Illume_Indicator_State;

typedef enum _Ecore_Gix_Illume_Clipboard_State
{
   ECORE_GIX_ILLUME_CLIPBOARD_STATE_UNKNOWN = 0,
   ECORE_GIX_ILLUME_CLIPBOARD_STATE_OFF,
   ECORE_GIX_ILLUME_CLIPBOARD_STATE_ON
} Ecore_Gix_Illume_Clipboard_State;

typedef enum _Ecore_Gix_Illume_Indicator_Opacity_Mode
{
   ECORE_GIX_ILLUME_INDICATOR_OPACITY_UNKNOWN = 0,
   ECORE_GIX_ILLUME_INDICATOR_OPAQUE,
   ECORE_GIX_ILLUME_INDICATOR_TRANSLUCENT,
   ECORE_GIX_ILLUME_INDICATOR_TRANSPARENT
} Ecore_Gix_Illume_Indicator_Opacity_Mode;

typedef enum _Ecore_Gix_Illume_Window_State
{
   ECORE_GIX_ILLUME_WINDOW_STATE_NORMAL = 0,
   ECORE_GIX_ILLUME_WINDOW_STATE_FLOATING
} Ecore_Gix_Illume_Window_State;

EAPI void ecore_x_e_illume_quickpanel_state_send(Ecore_Gix_Window win, Ecore_Gix_Illume_Quickpanel_State state);
EAPI void ecore_x_e_illume_quickpanel_zone_set(Ecore_Gix_Window win, unsigned int zone);
EAPI int ecore_x_e_illume_quickpanel_zone_get(Ecore_Gix_Window win);
EAPI Ecore_Gix_Window ecore_x_e_illume_zone_get(Ecore_Gix_Window win);
EAPI void ecore_x_e_illume_quickpanel_state_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Quickpanel_State state);
EAPI Ecore_Gix_Illume_Quickpanel_State ecore_x_e_illume_quickpanel_state_get(Ecore_Gix_Window win);
EAPI void ecore_x_e_illume_quickpanel_state_toggle(Ecore_Gix_Window win);

EAPI void ecore_x_window_client_sniff(Ecore_Gix_Window win);
EAPI Eina_Bool ecore_x_netwm_pid_get(Ecore_Gix_Window win, int *pid);
EAPI void ecore_x_netwm_pid_set(Ecore_Gix_Window win, int pid);

EAPI void ecore_x_window_defaults_set(Ecore_Gix_Window win);
EAPI void ecore_x_icccm_command_set(Ecore_Gix_Window win,int argc, char **argv);

EAPI void ecore_x_atoms_get(const char **names,	int num, Ecore_Gix_Atom *atoms);


#define ECORE_GIX_SELECTION_TARGET_TARGETS       "TARGETS"
#define ECORE_GIX_SELECTION_TARGET_TEXT          "TEXT"
#define ECORE_GIX_SELECTION_TARGET_COMPOUND_TEXT "COMPOUND_TEXT"
#define ECORE_GIX_SELECTION_TARGET_STRING        "STRING"
#define ECORE_GIX_SELECTION_TARGET_UTF8_STRING   "UTF8_STRING"
#define ECORE_GIX_SELECTION_TARGET_FILENAME      "FILENAME"


typedef struct _Ecore_Gix_Selection_Intern Ecore_Gix_Selection_Intern;

struct _Ecore_Gix_Selection_Intern
{
   Ecore_Gix_Window win;
   Ecore_Gix_Atom   selection;
   unsigned char *data;
   int            length;
   time_t           time;
};


typedef struct _Ecore_Gix_Selection_Converter Ecore_Gix_Selection_Converter;

struct _Ecore_Gix_Selection_Converter
{
   Ecore_Gix_Atom                 target;
   Eina_Bool                    (*convert)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_Gix_Atom *type, int *typeseize);
   Ecore_Gix_Selection_Converter *next;
};

typedef struct _Ecore_Gix_Selection_Parser Ecore_Gix_Selection_Parser;

struct _Ecore_Gix_Selection_Parser
{
   char                     *target;
   void                     *(*parse)(const char *target, void *data, int size, int format);
   Ecore_Gix_Selection_Parser *next;
};

typedef struct _Ecore_Gix_DND_Source
{
   int            version;
   Ecore_Gix_Window win, dest;

   enum {
      ECORE_GIX_DND_SOURCE_IDLE,
      ECORE_GIX_DND_SOURCE_DRAGGING,
      ECORE_GIX_DND_SOURCE_DROPPED,
      ECORE_GIX_DND_SOURCE_CONVERTING
   } state;

   struct
   {
      short          x, y;
      unsigned short width, height;
   } rectangle;

   struct
   {
      Ecore_Gix_Window window;
      int            x, y;
   } prev;

   time_t         time;

   Ecore_Gix_Atom action, accepted_action;

   int          will_accept;
   int          suppress;

   int          await_status;
} Ecore_Gix_DND_Source;

typedef struct _Ecore_Gix_DND_Target
{
   int            version;
   Ecore_Gix_Window win, source;

   enum {
      ECORE_GIX_DND_TARGET_IDLE,
      ECORE_GIX_DND_TARGET_ENTERED
   } state;

   struct
   {
      int x, y;
   } pos;

   time_t         time;

   Ecore_Gix_Atom action, accepted_action;

   int          will_accept;
} Ecore_Gix_DND_Target;


typedef enum _Ecore_Gix_Virtual_Keyboard_State
{
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_UNKNOWN = 0,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_OFF,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_ON,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_ALPHA,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_NUMERIC,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_PIN,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_HEX,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_TERMINAL,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_PASSWORD,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_IP,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_HOST,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_FILE,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_URL,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_KEYPAD,
   ECORE_GIX_VIRTUAL_KEYBOARD_STATE_J2ME
} Ecore_Gix_Virtual_Keyboard_State;



typedef enum _Ecore_Gix_Window_Type
{
   ECORE_GIX_WINDOW_TYPE_UNKNOWN = 0,
   ECORE_GIX_WINDOW_TYPE_DESKTOP,
   ECORE_GIX_WINDOW_TYPE_DOCK,
   ECORE_GIX_WINDOW_TYPE_TOOLBAR,
   ECORE_GIX_WINDOW_TYPE_MENU,
   ECORE_GIX_WINDOW_TYPE_UTILITY,
   ECORE_GIX_WINDOW_TYPE_SPLASH,
   ECORE_GIX_WINDOW_TYPE_DIALOG,
   ECORE_GIX_WINDOW_TYPE_NORMAL,
   ECORE_GIX_WINDOW_TYPE_DROPDOWN_MENU,
   ECORE_GIX_WINDOW_TYPE_POPUP_MENU,
   ECORE_GIX_WINDOW_TYPE_TOOLTIP,
   ECORE_GIX_WINDOW_TYPE_NOTIFICATION,
   ECORE_GIX_WINDOW_TYPE_COMBO,
   ECORE_GIX_WINDOW_TYPE_DND
} Ecore_Gix_Window_Type;

EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_ICON_GEOMETRY;

EAPI extern int ECORE_GIX_EVENT_XDND_ENTER;
EAPI extern int ECORE_GIX_EVENT_XDND_POSITION;
EAPI extern int ECORE_GIX_EVENT_XDND_STATUS;
EAPI extern int ECORE_GIX_EVENT_XDND_LEAVE;
EAPI extern int ECORE_GIX_EVENT_XDND_DROP;
EAPI extern int ECORE_GIX_EVENT_XDND_FINISHED;


/* window type */
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DOCK;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_MENU;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NORMAL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_COMBO;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_NET_WM_WINDOW_TYPE_DND;

/* dnd atoms */
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_XDND;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_XDND;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_AWARE;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ENTER;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_TYPE_LIST;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_POSITION;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_COPY;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_MOVE;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_PRIVATE;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_ASK;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_LIST;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_LINK;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_ACTION_DESCRIPTION;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_PROXY;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_STATUS;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_LEAVE;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_DROP;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_ATOM_XDND_FINISHED;

/* dnd atoms that need to be exposed to the application interface */
EAPI extern  Ecore_Gix_Atom ECORE_GIX_DND_ACTION_COPY;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_DND_ACTION_MOVE;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_DND_ACTION_LINK;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_DND_ACTION_ASK;
EAPI extern  Ecore_Gix_Atom ECORE_GIX_DND_ACTION_PRIVATE;

EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_TARGETS;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PRIMARY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_SECONDARY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_CLIPBOARD;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_PRIMARY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_SECONDARY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_SELECTION_PROP_CLIPBOARD;


EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_COMPOUND_TEXT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_FILE_NAME;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_TEXT;


/* Illume specific atoms */
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ZONE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ZONE_LIST;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CONFORMANT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_SINGLE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_TOP;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_MODE_DUAL_LEFT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_BACK;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_FORWARD;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_FOCUS_HOME;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_HOME_NEW;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_HOME_DEL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLOSE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_LOCKED;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_START;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_DRAG_END;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_GEOMETRY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_SOFTKEY_GEOMETRY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_KEYBOARD_GEOMETRY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ON;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_OFF;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_ZONE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_STATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_ON;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OFF;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_OPAQUE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSLUCENT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_INDICATOR_TRANSPARENT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_AVAILABLE_ANGLE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_STATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_ON;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_OFF;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE_NORMAL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_WINDOW_STATE_FLOATING;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_CONTROL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_NEXT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_PREV;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV;

EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_WM_WINDOW_ROLE;

EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_STATE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ON;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_OFF;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_ALPHA;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PIN;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HEX;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_IP;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_HOST;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_FILE;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_URL;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD;
EAPI extern Ecore_Gix_Atom ECORE_GIX_ATOM_E_VIRTUAL_KEYBOARD_J2ME;


EAPI Eina_Bool                 ecore_x_selection_notify_send(Ecore_Gix_Window requestor, Ecore_Gix_Atom selection, Ecore_Gix_Atom target, Ecore_Gix_Atom property, Ecore_Gix_Time time);
EAPI Eina_Bool                 ecore_x_selection_primary_set(Ecore_Gix_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_primary_clear(void);
EAPI Eina_Bool                 ecore_x_selection_secondary_set(Ecore_Gix_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_secondary_clear(void);
EAPI Eina_Bool                 ecore_x_selection_xdnd_set(Ecore_Gix_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_xdnd_clear(void);
EAPI Eina_Bool                 ecore_x_selection_clipboard_set(Ecore_Gix_Window w, const void *data, int size);
EAPI Eina_Bool                 ecore_x_selection_clipboard_clear(void);
EAPI void                      ecore_x_selection_primary_request(Ecore_Gix_Window w, const char *target);
EAPI void                      ecore_x_selection_secondary_request(Ecore_Gix_Window w, const char *target);
EAPI void                      ecore_x_selection_xdnd_request(Ecore_Gix_Window w, const char *target);
EAPI void                      ecore_x_selection_clipboard_request(Ecore_Gix_Window w, const char *target);
EAPI Eina_Bool                 ecore_x_selection_convert(Ecore_Gix_Atom selection, Ecore_Gix_Atom target, void **data_ret, int *len, Ecore_Gix_Atom *targprop, int *targsize);
EAPI void                      ecore_x_selection_converter_add(char *target, Eina_Bool (*func)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_Gix_Atom *, int *));
EAPI void                      ecore_x_selection_converter_atom_add(Ecore_Gix_Atom target, Eina_Bool (*func)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_Gix_Atom *tprop, int *tsize));
EAPI void                      ecore_x_selection_converter_del(char *target);
EAPI void                      ecore_x_selection_converter_atom_del(Ecore_Gix_Atom target);
EAPI void                      ecore_x_selection_parser_add(const char *target, void *(*func)(const char *target, void *data, int size, int format));
EAPI void                      ecore_x_selection_parser_del(const char *target);
EAPI void                      ecore_x_selection_owner_set(Ecore_Gix_Window win, Ecore_Gix_Atom atom, Ecore_Gix_Time tm);
EAPI Ecore_Gix_Window            ecore_x_selection_owner_get(Ecore_Gix_Atom atom);


EAPI void                      ecore_x_dnd_aware_set(Ecore_Gix_Window win, Eina_Bool on);
EAPI int                       ecore_x_dnd_version_get(Ecore_Gix_Window win);
EAPI Eina_Bool                 ecore_x_dnd_type_isset(Ecore_Gix_Window win, const char *type);
EAPI void                      ecore_x_dnd_type_set(Ecore_Gix_Window win, const char *type, Eina_Bool on);
EAPI void                      ecore_x_dnd_types_set(Ecore_Gix_Window win, const char **types, unsigned int num_types);
EAPI void                      ecore_x_dnd_actions_set(Ecore_Gix_Window win, Ecore_Gix_Atom *actions, unsigned int num_actions);
EAPI Eina_Bool                 ecore_x_dnd_begin(Ecore_Gix_Window source, unsigned char *data, int size);
EAPI Eina_Bool                 ecore_x_dnd_drop(void);
EAPI void                      ecore_x_dnd_send_status(Eina_Bool will_accept, Eina_Bool suppress, Ecore_Gix_Rectangle rectangle, Ecore_Gix_Atom action);
EAPI void                      ecore_x_dnd_send_finished(void);
EAPI void                      ecore_x_dnd_source_action_set(Ecore_Gix_Atom action);
EAPI Ecore_Gix_Atom              ecore_x_dnd_source_action_get(void);
EAPI void                      ecore_x_dnd_callback_pos_update_set(void (*cb)(void *, Ecore_Gix_Xdnd_Position *data), const void *data);





/* Illume functions */
EAPI void                            ecore_x_e_illume_zone_set(Ecore_Gix_Window win, Ecore_Gix_Window zone);
EAPI Ecore_Gix_Window                  ecore_x_e_illume_zone_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_zone_list_set(Ecore_Gix_Window win, Ecore_Gix_Window *zones, unsigned int n_zones);
EAPI void                            ecore_x_e_illume_conformant_set(Ecore_Gix_Window win, unsigned int is_conformant);
EAPI Eina_Bool                       ecore_x_e_illume_conformant_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_mode_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Mode mode);
EAPI Ecore_Gix_Illume_Mode             ecore_x_e_illume_mode_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_mode_send(Ecore_Gix_Window win, Ecore_Gix_Illume_Mode mode);
EAPI void                            ecore_x_e_illume_focus_back_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_focus_forward_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_focus_home_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_close_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_home_new_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_home_del_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_next_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_prev_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_activate_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_next_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_access_action_read_prev_send(Ecore_Gix_Window win);

EAPI void                            ecore_x_e_illume_drag_set(Ecore_Gix_Window win, unsigned int drag);
EAPI Eina_Bool                       ecore_x_e_illume_drag_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_drag_locked_set(Ecore_Gix_Window win, unsigned int is_locked);
EAPI Eina_Bool                       ecore_x_e_illume_drag_locked_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_drag_start_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_drag_end_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_indicator_geometry_set(Ecore_Gix_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_indicator_geometry_get(Ecore_Gix_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_softkey_geometry_set(Ecore_Gix_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_softkey_geometry_get(Ecore_Gix_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_keyboard_geometry_set(Ecore_Gix_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_keyboard_geometry_get(Ecore_Gix_Window win, int *x, int *y, int *w, int *h);
EAPI void                            ecore_x_e_illume_quickpanel_set(Ecore_Gix_Window win, unsigned int is_quickpanel);
EAPI Eina_Bool                       ecore_x_e_illume_quickpanel_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_state_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Quickpanel_State state);
EAPI Ecore_Gix_Illume_Quickpanel_State ecore_x_e_illume_quickpanel_state_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_state_send(Ecore_Gix_Window win, Ecore_Gix_Illume_Quickpanel_State state);
EAPI void                            ecore_x_e_illume_quickpanel_state_toggle(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_priority_major_set(Ecore_Gix_Window win, unsigned int priority);
EAPI int                             ecore_x_e_illume_quickpanel_priority_major_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_priority_minor_set(Ecore_Gix_Window win, unsigned int priority);
EAPI int                             ecore_x_e_illume_quickpanel_priority_minor_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_zone_set(Ecore_Gix_Window win, unsigned int zone);
EAPI int                             ecore_x_e_illume_quickpanel_zone_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_zone_request_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_illume_quickpanel_position_update_send(Ecore_Gix_Window win);

EAPI void                            ecore_x_e_illume_clipboard_state_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Clipboard_State state);

EAPI Ecore_Gix_Illume_Clipboard_State  ecore_x_e_illume_clipboard_state_get(Ecore_Gix_Window win);

EAPI void                            ecore_x_e_illume_clipboard_geometry_set(Ecore_Gix_Window win, int x, int y, int w, int h);
EAPI Eina_Bool                       ecore_x_e_illume_clipboard_geometry_get(Ecore_Gix_Window win, int *x, int *y, int *w, int *h);
//EAPI void                            ecore_x_e_comp_sync_counter_set(Ecore_Gix_Window win, Ecore_Gix_Sync_Counter counter);
//EAPI Ecore_Gix_Sync_Counter            ecore_x_e_comp_sync_counter_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_sync_draw_done_send(Ecore_Gix_Window root, Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_sync_draw_size_done_send(Ecore_Gix_Window root, Ecore_Gix_Window win, int w, int h);
EAPI void                            ecore_x_e_comp_sync_supported_set(Ecore_Gix_Window root, Eina_Bool enabled);
EAPI Eina_Bool                       ecore_x_e_comp_sync_supported_get(Ecore_Gix_Window root);
EAPI void                            ecore_x_e_comp_sync_begin_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_sync_end_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_sync_cancel_send(Ecore_Gix_Window win);

EAPI void                            ecore_x_e_comp_flush_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_dump_send(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_comp_pixmap_set(Ecore_Gix_Window win, Ecore_Gix_Pixmap pixmap);
EAPI Ecore_Gix_Pixmap                  ecore_x_e_comp_pixmap_get(Ecore_Gix_Window win);

EAPI char                            *ecore_x_e_window_profile_get(Ecore_Gix_Window win);
EAPI void                             ecore_x_e_window_profile_set(Ecore_Gix_Window win, const char *profile);
EAPI void                             ecore_x_e_window_profile_list_set(Ecore_Gix_Window  win, const char **profiles, unsigned int num_profiles);
EAPI Eina_Bool                        ecore_x_e_window_profile_list_get(Ecore_Gix_Window win, const char ***profiles, int *ret_num);


typedef struct _Ecore_Gix_Icon
{
   unsigned int  width, height;
   unsigned int *data;
} Ecore_Gix_Icon;

EAPI void                            ecore_x_netwm_icons_set(Ecore_Gix_Window win, Ecore_Gix_Icon *icon, int num);

EAPI Eina_Bool                       ecore_x_netwm_icons_get(Ecore_Gix_Window win, Ecore_Gix_Icon **icon, int *num);
EAPI void                            ecore_x_netwm_icon_geometry_set(Ecore_Gix_Window win, int x, int y, int width, int height);
EAPI Eina_Bool                       ecore_x_netwm_icon_geometry_get(Ecore_Gix_Window win, int *x, int *y, int *width, int *height);
EAPI void                            ecore_x_netwm_pid_set(Ecore_Gix_Window win, int pid);
EAPI Eina_Bool                       ecore_x_netwm_pid_get(Ecore_Gix_Window win, int *pid);
EAPI void                            ecore_x_netwm_handled_icons_set(Ecore_Gix_Window win);
EAPI Eina_Bool                       ecore_x_netwm_handled_icons_get(Ecore_Gix_Window win);

EAPI void                            ecore_x_e_init(void);
EAPI void                            ecore_x_e_frame_size_set(Ecore_Gix_Window win, int fl, int fr, int ft, int fb);
EAPI void                            ecore_x_e_virtual_keyboard_set(Ecore_Gix_Window win, unsigned int is_keyboard);
EAPI Eina_Bool                       ecore_x_e_virtual_keyboard_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_virtual_keyboard_state_set(Ecore_Gix_Window win, Ecore_Gix_Virtual_Keyboard_State state);
EAPI Ecore_Gix_Virtual_Keyboard_State  ecore_x_e_virtual_keyboard_state_get(Ecore_Gix_Window win);
EAPI void                            ecore_x_e_virtual_keyboard_state_send(Ecore_Gix_Window win, Ecore_Gix_Virtual_Keyboard_State state);


typedef enum _Ecore_Gix_Window_State
{
   ECORE_GIX_WINDOW_STATE_UNKNOWN = 0,
   ECORE_GIX_WINDOW_STATE_ICONIFIED,  /** The window is iconified. */
   ECORE_GIX_WINDOW_STATE_MODAL,  /** The window is a modal dialog box. */
   ECORE_GIX_WINDOW_STATE_STICKY, /** The window manager should keep the window's position fixed
                                 * even if the virtual desktop scrolls. */
   ECORE_GIX_WINDOW_STATE_MAXIMIZED_VERT,  /** The window has the maximum vertical size. */
   ECORE_GIX_WINDOW_STATE_MAXIMIZED_HORZ,  /** The window has the maximum horizontal size. */
   ECORE_GIX_WINDOW_STATE_SHADED,  /** The window is shaded. */
   ECORE_GIX_WINDOW_STATE_SKIP_TASKBAR,  /** The window should not be included in the taskbar. */
   ECORE_GIX_WINDOW_STATE_SKIP_PAGER,  /** The window should not be included in the pager. */
   ECORE_GIX_WINDOW_STATE_HIDDEN,  /** The window is invisible (i.e. minimized/iconified) */
   ECORE_GIX_WINDOW_STATE_FULLSCREEN,  /** The window should fill the entire screen and have no
                                      * window border/decorations */
   ECORE_GIX_WINDOW_STATE_ABOVE,
   ECORE_GIX_WINDOW_STATE_BELOW,
   ECORE_GIX_WINDOW_STATE_DEMANDS_ATTENTION
} Ecore_Gix_Window_State;

EAPI void                            ecore_x_netwm_window_state_set(Ecore_Gix_Window win, Ecore_Gix_Window_State *state, unsigned int num);
EAPI Eina_Bool                       ecore_x_netwm_window_state_get(Ecore_Gix_Window win, Ecore_Gix_Window_State **state, unsigned int *num);
EAPI void                            ecore_x_netwm_window_type_set(Ecore_Gix_Window win, Ecore_Gix_Window_Type type);
EAPI Eina_Bool                       ecore_x_netwm_window_type_get(Ecore_Gix_Window win, Ecore_Gix_Window_Type *type);
EAPI int                             ecore_x_netwm_window_types_get(Ecore_Gix_Window win, Ecore_Gix_Window_Type **types);
//EAPI Eina_Bool                       ecore_x_netwm_allowed_action_isset(Ecore_Gix_Window win, Ecore_Gix_Action action);
//EAPI void                            ecore_x_netwm_allowed_action_set(Ecore_Gix_Window win, Ecore_Gix_Action *action, unsigned int num);
//EAPI Eina_Bool                       ecore_x_netwm_allowed_action_get(Ecore_Gix_Window win, Ecore_Gix_Action **action, unsigned int *num);
EAPI void                            ecore_x_netwm_opacity_set(Ecore_Gix_Window win, unsigned int opacity);
EAPI Eina_Bool                       ecore_x_netwm_opacity_get(Ecore_Gix_Window win, unsigned int *opacity);

EAPI void                      ecore_x_icccm_window_role_set(Ecore_Gix_Window win, const char *role);
EAPI char                     *ecore_x_icccm_window_role_get(Ecore_Gix_Window win);

EAPI char                     *ecore_x_icccm_icon_name_get(Ecore_Gix_Window win);
EAPI void                      ecore_x_icccm_icon_name_set(Ecore_Gix_Window win, const char *t);

EAPI void                                  ecore_x_e_illume_indicator_state_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Indicator_State state);
EAPI Ecore_Gix_Illume_Indicator_State        ecore_x_e_illume_indicator_state_get(Ecore_Gix_Window win);


EAPI void                                  ecore_x_e_illume_indicator_state_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Indicator_State state);
EAPI Ecore_Gix_Illume_Indicator_State        ecore_x_e_illume_indicator_state_get(Ecore_Gix_Window win);
EAPI void                                  ecore_x_e_illume_indicator_state_send(Ecore_Gix_Window win, Ecore_Gix_Illume_Indicator_State state);

EAPI void                                  ecore_x_e_illume_indicator_opacity_set(Ecore_Gix_Window win, Ecore_Gix_Illume_Indicator_Opacity_Mode mode);

EAPI Ecore_Gix_Illume_Indicator_Opacity_Mode ecore_x_e_illume_indicator_opacity_get(Ecore_Gix_Window win);

EAPI void                                  ecore_x_e_illume_indicator_opacity_send(Ecore_Gix_Window win, Ecore_Gix_Illume_Indicator_Opacity_Mode mode);

EAPI void
ecore_x_e_illume_window_state_set(Ecore_Gix_Window win,
                                  Ecore_Gix_Illume_Window_State state);

EAPI Ecore_Gix_Illume_Window_State           ecore_x_e_illume_window_state_get(Ecore_Gix_Window win);

typedef enum _Ecore_Gix_Window_State_Hint
{
   /** Do not provide any state hint to the window manager */
   ECORE_GIX_WINDOW_STATE_HINT_NONE = -1,

   /** The window wants to remain hidden and NOT iconified */
   ECORE_GIX_WINDOW_STATE_HINT_WITHDRAWN,

   /** The window wants to be mapped normally */
   ECORE_GIX_WINDOW_STATE_HINT_NORMAL,

   /** The window wants to start in an iconified state */
   ECORE_GIX_WINDOW_STATE_HINT_ICONIC
} Ecore_Gix_Window_State_Hint;


EAPI void                      ecore_x_icccm_hints_set(Ecore_Gix_Window win, Eina_Bool accepts_focus, Ecore_Gix_Window_State_Hint initial_state, Ecore_Gix_Pixmap icon_pixmap, Ecore_Gix_Pixmap icon_mask, Ecore_Gix_Window icon_window, Ecore_Gix_Window window_group, Eina_Bool is_urgent);
EAPI void                            ecore_x_netwm_icon_name_set(Ecore_Gix_Window win, const char *name);
EAPI void             ecore_x_pointer_xy_get(Ecore_Gix_Window win, int *x, int *y);

EAPI void                      ecore_x_window_shape_input_rectangle_set(Ecore_Gix_Window win, int x, int y, int w, int h);

EAPI void                      ecore_x_window_ignore_set(Ecore_Gix_Window win, int ignore);
EAPI Ecore_Gix_Window           *ecore_x_window_ignore_list(int *num);


EAPI Ecore_Gix_Window            ecore_x_window_shadow_parent_get(Ecore_Gix_Window root, Ecore_Gix_Window win);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _ECORE_GIX_H */
