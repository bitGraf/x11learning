#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
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

	// Open the window
	window = XCreateSimpleWindow(
		display, 
		RootWindowOfScreen(screen), 
		0, 0, 320, 200, 
		1, 
		BlackPixel(display, screenId), 
		WhitePixel(display, screenId));

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
	}

	// Cleanup
	XDestroyWindow(display, window);
	//XFree(screen);
	XCloseDisplay(display);
	return 1;
}