#include "holyh/src/holy.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

/* This struct contains all of the properties for a window - borrowed from HAZE. */
struct hzwinprop {
	SDL_Window *window; /* The SDL window. Pain in the ass to access, so we just have a reference here */
	SDL_GLContext glcontext; /* The GL context. We don't use this much, but it's good to have a ref to it. */
	U32 winflags; /* The flags we gave the window. */
	INAT width; /* The window width and height. They're useful things to know. */
	INAT height;
	U1 quit; /* is the window in a quitting state? (e.g did the user click close) */
	U1 fullscreen; /* Is the window fullscreen or not? Not used here, but used in HAZE. */
};

/* We create a global declaration of a window struct to use elsewhere in the program. This is our primary window. */
struct hzwinprop primarywin;

/* This is a cleanup step, which destroys the primary SDL window and quits SDL. */
X0 cleanup()
{
	if(SDL_WasInit(SDL_INIT_VIDEO)) {
		/* Basically just exits fullscreen if it was enabled and frees the mouse if it was grabbed. */
		SDL_ShowCursor(SDL_TRUE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		if(primarywin.window) SDL_SetWindowGrab(primarywin.window, SDL_FALSE);
		#ifdef __APPLE__
		if(primarywin.window) SDL_SetWindowFullscreen(screen, 0);
		#endif

		/* Destroy the primary window */
		if(primarywin.window) SDL_DestroyWindow(primarywin.window);
	}

	/* Quit SDL (Does not quit the whole program, just presumably gets SDL to clean up) */
	SDL_Quit();
}

/* This function prints an error to both the terminal and an SDL window. Can be called at any point.
 * Arguments are the same as printf();
 */
X0 errwindow(const CHR *s, ...)
{
	#ifndef HZ_MAX_ERROR_LENGTH
	#define HZ_MAX_ERROR_LENGTH 4096
	#endif

	/* We create a buffer to store the final error message in, with a maximum length of 4096 characters.
	 * That's just over 2 whole Discord messages worth of error!
	 */
	CHR buffer[HZ_MAX_ERROR_LENGTH];

	/* This stuff is just fancy variadic argument stuff. 
	 * See... uh.. this, maybe? https://www.thegeekstuff.com/2017/05/c-variadic-functions/
	 */
	va_list args;
	va_start(args, s);

	/* This uses the vsnprintf function to replicate printf's functionality without actually printing anything.
	 * If vsnprintf failed for some reason, it will return a number below 0. If it does, we create a new error.
	 */
	if (vsnprintf(buffer, HZ_MAX_ERROR_LENGTH, s, args) < 0)
		strcpy(buffer,
			"errwindow() was unable to format the fatal exception message while handling an exception.\0");
	/* We then print the fully formatted error to the stderr output.
	 * This is just in case the user is unable to read the SDL error window.
	 */
	fprintf(stderr, "FATAL ERROR: %s\n", buffer);

	/* Call the global cleanup function to ensure everything is.. well, clean. */
	cleanup();

	/* Show an SDL message box in case the user cannot read the terminal.
	 * ShowSimpleMessageBox will work even after you've called SDL_Quit or before you've called SDL_Init.
	 * It's especially designed for situations like this.
	 */
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Exception", buffer, NULL);
	
	va_end(args);

	/* Return the failure exit code and terminate (code 1 (failure), aka not 0, which is success) */
	exit(EXIT_FAILURE);
}

/* The main function. This is always the function that is automatically called first, so this
 * is our engine's "entrypoint"
 */
INAT main(INAT argc, CHR *argv[]) /* Remember, argc is the number of arguments, argv is the array of arguments */
{
	/* Initialize SDL. If this fails, we can probably determine that the user does not have a
	 * [supported] graphical backend. */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		/* This might look stupid, but keep in mind that errwindow() does a printf() as a fallback too */
		errwindow("Unable to initialize video!\n SDL Error: %s", SDL_GetError());
	}	

	/* Set the window flags and OpenGL version
	 * This tells SDL what features we want. 
	 * SDL_WINDOW_OPENGL - Tells SDL we want to use OpenGL in our window
	 * SDL_WINDOW_REISIZABLE - Tells SDL we want the user to be able to resize the window at will
	 * SDL_WINDOW_SHOWN - Tells SDL we want the window to be visible on launch
	 * Full list of flags: https://wiki.libsdl.org/SDL_WindowFlags
	 */
	primarywin.winflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;

	/* Tell SDL we want to use OpenGL major version 3 minor version 3 (OpenGL 3.3), with the core profile
	 * See the bottom of this template for a link to a place you can learn about what that means.
	 * There are some big differences between OpenGL 3.3 and previous versions.
	 * We need to do this before we create the window or the GL context.
	 */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	/* Finally, actually create the window. We give it a title, a starting position, a width, a height, and our
	 * previously defined flags. If the window cannot be created, we display the error SDL gave us.
	 */
	if (!(primarywin.window = SDL_CreateWindow(
		"OpenGL 3.3 + SDL Template",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 480,
		primarywin.winflags)))
		errwindow("Unable to create the primary window!\n SDL Error: %s", SDL_GetError());

	/* Create the OpenGL context. If this fails, the user cannot use OpenGL [probably]. Still, we print the SDL
	 * error too just in case.
	 *
	 * Since OpenGL is one big state machine, you need a context to be able to keep track of all the states.
	 * This context is bound to our primary window. When the user looks at the window, they'll be looking at the
	 * OpenGL context we created here.
	 */
	if (!(primarywin.glcontext = SDL_GL_CreateContext(primarywin.window)))
		errwindow("Unable to create GL context! Does your device support OpenGL?\n"
			"Are you sure you're using the very latest versions of your graphics drivers?\n"
			"You might be able to resolve this by using Mesa software rendering.\n\n"
			"SDL Error: %s", SDL_GetError());

	/* Initialize GLEW */
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK) errwindow("Error initializing GLEW! %s\n", glewGetErrorString(glewError));

	/* This makes our buffer swap syncronized with the monitor's vertical refresh. In other words, V-Sync.
	 * You'll see this in action a bit later.
	 */
	SDL_GL_SetSwapInterval(1);

	/* Specifies clear values for the colour buffers. We want the whole colour buffer to be magenta, so
	 * we set the colour buffer's clear value to magenta.
	 */
	glClearColor(1.f, 0.f, 1.f, 0.f);

	/* The main loop. This renders every single frame, so when one frame is done, the loop starts again. */
	while (!primarywin.quit) {
		/* Poll SDL for events. If SDL has no events for us to collect, continue rendering instead. */
		SDL_Event Event;
		while (SDL_PollEvent(&Event)) {
			/* Check the event type. This could be many things, e.g a mouse movement or a key press. */
			switch (Event.type) {
			/* This event is triggered when SDL thinks we need to quit, e.g when you
			 * click the close button on the window */
			case SDL_QUIT:
				/* If we do need to quit, we set that as a window property, so next time we're about
				 * to re-enter the main loop, it simply decides not to loop again.
				 * Hence this condition: `while (!primarywin.quit) {`
				 */
				primarywin.quit = true;
				break;
			default:
				/* If the event is anything else, we simply ignore it.
				 * You can implement your own events above this. Here's the full list:
				 * https://wiki.libsdl.org/SDL_EventType
				 */
				break;
			}
		}

		/* Check if the window size has changed and record it in the primarywin properties for use elsewhere
		 * (Not strictly neccessary in this template, but it's used in HAZE)
		 */
		SDL_GetWindowSize(primarywin.window, &primarywin.width, &primarywin.height);

		/* Then we clear the colour buffer, making everything magenta. */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Swap our buffer to display the current contents of buffer on screen.
		 * Since we set SDL_GL_SetSwapInterval(1), this will use the monitor's refresh rate.
		 */
		SDL_GL_SwapWindow(primarywin.window);
	}

	/* Cleanup before exit, just in case. */
	cleanup();

	/* Nothing bad happened (we think), so return the success code and bugger off. */
	return EXIT_SUCCESS;
}

/* You can learn more things about SDL2 here: https://wiki.libsdl.org/FrontPage 
 * And I think you might be able to get some knowledge about OpenGL here: https://learnopengl.com/
 * That guy uses GLFW, GLAD and a bunch of other weird things. We're using SDL2, so you should just ignore all of that,
 * SDL2 does it all for us. The only things you need to look at are the OpenGL function calls, in other words, the
 * things beginning with `gl`.
 *
 * See if you can complete the Hello Triangle task (https://learnopengl.com/Getting-started/Hello-Triangle) using this
 * template, by adding code just before `while (!primarywin.quit) {` and replacing the `glClearColor`/`glClear` bits.
 * Those should be the only sections you need to change - just before the main loop, and just inside the main loop.
 */