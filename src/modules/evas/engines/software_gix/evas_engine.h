#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H



#include <sys/ipc.h>
//#include <sys/shm.h>


#include <gi/gi.h>
#include <gi/property.h>
#include <gi/region.h>
#define __UNUSED__

#include "../software_generic/Evas_Engine_Software_Generic.h"

extern int _evas_engine_soft_gix_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_soft_gix_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_soft_gix_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_soft_gix_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_soft_gix_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_soft_gix_log_dom, __VA_ARGS__)

//typedef struct BITMAPINFO_GDI     BITMAPINFO_GDI;

typedef struct _Gix_Output_Buffer Gix_Output_Buffer;
/*
struct BITMAPINFO_GDI
{
   BITMAPINFOHEADER bih;
   DWORD            masks[3];
};
*/

struct _Outbuf
{
   int                   width;
   int                   height;
   int                   rot;
   int                   onebuf;
   //int                   depth;

   struct {
      Convert_Pal       *pal;
      struct {
		  // gi_window_id_t            win;
            gi_gc_ptr_t                gc;
			gi_format_code_t ogformat;

         //BITMAPINFO_GDI *bitmap_info;
         gi_window_id_t            window;
        // HDC             dc;
         //HRGN            regions;
         unsigned char   borderless : 1;
         unsigned char   fullscreen : 1;
         unsigned char   region     : 1;
		  unsigned char     swap     : 1;
            unsigned char     bit_swap : 1;
      } gix_platform;

	  struct {
		DATA32    r, g, b;
      } mask;

      /* 1 big buffer for updates - flush on idle_flush */
      RGBA_Image        *onebuf;
      Eina_List         *onebuf_regions;

      /* a list of pending regions to write to the target */
      Eina_List         *pending_writes;
      /* a list of previous frame pending regions to write to the target */
      Eina_List         *prev_pending_writes;

      unsigned char      mask_dither       : 1;
      unsigned char      destination_alpha : 1;
      unsigned char      debug             : 1;
      unsigned char      synced            : 1;

      unsigned char      region_built      : 1;
   } priv;
};


struct _Gix_Output_Buffer
{
	gi_image_t          *xim;
   gi_bool_t need_free;
   //void            *data;
   //BITMAPINFO_GDI *bitmap_info;
   //HBITMAP         bitmap;
   //HDC             dc;
   int             w;
   int             h;
   void           *data;
   int             pitch;
   int             psize;
   int             depth;
};

/* evas_gix_main.c */

int evas_software_gix_init (gi_window_id_t         window,
                            unsigned int borderless,
                            unsigned int fullscreen,
                            unsigned int region,
                            Outbuf      *buf);

void evas_software_gix_shutdown(Outbuf *buf);

void evas_software_gix_bitmap_resize(Outbuf *buf);

/* evas_gix_buffer.c */

Gix_Output_Buffer *evas_software_gix_output_buffer_new(gi_format_code_t format,
                                                       int             width,
                                                       int             height,
                                                       void           *data);

void evas_software_gix_output_buffer_free(Gix_Output_Buffer *gdiob);

void evas_software_gix_output_buffer_paste(Gix_Output_Buffer *gdiob,gi_window_id_t d,
	gi_gc_ptr_t gc,
                                           int                x,
                                           int                y);

DATA8 *evas_software_gix_output_buffer_data(Gix_Output_Buffer *gdiob,
                                            int               *pitch);

/* evas_outbuf.c */

void evas_software_gix_outbuf_init(void);

void evas_software_gix_outbuf_free(Outbuf *buf);

Outbuf *
evas_software_gix_outbuf_setup(int width, int height, int rot, 
				  gi_window_id_t draw, 
				  gi_format_code_t format,
				 int destination_alpha);

void evas_software_gix_outbuf_reconfigure(Outbuf      *buf,
                                          int          width,
                                          int          height,
                                          int          rotation,
                                          Outbuf_Depth depth);

void *evas_software_gix_outbuf_new_region_for_update(Outbuf *buf,
                                                     int     x,
                                                     int     y,
                                                     int     w,
                                                     int     h,
                                                     int    *cx,
                                                     int    *cy,
                                                     int    *cw,
                                                     int    *ch);

void evas_software_gix_outbuf_push_updated_region(Outbuf     *buf,
                                                  RGBA_Image *update,
                                                  int         x,
                                                  int         y,
                                                  int         w,
                                                  int         h);

void evas_software_gix_outbuf_flush(Outbuf *buf, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);

void evas_software_gix_outbuf_idle_flush(Outbuf *buf);

int evas_software_gix_outbuf_width_get(Outbuf *buf);

int evas_software_gix_outbuf_height_get(Outbuf *buf);

int evas_software_gix_outbuf_rot_get(Outbuf *buf);


#endif /* EVAS_ENGINE_H */

