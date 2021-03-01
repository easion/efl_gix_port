#include <string.h>

#include "evas_common_private.h"
#include "evas_engine.h"

Gix_Output_Buffer *
evas_software_gix_output_buffer_new(gi_format_code_t format,
                                    int             w,
                                    int             h,
                                    void           *data)
{
   Gix_Output_Buffer *xob;
   int pitch = 0;
   void *pdata = NULL;

   xob = calloc(1, sizeof(Gix_Output_Buffer));
   if (!xob) return NULL;

   xob->xim = NULL;
   xob->w = w;
   xob->h = h;


   if (format == GI_RENDER_a8 && data)  {
	   pdata = bitmap_create_from_data(data, w,h,FALSE,&pitch);
	   xob->need_free = TRUE;
		xob->xim = gi_create_image_with_data( w, h,pdata, format);	   
   }
  else if (data){
   pdata = data;
   xob->need_free = FALSE;
   xob->xim = gi_create_image_with_data( w, h,pdata, format);
  }
  else{
   xob->need_free = TRUE;
   xob->xim = gi_create_image_depth( w, h, format);
   if (xob->xim)
   {
	   pdata = xob->xim->rgba;
	   memset(pdata,0x00,xob->xim->pitch * h); //RGBA±ØÐëÈç´Ë
   }
  }


   if (!xob->xim)
     {
	free(xob);
	return NULL;
     }

   xob->data = pdata; 
   xob->pitch = xob->xim->pitch;
   xob->psize = xob->pitch * xob->h;  
   xob->depth = GI_RENDER_FORMAT_DEPTH(format);
   return xob;
}

void
evas_software_gix_output_buffer_free(Gix_Output_Buffer *xob)
{
  if (xob->data && !xob->need_free){
	  xob->xim->rgba = NULL; //free?
  }
  gi_destroy_image(xob->xim);
  free(xob);
}

void
evas_software_gix_output_buffer_paste(Gix_Output_Buffer *xob,
	gi_window_id_t d,
	gi_gc_ptr_t gc,
	int	x,
	int	y)
{
  int err;

  gi_gc_attch_window(gc,d);


  //err = gi_bitblt_image(gc,0, 0,xob->w, xob->h, xob->xim,x,y );
  err = gi_draw_image(gc, xob->xim,x,y );

#if 1
  gi_image_t *img = xob->xim;

  printf("IMAGE %p: %dx%d, pitch %d,Format 0x%x\n", img,
		img->w, img->h,img->pitch, img->format);
  printf("PASTE %p: %dx%d pos%d,%d \n", xob,
		xob->w, xob->h,x,y);
	
  gi_image_save_as_png(xob->xim,"/evas.png");
#endif	
  //sleep(1);

  if (err) {
    /*printf("%s: buffer paste error (size %dx%d pos %d,%d) %s\n",__FUNCTION__, 
		xob->w, xob->h,x,y, strerror(errno));
		*/
  }
}

DATA8 *
evas_software_gix_output_buffer_data(Gix_Output_Buffer *xob,
                                     int               *bytes_per_line_ret)
{
   //if (bytes_per_line_ret) *bytes_per_line_ret = xob->pitch;
   //return xob->data;

	if (!xob->xim)
	{
		*bytes_per_line_ret = 0;
		return NULL;
	}
   if (bytes_per_line_ret) 
	   *bytes_per_line_ret = xob->xim->pitch;
   return (DATA8 *)xob->xim->rgba;
}

