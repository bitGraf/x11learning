#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char** argv) {
    Display* display;
    Window window;
    Screen* screen;
    int screenId;
    XEvent ev;

    // Open the display
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("%s\n", "Could not open display");
        return 1;
    }
    screen = DefaultScreenOfDisplay(display);
    screenId = DefaultScreen(display);

    // Check GLX version
    GLint majorGLX, minorGLX = 0;
    glXQueryVersion(display, &majorGLX, &minorGLX);
    if (majorGLX <= 1 && minorGLX < 2) {
        printf("GLX 1.2 or greater is required.\n");
        XCloseDisplay(display);
        return 1;
    } else {
        printf("GLX version: %d.%d\n", majorGLX, minorGLX);
    }

    GLint glxAttribs[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   8,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES,        0,
        None
    };
    XVisualInfo* visual = glXChooseVisual(display, screenId, glxAttribs);

    if (visual == 0) {
        printf("Could not create correct visual window.\n");
        XCloseDisplay(display);
        return 1;
    }

    // Open the window
    XSetWindowAttributes windowAttribs;
    windowAttribs.border_pixel = BlackPixel(display, screenId);
    windowAttribs.background_pixel = WhitePixel(display, screenId);
    windowAttribs.override_redirect = True;
    windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
    windowAttribs.event_mask = ExposureMask;
    window = XCreateWindow(
        display, 
        RootWindow(display, screenId), 
        0, 0, 340, 200, 0, 
        visual->depth, InputOutput, 
        visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, 
        &windowAttribs);

    // Create GLX OpenGL context
    GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
    glXMakeCurrent(display, window, context);

    printf("GL Vendor: %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("GL Version: %s\n", glGetString(GL_VERSION));
    printf("GL Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // subscribe to certain events
    XSelectInput(display, window, 
        KeyPressMask | KeyReleaseMask | KeymapStateMask | 
        PointerMotionMask | 
        ButtonPressMask | ButtonReleaseMask | 
        EnterWindowMask | LeaveWindowMask |
        StructureNotifyMask);

    // Set window title
    XStoreName(display, window, "hehe on Linux xD");

    // Show the window
    XClearWindow(display, window);
    XMapRaised(display, window);

    // Set GL Sample stuff
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    // get window attributes
    XWindowAttributes attribs;
    XGetWindowAttributes(display, window, &attribs);
    printf("Initial config:\n  Window width:  %d\n         height: %d\n", attribs.width, attribs.height);

    // Resize window
    unsigned int change_values = CWWidth | CWHeight;
    XWindowChanges values;
    values.width = 800;
    values.height = 600;
    XConfigureWindow(display, window, change_values, &values);

    char str[25] = {0};
    KeySym keysym = 0;
    int len = 0;
    bool running = true;
    int x, y;
    int width = 0, height = 0;
    
    // Enter message loop
    while (running) {
        XNextEvent(display, &ev);
        
        switch(ev.type) {
            case ConfigureNotify: {
                if ((width != ev.xconfigure.width) || (height != ev.xconfigure.height)) {
                    width  = ev.xconfigure.width;
                    height = ev.xconfigure.height;

                    printf("Window width:  %d\n       height: %d\n", width, height);
                    glViewport(0, 0, width, height);
                }
            } break;

            case KeymapNotify:
                XRefreshKeyboardMapping(&ev.xmapping);
                break;

            case MotionNotify:
                x = ev.xmotion.x;
                y = ev.xmotion.y;
                //printf("Mouse: <%3d, %3d>\n", x, y);
                break;

            case EnterNotify:
                printf("Mouse Enter\n");
                break;

            case LeaveNotify:
                printf("Mouse Leave\n");
                break;

            case ButtonPress:
            case ButtonRelease: {
                unsigned int button = ev.xbutton.button;
                // translate button to my mouse button mapping!
                bool pressed = (ev.type == ButtonPress);

                printf("Button %s: %i\n", pressed ? "pressed" : "released", button);
            } break;

            case KeyPress:
            case KeyRelease: {
                keysym = XLookupKeysym(&ev.xkey, 0);
                // translate keysym to my key mapping!
                bool pressed = (ev.type == KeyPress);
                
                printf("Key %s: %ld\n", pressed ? "pressed" : "released", keysym);
                if (pressed && keysym == XK_Escape) {
                    running = false;
                }
            } break;
        }

        // OpenGL Rendering
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
            glColor3f(  1.0f,  0.0f, 0.0f);
            glVertex3f( 0.0f, -1.0f, 0.0f);
            glColor3f(  0.0f,  1.0f, 0.0f);
            glVertex3f(-1.0f,  1.0f, 0.0f);
            glColor3f(  0.0f,  0.0f, 1.0f);
            glVertex3f( 1.0f,  1.0f, 0.0f);
        glEnd();

        // Present frame
        glXSwapBuffers(display, window);
    }

    // Cleanup GLX
    glXDestroyContext(display, context);

    // Cleanup X11
    XFree(visual);
    XFreeColormap(display, windowAttribs.colormap);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    
    return 1;
}