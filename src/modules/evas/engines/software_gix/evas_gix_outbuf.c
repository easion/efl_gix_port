#include "evas_common_private.h"
#include "evas_engine.h"
#include <sys/time.h>
#include <sys/utsname.h>


#include <eina/eina_list.h>

typedef struct _Outbuf_Region   Outbuf_Region;

struct _Outbuf_Region
{
   Gix_Output_Buffer *xob;
   int              x;
   int              y;
   int              w;
   int              h;
};

static Eina_List *shmpool = NULL;
gi_screen_info_t* gi_default_screen_info(void);


#define SHMPOOL_LOCK()
#define SHMPOOL_UNLOCK()

static Gix_Output_Buffer *
_find_xob(  gi_format_code_t format, int w, int h, void *data)
{
   return evas_software_gix_output_buffer_new(  format, w, h,  data);
}

static void
_unfind_xob(Gix_Output_Buffer *xob, int sync)
{
     evas_software_gix_output_buffer_free(xob);
}

static void
_clear_xob(int sync)
{
   SHMPOOL_LOCK();
   while (shmpool)
     {
	Gix_Output_Buffer *xob;

	xob = shmpool->data;
	shmpool = eina_list_remove_list(shmpool, shmpool);
	evas_software_gix_output_buffer_free(xob);
     }
   SHMPOOL_UNLOCK();
}


int
evas_software_gix_output_buffer_depth(Gix_Output_Buffer *xob)
{
   return xob->depth;
}


void
evas_software_gix_outbuf_idle_flush(Outbuf *buf)
{
   //if (buf->priv.onebuf)
     
   //else
     //{
	while (buf->priv.prev_pending_writes)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;

	     im = buf->priv.prev_pending_writes->data;
	     buf->priv.prev_pending_writes =
	       eina_list_remove_list(buf->priv.prev_pending_writes,
				     buf->priv.prev_pending_writes);
	     obr = im->extended_info;
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               {
                  evas_cache2_image_close(&im->cache_entry);
               }
             else
#endif
               evas_cache_image_drop(&im->cache_entry);
	     if (obr->xob) _unfind_xob(obr->xob, 0);

	     free(obr);
	  }

	_clear_xob(0);
     //}
}


void
evas_software_gix_outbuf_init(void)
{
}

void
evas_software_gix_outbuf_free(Outbuf *buf)
{
   if (!buf)
     return;

   while (buf->priv.pending_writes)
     {
	RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.pending_writes->data;
	buf->priv.pending_writes = eina_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	obr = im->extended_info;
	evas_cache_image_drop(&im->cache_entry);
	if (obr->xob)
          evas_software_gix_output_buffer_free(obr->xob);
/* 	if (obr->mxob) _unfind_xob(obr->mxob, 0); */
	free(obr);
     }
   evas_software_gix_outbuf_idle_flush(buf);
   evas_software_gix_outbuf_flush(buf, NULL, NULL, MODE_FULL);

   if (buf->priv.gix_platform.gc)
      gi_destroy_gc( buf->priv.gix_platform.gc); 

   //eina_array_flush(&buf->priv.onebuf_regions);
   free(buf);
   _clear_xob(0);

   //evas_software_gix_shutdown(buf);
}



void
evas_software_gix_outbuf_drawable_set(Outbuf *buf, gi_window_id_t draw)
{

   if (buf->priv.gix_platform.window == draw) return;
   if (buf->priv.gix_platform.gc)
     {
	gi_destroy_gc( buf->priv.gix_platform.gc);
	buf->priv.gix_platform.gc = NULL;
     }
   buf->priv.gix_platform.window = draw;
   buf->priv.gix_platform.gc = gi_create_gc( buf->priv.gix_platform.window, 0);
}


