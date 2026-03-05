#ifndef APP_H__
#define APP_H__

// EGL is only available on Android and (optionally) iOS.
// Linux, WASM, Win32, and RPI handle buffer swapping at the AppPlatform layer.
#if defined(ANDROID)
#define APP_USE_EGL
#endif
#ifdef STANDALONE_SERVER
#undef APP_USE_EGL
#endif

#include "AppPlatform.h"
#ifdef APP_USE_EGL
    #include <EGL/egl.h>
#endif
#include "platform/log.h"

typedef struct AppContext {
#ifdef APP_USE_EGL
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
#endif
	AppPlatform* platform;
	bool doRender;
} AppContext;


class App
{
public:
    App()
	:	_finished(false),
		_inited(false)
	{
		_context.platform = 0;
	}
	virtual ~App() {}

	void init(AppContext& c) {
        _context = c;
		init();
		_inited = true;
    }
	bool isInited() { return _inited; }

	virtual AppPlatform* platform() { return _context.platform; }

	void onGraphicsReset(AppContext& c) {
		_context = c;
		onGraphicsReset();
	}

    virtual void audioEngineOn () {}
    virtual void audioEngineOff() {}
    
	virtual void destroy() {}

    virtual void loadState(void* state, int stateSize) {}
    virtual bool saveState(void** state, int* stateSize) { return false; }
    
	void swapBuffers() {
#ifdef APP_USE_EGL
		if (_context.doRender) {
			eglSwapBuffers(_context.display, _context.surface);
		}
#elif defined(WIN32)
		if (_context.doRender) {
			// Win32 WGL path — display holds the HDC
			::SwapBuffers(reinterpret_cast<HDC>(_context.display));
		}
#endif
		// Linux / WASM / RPI / iOS: buffer swap is owned by AppPlatform
		// (e.g. SDL_GL_SwapWindow), not by App directly.
	}

	virtual void draw() {}
	virtual void update() {};// = 0;
	virtual void setSize(int width, int height) {}
	
	virtual void quit() { _finished = true; }
	virtual bool wantToQuit() { return _finished; }
	virtual bool handleBack(bool isDown) { return false; }

protected:
	virtual void init() {}
	//virtual void onGraphicsLost() = 0;
	virtual void onGraphicsReset() = 0;

private:
	bool _inited;
	bool _finished;
    AppContext _context;
};

#endif//APP_H__
