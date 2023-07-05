#ifndef __EVAS_ENGINE_SOFTWARE_GDI_H__
#define __EVAS_ENGINE_SOFTWARE_GDI_H__


#include <gi/gi.h>

typedef struct _Evas_Engine_Info_Software_Gix Evas_Engine_Info_Software_Gix;

struct _Evas_Engine_Info_Software_Gix
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      unsigned int         drawable;
	  unsigned int                          mask;
      gi_format_code_t                      gformat;

	  unsigned int                          debug              : 1;
      unsigned int                          destination_alpha  : 1;

      int          rotation;
      unsigned int borderless : 1;
      unsigned int fullscreen : 1;
      unsigned int region     : 1;
   } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};


#endif /* __EVAS_ENGINE_SOFTWARE_GDI_H__ */