Outbuf *
evas_software_gix_outbuf_setup(int width, int height, int rot, 
				  gi_window_id_t draw, 
				  gi_format_code_t format,
				 int destination_alpha)
{
   /*Outbuf *buf;

   buf = (Outbuf *)calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   buf->rot = rotation;

   buf->priv.mask_dither = mask_dither;
   buf->priv.destination_alpha = destination_alpha;
   buf->priv.gix_platform.ogformat = format;
   buf->priv.destination_alpha = destination_alpha;

   if (!evas_software_gix_init(window, borderless, fullscreen, region, buf))
     {
        free(buf);
        return NULL;
     }

   {
      Gfx_Func_Convert  conv_func;
      Gix_Output_Buffer *xob;

      xob = evas_software_gix_output_buffer_new(buf->priv.gix_platform.gc, buf->priv.gix_platform.bitmap_info, 1, 1, NULL);

      conv_func = NULL;
      if (xob)
        {
           if ((rotation == 0) || (rotation == 180))
             conv_func = evas_common_convert_func_get(0,
                                                      width,
                                                      height,
                                                      32,
                                                      buf->priv.gix_platform.bitmap_info->masks[0],
                                                      buf->priv.gix_platform.bitmap_info->masks[1],
                                                      buf->priv.gix_platform.bitmap_info->masks[2],
                                                      PAL_MODE_NONE,
                                                      rotation);
           else if ((rotation == 90) || (rotation == 270))
             conv_func = evas_common_convert_func_get(0,
                                                      height,
                                                      width,
                                                      32,
                                                      buf->priv.gix_platform.bitmap_info->masks[0],
                                                      buf->priv.gix_platform.bitmap_info->masks[1],
                                                      buf->priv.gix_platform.bitmap_info->masks[2],
                                                      PAL_MODE_NONE,
                                                      rotation);

           evas_software_gix_output_buffer_free(xob);

           if (!conv_func)
             {
                ERR(".[ soft_gix engine Error ]."
                      " {"
                      "  At depth         32:"
                      "  RGB format mask: %08lx, %08lx, %08lx"
                      "  Not supported by and compiled in converters!"
                      " }",
                        buf->priv.gix_platform.bitmap_info->masks[0],
                        buf->priv.gix_platform.bitmap_info->masks[1],
                        buf->priv.gix_platform.bitmap_info->masks[2]);
             }
        }
   }*/
   Outbuf             *buf;
   gi_screen_info_t *vis;

   vis = gi_default_screen_info();
   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   //buf->depth = depth;
   //buf->depth = GI_RENDER_FORMAT_DEPTH(format);
   buf->rot = rot;
   buf->priv.gix_platform.ogformat = format;
   buf->priv.destination_alpha = destination_alpha;
   //eina_array_step_set(&buf->priv.onebuf_regions, sizeof (Eina_Array), 8);

   {
      Gfx_Func_Convert    conv_func;
      Gix_Output_Buffer    *xob;

      xob = evas_software_gix_output_buffer_new(  
		  buf->priv.gix_platform.ogformat, 1, 1,  NULL);
      conv_func = NULL;
      if (xob)
	{
#ifdef WORDS_BIGENDIAN
	     buf->priv.gix_platform.swap = 1;
	     buf->priv.gix_platform.bit_swap = 1;
#else
	     buf->priv.gix_platform.swap = 0;
	     buf->priv.gix_platform.bit_swap = 0;
#endif
	
	     {
		buf->priv.mask.r = (DATA32) vis->redmask;
		buf->priv.mask.g = (DATA32) vis->greenmask;
		buf->priv.mask.b = (DATA32) vis->bluemask;
		if (buf->priv.gix_platform.swap)
		  {
		     SWAP32(buf->priv.mask.r);
		     SWAP32(buf->priv.mask.g);
		     SWAP32(buf->priv.mask.b);
		  }
	     }
	   
	}
	
      evas_software_gix_outbuf_drawable_set(buf, draw);
   }

   return buf;
}

void
evas_software_gix_outbuf_reconfigure(Outbuf      *buf,
                                     int          width,
                                     int          height,
                                     int          rotation,
                                     Outbuf_Depth depth EINA_UNUSED)
{
   if ((width == buf->width) && (height == buf->height) &&
       (rotation == buf->rot))
     return;
   buf->width = width;
   buf->height = height;
   buf->rot = rotation;
   //evas_software_gix_bitmap_resize(buf);
   buf->priv.region_built = 0;
   evas_software_gix_outbuf_idle_flush(buf);
}

