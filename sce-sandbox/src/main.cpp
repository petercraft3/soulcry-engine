
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

struct {
   Display *dpy;
   int scn;
   GLXFBConfig *fbcfgs;
   XVisualInfo *vi;
   Colormap cmap;
   Window wnd;
   GLXContext xcontext;

   Atom deleteWindow;
} globalState;

int main()
{
   globalState.dpy = XOpenDisplay(NULL);
   if (!globalState.dpy) return 0;

   globalState.scn = XDefaultScreen(globalState.dpy);

   {
      int major, minor;
      glXQueryVersion(globalState.dpy, &major, &minor);
      if (major <= 1 && minor < 3)
      {
         printf("GLX %d.%d is supported (minimum 1.3).\n", major, minor);
         return 0;
      }
   }

   {
      int num_fbconfigs;
      int attribList[] = {
         GLX_RGBA,
         GLX_DOUBLEBUFFER,
         GLX_X_RENDERABLE, True,
         GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
         GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
         GLX_RED_SIZE, 8,
         GLX_GREEN_SIZE, 8,
         GLX_BLUE_SIZE, 8,
         GLX_ALPHA_SIZE, 8,
         GLX_DEPTH_SIZE, 24,
         GLX_STENCIL_SIZE, 8,
         GLX_SAMPLE_BUFFERS, 0,
         GLX_SAMPLES, 0,
         None
      };
      globalState.fbcfgs = glXChooseFBConfig(globalState.dpy, globalState.scn, attribList, &num_fbconfigs);
      if (!globalState.fbcfgs) return 0;

      globalState.vi = glXGetVisualFromFBConfig(globalState.dpy, globalState.fbcfgs[0]);
   }

   globalState.cmap = XCreateColormap(
      globalState.dpy,
      XRootWindow(globalState.dpy, globalState.scn),
      globalState.vi->visual,
      AllocNone
   );

   XSetWindowAttributes windowAttribs = { 0 };
   windowAttribs.border_pixel = XWhitePixel(globalState.dpy, globalState.scn);
   windowAttribs.background_pixel = XBlackPixel(globalState.dpy, globalState.scn);
   windowAttribs.colormap = globalState.cmap;
   windowAttribs.event_mask = ExposureMask | StructureNotifyMask;
   globalState.wnd = XCreateWindow(
      globalState.dpy,
      XRootWindow(globalState.dpy, globalState.scn),
      0, 0,
      853, 480,
      0,
      globalState.vi->depth,
      InputOutput,
      globalState.vi->visual,
      CWBorderPixel | CWBackPixel | CWColormap | CWEventMask,
      &windowAttribs
   );

   globalState.xcontext = glXCreateNewContext(
      globalState.dpy,
      globalState.fbcfgs[0],
      GLX_RGBA_TYPE,
      NULL,
      True
   );
   glXMakeCurrent(
      globalState.dpy,
      globalState.wnd,
      globalState.xcontext
   );

   globalState.deleteWindow = XInternAtom(globalState.dpy, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(globalState.dpy, globalState.wnd, &globalState.deleteWindow, 1);
   
   XClearWindow(globalState.dpy, globalState.wnd);
   XMapWindow(globalState.dpy, globalState.wnd);
   XStoreName(globalState.dpy, globalState.wnd, "sce_sandbox");

   while (true)
   {
      XEvent ev;
      XNextEvent(globalState.dpy, &ev);
      if (ev.type == ClientMessage) {
         if (ev.xclient.data.l[0] == (long int)globalState.deleteWindow) {
            break;
         }
      }
      else if (ev.type == DestroyNotify) { 
         break;
      }

      glClearColor(1, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      glXSwapBuffers(globalState.dpy, globalState.wnd);
   }

   glXDestroyContext(globalState.dpy, globalState.xcontext);
   XDestroyWindow(globalState.dpy, globalState.wnd);
   XFreeColormap(globalState.dpy, globalState.cmap);
   XFree(globalState.vi);
   XFree(globalState.fbcfgs);
   XCloseDisplay(globalState.dpy);

   printf("Exiting...\n");
   return 0;
}