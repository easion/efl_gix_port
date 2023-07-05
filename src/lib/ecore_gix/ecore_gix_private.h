#ifndef _ECORE_GIX_PRIVATE_H
#define _ECORE_GIX_PRIVATE_H
/* eina_log related things */

#ifdef __GNUC__
# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define EVIL_UNUSED __attribute__ ((__unused__))
# else
#  define EVIL_UNUSED
# endif
#define __UNUSED__ EVIL_UNUSED
#endif


extern int _ecore_gix_log_dom;

#ifdef ECORE_GIX_DEFAULT_LOG_COLOR
#undef ECORE_GIX_DEFAULT_LOG_COLOR
#endif /* ifdef ECORE_GIX_DEFAULT_LOG_COLOR */
#define ECORE_GIX_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_gix_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_gix_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...) EINA_LOG_DOM_INFO(_ecore_gix_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_gix_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif /* ifdef CRIT */
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_gix_log_dom, __VA_ARGS__)



struct keymap
{
   char *name;
   char *string;
};

#ifdef LOGFNS
# include <stdio.h>
# define LOGFN(fl, ln, fn) \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
#else
# define LOGFN(fl, ln, fn)
#endif

#define _ecore_x_disp ((void*)1)


#endif /* ifndef _ECORE_GIX_PRIVATE_H */