void *
evas_software_gix_outbuf_new_region_for_update(Outbuf *buf,
                                               int     x,
                                               int     y,
                                               int     w,
                                               int     h,
                                               int    *cx,
                                               int    *cy,
                                               int    *cw,
                                               int    *ch)
{
   RGBA_Image         *im;
   Outbuf_Region      *obr;
   int                 bpl = 0;
   int                 alpha;

   long dest_fmt = buf->priv.gix_platform.ogformat;

   
  

   obr = calloc(1, sizeof(Outbuf_Region));
   if (!obr) return NULL;
   obr->x = x;
   obr->y = y;
   obr->w = w;
   obr->h = h;
   *cx = 0;
   *cy = 0;
   *cw = w;
   *ch = h;

   alpha = ( (buf->priv.destination_alpha));

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
	  //dest_fmt = GI_RENDER_a8r8g8b8;

	  obr->xob = _find_xob(dest_fmt, w, h, NULL);
        if (!obr->xob)
          {
             free(obr);
             return NULL;
          }
		  
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get()){
          im = (RGBA_Image *)evas_cache2_image_data(evas_common_image_cache2_get(),
					w, h,
					(DATA32 *) evas_software_gix_output_buffer_data(obr->xob, &bpl),
					alpha, EVAS_COLORSPACE_ARGB8888);
        
		}else
#endif
		{
		
      im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
              w, h,
              (DATA32 *) evas_software_gix_output_buffer_data(obr->xob, &bpl),
              alpha, EVAS_COLORSPACE_ARGB8888);
		}
        if (!im)
          {
             _unfind_xob(obr->xob, 0);
             free(obr);
             return NULL;
          }
	//printf("%s: got line %d [%d,%d,%d,%d] bpl=%d,alpha=%d\n",
	//	__func__,__LINE__,x,y,w,h,bpl,alpha);
	im->extended_info = obr;
	
     }
   else
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          im = (RGBA_Image *)evas_cache2_image_empty(evas_common_image_cache2_get());
        else
#endif
        im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        if (!im)
          {
             free(obr);
             return NULL;
          }
        im->cache_entry.w = w;
        im->cache_entry.h = h;
        im->cache_entry.flags.alpha |= alpha ? 1 : 0;
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_surface_alloc(&im->cache_entry, w, h);
        else
#endif
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);
	im->extended_info = obr;
	if ((buf->rot == 0) || (buf->rot == 180))
          {
             obr->xob = _find_xob(dest_fmt,
                                  w, h, 
                                  NULL);
             if (!obr->xob)
               {
#ifdef EVAS_CSERVE2
                  if (evas_cserve2_use_get())
                    {
                       evas_cache2_image_close(&im->cache_entry);
                    }
                  else
#endif
                    evas_cache_image_drop(&im->cache_entry);
                  free(obr);
                  return NULL;
               }
          }
	else if ((buf->rot == 90) || (buf->rot == 270))
          {
             obr->xob = _find_xob( dest_fmt,
                                  h, w,                                  
                                  NULL);             
             if (!obr->xob)
               {
#ifdef EVAS_CSERVE2
                  if (evas_cserve2_use_get())
                    {
                       evas_cache2_image_close(&im->cache_entry);
                    }
                  else
#endif
                    evas_cache_image_drop(&im->cache_entry);
                  free(obr);
                  return NULL;
               }
     }
     }

   if ( (buf->priv.destination_alpha)&&
       (im->image.data))
     {
     memset(im->image.data, 0, w * h * sizeof(DATA32));
     }

   buf->priv.pending_writes = eina_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_gix_outbuf_push_updated_region(Outbuf     *buf,
                                             RGBA_Image *update,
                                             int         x,
                                             int         y,
                                             int         w,
                                             int         h)
{
   Gfx_Func_Convert    conv_func = NULL;
   Outbuf_Region      *obr;
   DATA32             *src_data;
   void               *data;
   int                 bpl = 0, yy;

   obr = update->extended_info;
   if (buf->priv.pal)
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_gix_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_gix_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_gix_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_gix_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
     }
   if (!conv_func) return;

   if (!obr->xob) return;
   data = evas_software_gix_output_buffer_data(obr->xob, &bpl);
   if (!data) return;
   src_data = update->image.data;
   if (!src_data) return;
   if (buf->rot == 0)
     {
	obr->x = x;
	obr->y = y;
     }
   else if (buf->rot == 90)
     {
	obr->x = y;
	obr->y = buf->width - x - w;
     }
   else if (buf->rot == 180)
     {
	obr->x = buf->width - x - w;
	obr->y = buf->height - y - h;
     }
   else if (buf->rot == 270)
     {
	obr->x = buf->height - y - h;
	obr->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
	obr->w = w;
	obr->h = h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
	obr->w = h;
	obr->h = w;
     }
   /*if (buf->onebuf)
     {
        src_data += x + (y * update->cache_entry.w);
        data += (bpl * obr->y) + 
           (obr->x * (evas_software_gix_output_buffer_depth(obr->xob) / 8));
     }*/
   if (buf->priv.pal)
     {
	if (data != (unsigned char *)src_data)
	  conv_func(src_data, data,
		    update->cache_entry.w - w,
		    bpl /
		    ((evas_software_gix_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y,
		    buf->priv.pal->lookup);
     }
   else
     {
	if (data != (unsigned char *)src_data)
	  conv_func(src_data, data,
		    update->cache_entry.w - w,
		    bpl /
		    ((evas_software_gix_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y, NULL);
     }
}

void
evas_software_gix_outbuf_flush(Outbuf *buf, Tilebuf_Rect *surface_damage EINA_UNUSED, Tilebuf_Rect *buffer_damage EINA_UNUSED, Evas_Render_Mode render_mode)
{
   Eina_List     *l;
   RGBA_Image    *im;
   Outbuf_Region *obr;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   /* copy safely the images that need to be drawn onto the back surface */
   EINA_LIST_FOREACH(buf->priv.pending_writes, l, im)
     {
	Gix_Output_Buffer *xob;

        obr = im->extended_info;
        xob = obr->xob;
		evas_software_gix_output_buffer_paste(obr->xob, 
			 buf->priv.gix_platform.window,
						     buf->priv.gix_platform.gc,
						     obr->x, obr->y);	     
       // evas_software_gix_output_buffer_paste(xob,
         //                                     obr->x,
           //                                   obr->y);
/*         if (obr->mgdiob) */
/*           evas_software_gix_output_buffer_paste(obr->mgdiob, */
/*                                                 buf->priv.x11.xlib.mask, */
/*                                                 buf->priv.x11.xlib.gcm, */
/*                                                 obr->x, obr->y, 0); */
     }

   while (buf->priv.prev_pending_writes)
     {
        im = buf->priv.prev_pending_writes->data;
        buf->priv.prev_pending_writes =
          eina_list_remove_list(buf->priv.prev_pending_writes,
                                buf->priv.prev_pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop(&im->cache_entry);
        if (obr->xob)
          evas_software_gix_output_buffer_free(obr->xob);
/*         if (obr->mgdiob) _unfind_gixob(obr->mgdiob); */
        free(obr);
     }
   buf->priv.prev_pending_writes = buf->priv.pending_writes;
   buf->priv.pending_writes = NULL;

   evas_common_cpu_end_opt();
}

#if 0
void
evas_software_gix_outbuf_idle_flush(Outbuf *buf)
{
   while (buf->priv.prev_pending_writes)
     {
        RGBA_Image *im;
        Outbuf_Region *obr;

        im = buf->priv.prev_pending_writes->data;
        buf->priv.prev_pending_writes =
          eina_list_remove_list(buf->priv.prev_pending_writes,
                                buf->priv.prev_pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop((Image_Entry *)im);
        if (obr->xob)
          evas_software_gix_output_buffer_free(obr->xob);
/*         if (obr->mxob) _unfind_xob(obr->mxob, 0); */
        free(obr);
     }
}
#endif

int
evas_software_gix_outbuf_width_get(Outbuf *buf)
{
   return buf->width;
}

int
evas_software_gix_outbuf_height_get(Outbuf *buf)
{
   return buf->height;
}

int
evas_software_gix_outbuf_rot_get(Outbuf *buf)
{
   return buf->rot;
}

/////////////////


