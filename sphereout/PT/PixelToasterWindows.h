// Windows Platform
// Copyright � 2006-2007 Glenn Fiedler
// Part of the PixelToaster Framebuffer Library - http://www.pixeltoaster.com

#define VC_EXTRALEAN 
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <windowsx.h>

#include <d3d9.h>

#ifndef __MINGW32__ 
#include <d3dx9.h> 
#endif

#ifdef _MSC_VER
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "gdi32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "user32.lib" )
#endif

namespace PixelToaster
{
	// Windows Adapter interface

	class WindowsAdapter
	{
	public:

		virtual bool paint() = 0;				// paint pixels to display
		virtual bool fullscreen() = 0;			// switch to fullscreen output
		virtual bool windowed() = 0;			// switch to windowed output
		virtual void toggle() = 0;				// toggle fullscreen/windowed output
		virtual void exit() = 0;				// user wants to exit (pressed escape or alt-f4 and there is no listener)
	};

	// Window implementation

	class WindowsWindow
	{
	public:

		WindowsWindow( DisplayInterface * display, WindowsAdapter * adapter, const char title[], int width, int height )
		{
			assert( display );
			assert( adapter );

			// save display
			this->display = display;		// note: required for listener callbacks

			#ifdef UNICODE
			// convert title to unicode
			wchar_t unicodeTitle[1024];
			MultiByteToWideChar( CP_ACP, 0, title, -1, unicodeTitle, sizeof(unicodeTitle) );
			#endif

			// setup data

			this->adapter = adapter;
			this->width = width;
			this->height = height;

			// defaults

			window = NULL;
			systemMenu = NULL;
			active = false;
			_listener = NULL;
			centered = false;
			zoomLevel = ZOOM_ORIGINAL;
			mode = Windowed;

			// get handle to system arrow cursor

			arrowCursor = LoadCursor( NULL, IDC_ARROW );

			// create null cursor so we can hide it reliably

			integer32 cursorAnd = 0xFFFFFFFF;
			integer32 cursorXor = 0;

			nullCursor = CreateCursor( NULL, 0, 0, 1, 1, &cursorAnd, &cursorXor );

			// clear mouse data

			mouse.x = 0;
			mouse.y = 0;
			mouse.buttons.left = false;
			mouse.buttons.middle = false;
			mouse.buttons.right = false;

			// setup keyboard data

			for ( int i = 0; i < 256 ; ++i ) 
			{
				translate[i] = (Key::Code) i;
				down[i] = false;
			}

			translate[219] = Key::OpenBracket;
			translate[221] = Key::CloseBracket;
			translate[220] = Key::BackSlash;
			translate[13]  = Key::Enter;
			translate[187] = Key::Equals;
			translate[189] = Key::Separator;
			translate[186] = Key::SemiColon;
			translate[191] = Key::Slash;
			translate[190] = Key::Period;
			translate[188] = Key::Comma;
			translate[45]  = Key::Insert;
			translate[46]  = Key::Delete;

			// setup window class

			HINSTANCE instance = GetModuleHandle(0);

			WNDCLASSEX windowClass;
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = 0;
			windowClass.lpfnWndProc = &StaticWindowProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = instance;
			windowClass.hIcon = LoadIcon( instance, TEXT("DisplayIcon") );
			windowClass.hCursor = NULL;
			windowClass.hbrBackground = NULL;
			windowClass.lpszMenuName = NULL;
			windowClass.hIconSm = NULL;
			#ifdef UNICODE
			windowClass.lpszClassName = unicodeTitle;
			#else
			windowClass.lpszClassName = title;
			#endif

			#ifdef UNICODE
			UnregisterClass( unicodeTitle, instance );
			#else
			UnregisterClass( title, instance );
			#endif

			if ( !RegisterClassEx( &windowClass ) ) 
				return;

			// create window

			#ifdef UNICODE
			window = CreateWindow( unicodeTitle, unicodeTitle, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, instance, NULL );
			#else
			window = CreateWindow( title, title, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, instance, NULL );
			#endif

			if ( !window )
				return;

			#ifdef _MSC_VER
				#pragma warning( push )
				#pragma warning( disable : 4244 )		// conversion from 'LONG_PTR' to 'LONG', possible loss of data (stupid win32 api! =p)
			#endif

			SetWindowLongPtr( window, GWLP_USERDATA, (LONG_PTR) this );

			#ifdef _MSC_VER
				#pragma warning( pop )
			#endif

			// setup system menu

			updateSystemMenu();
		}

		~WindowsWindow()
		{
			DestroyWindow( window );
			window = NULL;

			DestroyCursor( nullCursor );
		}

		// show the window (it is initially hidden)

		void show()
		{
			ShowWindow( window, SW_SHOW );
		}
		
		// hide the window

		void hide()
		{
			ShowWindow( window, SW_HIDE );
		}

		// check if window is visible?

		bool visible() const
		{
			return IsWindowVisible( window ) != 0;
		}

		// put window in fullscreen mode

		void fullscreen(int width, int height)
		{
			// 1. hide window
			// 2. hide mouse cursor
			// 3. popup window style
			// 4. move window to cover display entirely
			// 5. show window
			// 6. update system menu

			this->width = width;
			this->height = height;
			
			hide();

			SetCursor( nullCursor );

			SetWindowLongPtr( window, GWL_STYLE, WS_POPUPWINDOW );	

			int w = GetSystemMetrics( SM_CXSCREEN );
			int h = GetSystemMetrics( SM_CYSCREEN );

			if ( width > w )
				w = width;

			if ( height > h )
				h = height;

			SetWindowPos( window, 0, 0, 0, w, h, SWP_NOZORDER );

			show();

			mode = Fullscreen;
			updateSystemMenu();
		}

		// put window in windowed mode

		void windowed( int width, int height )
		{
			// 1. hide window
			// 2. overlapped window style
			// 3. adjust window rect
			// 4. center window
			// 5. show mouse cursor
			// 6. update system menu

			this->width = width;
			this->height = height;
			
			hide();

			SetWindowLongPtr( window, GWL_STYLE, WS_OVERLAPPEDWINDOW );

			RECT rect;
			GetWindowRect( window, &rect );
			AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, 0 );
			SetWindowPos( window, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER );

			center();

			SetCursor( arrowCursor );

			mode = Windowed;
			updateSystemMenu();
		}

		// center the window on the desktop
		// note: has no effect if the window is minimized or maximized

		void center()
		{
			RECT rect;
			GetWindowRect( window, &rect );
			
			const int width = rect.right - rect.left;
			const int height = rect.bottom - rect.top;
			
			int x = ( GetSystemMetrics(SM_CXSCREEN) - width ) >> 1;
			int y = ( GetSystemMetrics(SM_CYSCREEN) - height ) >> 1;
			
			if ( x < 0 )
				x = 0;

			if ( y < 0 )
				y = 0;
		
			SetWindowPos( window, 0, x, y, width, height, SWP_NOZORDER );

			centered = true;

			updateSystemMenu();
		}

		// zoom window

		void zoom( float scale )
		{
			// get current window rect and calculate current window center

			RECT rect;
			GetWindowRect( window, &rect );

			const int cx = ( rect.left + rect.right ) / 2;
			const int cy = ( rect.top + rect.bottom ) / 2;

			// calculate window rect with origin (0,0)

			rect.left = 0;
			rect.top = 0;
			rect.right = rect.left + (int) ( width * scale );
			rect.bottom = rect.top + (int) ( height * scale );

			// adjust window rect then make adjust origin back to (0,0)

			AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, 0 );

			if ( rect.left < 0 )
			{
				rect.right -= rect.left;
				rect.left = 0;
			}

			if ( rect.top < 0 )
			{
				rect.bottom -= rect.top;
				rect.top = 0;
			}

			// center zoomed window around previous window center

			const int dx = cx - ( rect.right - rect.left ) / 2;
			const int dy = cy - ( rect.bottom - rect.top ) / 2;

			rect.left += dx;
			rect.right += dx;
			rect.top += dy;
			rect.bottom += dy;

			// check that the newly centered window position is origin (0,0) or larger. no negative origin values allowed

			if ( rect.left < 0 )
			{
				rect.right -= rect.left;
				rect.left = 0;
			}

			if ( rect.top < 0 )
			{
				rect.bottom -= rect.top;
				rect.top = 0;
			}

			// finally set the zoomed window position

			SetWindowPos( window, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER );
			
			// detect what zoom level we are at and update system menu

			if ( scale == 1.0f )
				zoomLevel = ZOOM_ORIGINAL;
			else if ( scale == 2.0f )
				zoomLevel = ZOOM_2X;
			else if ( scale == 4.0f )
				zoomLevel = ZOOM_4X;
			else if ( scale == 8.0f )
				zoomLevel = ZOOM_8X;
			else
				zoomLevel = ZOOM_RESIZED;
		}

		// window update pumps the message queue

		void update()
		{
			// hide mouse cursor if fullscreen

			if ( mode == Fullscreen && active )
				SetCursor( nullCursor );

			// check window

			if ( !window )
				return;

			// message pump

			MSG message;

			bool done = false;

			while ( !done )
			{
				if ( PeekMessage( &message, window, 0, 0, PM_REMOVE ) ) 
				{
					TranslateMessage( &message );
					DispatchMessage( &message );
				}
				else
				{
					done = true;
				}

				Sleep(0);
			}
		}

		// get the window handle.
		// null if the window failed to initialize.

		HWND handle() const
		{
			return window;
		}

		// listener management

		void listener( Listener * listener )
		{
			_listener = listener;
		}

		Listener * listener()
		{
			return _listener;
		}

	protected:

		static LRESULT CALLBACK StaticWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			LONG_PTR extra = GetWindowLongPtr( hWnd, GWLP_USERDATA );

			if (!extra) 
				return DefWindowProc( hWnd, uMsg, wParam, lParam );

			WindowsWindow * window = (WindowsWindow*) extra;

			return window->WindowProc( hWnd, uMsg, wParam, lParam );
		}

		LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch ( uMsg )
			{
				case WM_ACTIVATE:
					active = wParam != WA_INACTIVE;
					if ( _listener )
						_listener->onActivate( display->wrapper() ? *display->wrapper() : *display, active );
					break;

				case WM_PAINT:
					adapter->paint();
					break;

				case WM_SIZING:
					adapter->paint();
					zoomLevel = ZOOM_RESIZED;
				case WM_SIZE:
					updateSystemMenu();
					break;

				case WM_QUIT:
				case WM_CLOSE:
					if ( _listener )
						if ( _listener->onClose( display->wrapper() ? *display->wrapper() : *display ) )
							adapter->exit();
					else
						adapter->exit();
					break;

				case WM_SETCURSOR:
					if ( LOWORD( lParam ) == HTCLIENT )
					{
						if ( mode == Fullscreen )
							SetCursor( nullCursor );
						else
							SetCursor( arrowCursor );
					}
					else
						return DefWindowProc( hWnd, uMsg, wParam, lParam );
					break;

				case WM_SYSCOMMAND:
				{
					switch ( wParam )
					{
						case MENU_CENTER:
							center();
							updateSystemMenu();
							break;

						case MENU_ZOOM_ORIGINAL:
							zoom( 1.0f );
							updateSystemMenu();
							break;

						case MENU_ZOOM_2X:
							zoom( 2.0f );
							updateSystemMenu();
							break;

						case MENU_ZOOM_4X:
							zoom( 4.0f );
							updateSystemMenu();
							break;

						case MENU_ZOOM_8X:
							zoom( 8.0f );
							updateSystemMenu();
							break;

						case MENU_WINDOWED:
							adapter->windowed();
							break;

						case MENU_FULLSCREEN:
							adapter->fullscreen();
							break;

						default:
							return DefWindowProc( hWnd, uMsg, wParam, lParam );
					}
				}
				break;

				case WM_MOVING:
				{
					if ( centered )
					{
						centered = false;
						updateSystemMenu();
					}
				}
				break;

				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
				{
					unsigned char key = (unsigned char)wParam;
				
					if ( key==VK_RETURN && GetAsyncKeyState( VK_MENU ) )
					{
						adapter->toggle();		// note: toggle fullscreen and windowed output
						break;
					}

					if ( !down[key] )
					{
						bool defaultKeyHandlers = true;

						if ( _listener )
						{
							_listener->onKeyDown( display->wrapper() ? *display->wrapper() : *display, (Key::Code)translate[key] );
							defaultKeyHandlers = _listener->defaultKeyHandlers();
						}

						if ( defaultKeyHandlers )
						{
							if ( key == 27 )
								adapter->exit();		// quit on escape by default, return false in listener to disable this
						}

						down[key] = true;
					}

					if ( _listener )
						_listener->onKeyPressed( display->wrapper() ? *display->wrapper() : *display, (Key::Code)translate[key] );
				}
				break;

				case WM_KEYUP:
				case WM_SYSKEYUP:
				{
					unsigned char key = (unsigned char)wParam;
				
					if ( _listener )
						_listener->onKeyUp( display->wrapper() ? *display->wrapper() : *display, (Key::Code)translate[key] );

					down[key] = false;
				}
				break;

				case WM_LBUTTONDOWN:
				{
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						SetCapture( hWnd );
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.left = true;
					if ( _listener )
						_listener->onMouseButtonDown( display->wrapper() ? *display->wrapper() : *display, mouse );
				}
				break;

				case WM_MBUTTONDOWN:
				{
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						SetCapture( hWnd );
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.middle = true;
					if ( _listener )
						_listener->onMouseButtonDown( display->wrapper() ? *display->wrapper() : *display, mouse );
				}
				break;

				case WM_RBUTTONDOWN:
				{
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						SetCapture( hWnd );
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.right = true;
					if ( _listener )
						_listener->onMouseButtonDown( display->wrapper() ? *display->wrapper() : *display, mouse );
				}
				break;

				case WM_LBUTTONUP:
				{
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.left = false;
					if ( _listener )
						_listener->onMouseButtonUp( display->wrapper() ? *display->wrapper() : *display, mouse );
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						ReleaseCapture();
				}
				break;

				case WM_MBUTTONUP:
				{
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.middle = false;
					if ( _listener )
						_listener->onMouseButtonUp( display->wrapper() ? *display->wrapper() : *display, mouse );
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						ReleaseCapture();
				}
				break;

				case WM_RBUTTONUP:
				{
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					mouse.buttons.right = false;
					if ( _listener )
						_listener->onMouseButtonUp( display->wrapper() ? *display->wrapper() : *display, mouse );
					if ( !( mouse.buttons.left | mouse.buttons.right | mouse.buttons.middle ) )
						ReleaseCapture();
				}
				break;

				case WM_MOUSEMOVE:
				{
					RECT rect;
					GetClientRect( window, &rect );
					const float widthRatio = (float) width / ( rect.right - rect.left );
					const float heightRatio = (float) height / ( rect.bottom - rect.top );
					mouse.x = (float) GET_X_LPARAM( lParam ) * widthRatio;
					mouse.y = (float) GET_Y_LPARAM( lParam ) * heightRatio;
					if ( _listener )
						_listener->onMouseMove( display->wrapper() ? *display->wrapper() : *display, mouse );
				}
				break;

				default: break;
			}

			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}

		// system menu item ids

		enum SystemMenuItems
		{
			MENU_SEPARATOR_A = 1,
			MENU_ZOOM_ORIGINAL,
			MENU_ZOOM_2X,
			MENU_ZOOM_4X,
			MENU_ZOOM_8X,
			MENU_SEPARATOR_B,
			MENU_WINDOWED,
			MENU_FULLSCREEN,
			MENU_SEPARATOR_C,
			MENU_CENTER
		};

		// current zoom level for window

		enum ZoomLevel
		{
			ZOOM_RESIZED,
			ZOOM_ORIGINAL,
			ZOOM_2X,
			ZOOM_4X,
			ZOOM_8X
		};

		// update addition system menu items

		void updateSystemMenu()
		{
			systemMenu = GetSystemMenu( window, FALSE );

			// remove additional items

			RemoveMenu( systemMenu, MENU_SEPARATOR_A, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_ZOOM_ORIGINAL, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_ZOOM_2X, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_ZOOM_4X, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_ZOOM_8X, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_SEPARATOR_B, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_FULLSCREEN, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_WINDOWED, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_SEPARATOR_C, MF_BYCOMMAND );
			RemoveMenu( systemMenu, MENU_CENTER, MF_BYCOMMAND );

			// rebuild menu

			bool windowed = mode == Windowed;

			if ( windowed && !IsIconic(window) && !IsMaximized(window) )
			{
				AppendMenu( systemMenu, MF_SEPARATOR, MENU_SEPARATOR_A, TEXT("") );
				AppendMenu( systemMenu, MF_STRING, MENU_ZOOM_ORIGINAL, TEXT("Original Size") );

				const int desktopWidth = GetSystemMetrics( SM_CXSCREEN );
				const int desktopHeight = GetSystemMetrics( SM_CYSCREEN );

				if ( width*2 < desktopWidth && height*2 < desktopHeight )
					AppendMenu( systemMenu, MF_STRING, MENU_ZOOM_2X, TEXT("2x Zoom") );

				if ( width*4 < desktopWidth && height*4 < desktopHeight )
					AppendMenu( systemMenu, MF_STRING, MENU_ZOOM_4X, TEXT("4x Zoom") );
				
				if ( width*8 < desktopWidth && height*8 < desktopHeight )
					AppendMenu( systemMenu, MF_STRING, MENU_ZOOM_8X, TEXT("8x Zoom") );
			}

			AppendMenu( systemMenu, MF_SEPARATOR, MENU_SEPARATOR_B, TEXT("") );

			if ( !windowed )
				AppendMenu( systemMenu, MF_STRING, MENU_WINDOWED, TEXT("Windowed") );
			else
				AppendMenu( systemMenu, MF_STRING, MENU_FULLSCREEN, TEXT("Fullscreen") );

			if ( !centered && windowed )
			{
				AppendMenu( systemMenu, MF_SEPARATOR, MENU_SEPARATOR_C, TEXT("") );
				AppendMenu( systemMenu, MF_STRING, MENU_CENTER, TEXT("Center") );
			}
		}

	private:

		HWND window;					// window handle
		HMENU systemMenu;				// system menu handle
		int width;						// natural window width
		int height;						// natural window height
		bool active;					// true if window is currently active

		enum Mode
		{
			Fullscreen,
			Windowed
		};

		Mode mode;						// current window mode (fullscreen or windowed)

		Mouse mouse;					// current mouse input data

		Key translate[256];				// key translation table (win32 scancode -> Key::Code)
		bool down[256];					// key down table (true means key is down)

		HCURSOR arrowCursor;			// handle to system arrow cursor (does not need to be freed)
		HCURSOR nullCursor;				// null cursor when we dont want to see it

		bool centered;					// true if window is centered
		ZoomLevel zoomLevel;			// current zoom level

		WindowsAdapter * adapter;		// the adapter interface (must not be null)

		DisplayInterface * display;		// required for listener callbacks (may be null)

		Listener * _listener;			// the listener interface (may be null)
	};

	// ********************* Windows Device (DirectX 9.0) ***************************

	class WindowsDevice
	{
	public:

		WindowsDevice( LPDIRECT3D9 direct3d, HWND window, int width, int height, Mode mode, bool windowed )
		{
			assert( direct3d );
			assert( window );

			this->direct3d = direct3d;
			this->window = window;

			// store parameters

			this->width = width;
			this->height = height;
			this->window = window;
			this->mode = mode;
			this->windowed = windowed;

			// defaults

			surface = NULL;
			device = NULL;

			// setup

			createDeviceAndSurface();
		}

		/// destructor

		~WindowsDevice()
		{
			destroyDeviceAndSurface();
		}

		/// check if device is valid
		/// @returns true if the device is valid

		bool valid() const
		{
			if ( device == NULL && surface == NULL )
				return false;

			return !FAILED( device->TestCooperativeLevel() );		// not valid if device is lost
		}

		/// update the device pixels.
		/// @returns true if the update succeeded, false otherwise.

		bool update( const TrueColorPixel * trueColorPixels, const FloatingPointPixel * floatingPointPixels )
		{
			// handle device loss

			HRESULT result = device->TestCooperativeLevel();

			if ( FAILED( result ) )
			{
				if ( result == D3DERR_DEVICENOTRESET )
					device->Reset( &presentation );
			}

			if ( !valid() )
				return false;

			// copy pixels to surface

			if ( !surface )
				return false;

			D3DLOCKED_RECT lock;

			if ( FAILED( surface->LockRect( &lock, NULL, D3DLOCK_DISCARD ) ) )
				return false;

			unsigned char * data = (unsigned char*) lock.pBits;
			const int pitch = lock.Pitch;

			unsigned char * line = data;

			if ( floatingPointPixels )
			{
				Converter * converter = requestConverter( Format::XBGRFFFF, format );

				if ( converter )
				{
					converter->begin();

					const Pixel * source = floatingPointPixels;

					for ( int y = 0; y < height; ++y )
					{
						converter->convert( source, line, width );
						source += width;
						line += pitch; 
					}

					converter->end();
				}
			}
			else if ( trueColorPixels )
			{
				Converter * converter = requestConverter( Format::XRGB8888, format );

				if (converter)
				{
					converter->begin();

					const TrueColorPixel * source = trueColorPixels;

					for ( int y = 0; y < height; y++ )
					{
						converter->convert( source, line, width );
						source += width;
						line += pitch; 
					}

					converter->end();
				}
			}

			surface->UnlockRect();

			// paint display

			return paint();
		}
		
		/// paint pixels to device
		/// @returns true if paint succeeded, false if it failed.

		bool paint()
		{
			// check if valid

			if ( !valid() )
				return false;

			// copy surface to back buffer

			LPDIRECT3DSURFACE9 backBuffer;

			device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer );

			if ( !backBuffer )
				return false;

			HRESULT result = device->UpdateSurface( surface, 0, backBuffer, NULL );

			backBuffer->Release();

			if ( FAILED( result ) )
				return false;

			// present back buffer to display

			if ( FAILED( device->Present( NULL, NULL, NULL, NULL ) ) )
				return false;

			// tell windows that we dont need to repaint anything

			ValidateRect( window, NULL );

			return true;
		}

	protected:

		Format convertFormat( D3DFORMAT format )
		{
			switch ( format )
			{
				case D3DFMT_A32B32G32R32F: return Format::XBGRFFFF;
				case D3DFMT_X8R8G8B8: return Format::XRGB8888;
				case D3DFMT_X8B8G8R8: return Format::XBGR8888;
				case D3DFMT_R8G8B8: return Format::RGB888;
				case D3DFMT_R5G6B5: return Format::RGB565;
				case D3DFMT_X1R5G5B5: return Format::XRGB1555;
				default: return Format::Unknown;
			}
		}

		D3DFORMAT convertFormat( Format format )
		{
			switch ( format )
			{
				case Format::XBGRFFFF: return D3DFMT_A32B32G32R32F;
				case Format::XRGB8888: return D3DFMT_X8R8G8B8;
				case Format::XBGR8888: return D3DFMT_X8B8G8R8;
				case Format::RGB888: return D3DFMT_R8G8B8;
				case Format::RGB565: return D3DFMT_R5G6B5;
				case Format::XRGB1555: return D3DFMT_X1R5G5B5;
				default: return D3DFMT_UNKNOWN;
			}
		}

	protected:

		void zeroMemory( char * memory, int size )
		{
			while ( size-- )
				*(memory++) = 0; 
		}

		bool createDevice( LPDIRECT3D9 direct3d, int width, int height, Format format, bool windowed )
		{
			this->format = format;

			// triple buffered device

			zeroMemory( (char*) &presentation, sizeof(presentation) );

			presentation.BackBufferWidth = width;
			presentation.BackBufferHeight = height;
			presentation.Windowed = windowed;
			presentation.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentation.BackBufferFormat = convertFormat(format);
			presentation.hDeviceWindow = window;
			presentation.BackBufferCount = 2;

			if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentation, &device ) ) )
				if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_MIXED_VERTEXPROCESSING, &presentation, &device ) ) )
					if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentation, &device ) ) )
						device = NULL;

			// double buffered fallback

			if ( !device )
			{
				presentation.BackBufferCount = 1;

				if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentation, &device ) ) )
					if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_MIXED_VERTEXPROCESSING, &presentation, &device ) ) )
						if ( FAILED( direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentation, &device ) ) )
							device = NULL;
			}

			// check for failure

			if ( !device )
				return false;

			return true;
		}

	protected:

		void createDeviceAndSurface()
		{
			// create device

			if ( mode == Mode::FloatingPoint )
			{
				if ( !createDevice( direct3d, width, height, Format::XBGRFFFF, windowed ) )
					if ( !createDevice( direct3d, width, height, Format::XRGB8888, windowed ) )
						if ( !createDevice( direct3d, width, height, Format::XBGR8888, windowed ) )
							if ( !createDevice( direct3d, width, height, Format::RGB888, windowed ) )
								if ( !createDevice( direct3d, width, height, Format::RGB565, windowed ) )
									if ( !createDevice(direct3d, width, height, Format::XRGB1555, windowed ) )
										return;
			}
			else
			{
				if ( !createDevice(direct3d, width, height, Format::XRGB8888, windowed ) )
					if ( !createDevice(direct3d, width, height, Format::XBGR8888, windowed ) )
						if ( !createDevice(direct3d, width, height, Format::RGB888, windowed ) )
							if ( !createDevice(direct3d, width, height, Format::RGB565, windowed ) )
								if ( !createDevice(direct3d, width, height, Format::XRGB1555, windowed ) )
									return;
			}

			// create surface

			device->CreateOffscreenPlainSurface( width, height, convertFormat(format), D3DPOOL_SYSTEMMEM, &surface, NULL );
		}

		void destroyDeviceAndSurface()
		{
			if ( surface )
			{
				surface->Release();
				surface = NULL;
			}

			if ( device )
			{
				device->Release();
				device = NULL;
			}
		}

	private:

		LPDIRECT3D9 direct3d;
		HWND window;

		D3DPRESENT_PARAMETERS presentation;
		LPDIRECT3DDEVICE9 device;
		LPDIRECT3DSURFACE9 surface;

		int width;
		int height;
		Format format;
		Mode mode;
		bool windowed;
		bool lost;
	};

	// ********************* Windows Display Implementation ***************************

	class WindowsDisplay : public DisplayAdapter, public WindowsAdapter
	{
	public:

		WindowsDisplay()
		{
			direct3d = Direct3DCreate9( D3D_SDK_VERSION );
			defaults();
		}

		~WindowsDisplay()
		{
			close();

			if ( direct3d )
			{
				direct3d->Release();
				direct3d = NULL;
			}
		}

		bool open( const char title[], int width, int height, Output output, Mode mode )
		{
			DisplayAdapter::open( title, width, height, output, mode );

			window = new WindowsWindow( this, this, title, width, height );

			window->listener( DisplayAdapter::listener() );			// note: fixes bug where listener was forgotten after display close

			if ( !window->handle() )
			{
				close();
				return false;
			}

			bool result = false;

			if ( output==Output::Default || output == Output::Windowed )
				result = windowed();
			else
				result = fullscreen();

			if ( !result )
				return false;

			if ( DisplayAdapter::listener() )
				DisplayAdapter::listener()->onOpen( wrapper() ? *wrapper() : *(DisplayInterface*)this );

			return true;
		}

		void close()
		{
			delete device;
			delete window;

			DisplayAdapter::close();
		}

		bool update( const TrueColorPixel * trueColorPixels, const FloatingPointPixel * floatingPointPixels )
		{
			if ( shutdown )
			{
				close();
				return false;
			}

			if ( pendingToggle )
			{
				pendingToggle = false;

				if ( output() == Output::Windowed )
					fullscreen();
				else
					windowed();
			}

			if ( window )
				window->update();

			if ( device )
				device->update( trueColorPixels, floatingPointPixels );

			if ( window && !window->visible() )
			{
				window->show();
				window->update();
			}

			return true;
		}

		void listener( Listener * listener )
		{
			DisplayAdapter::listener( listener );

			if ( window )
				window->listener( listener );
		}

		// implement adapter interface for interoperability with window class

		bool paint()
		{
			if ( !device || !device->valid() )
			{
				HDC dc = GetDC( window->handle() );

				if ( dc )
				{
					HBRUSH brush = CreateSolidBrush( RGB(0,0,0) );
					SelectObject( dc, brush );
					RECT rect;
					GetClientRect( window->handle(), &rect );
					Rectangle( dc, 0, 0, rect.right, rect.bottom );
					DeleteObject( brush );
					ReleaseDC( window->handle(), dc );
				}

				ValidateRect( window->handle(), NULL );

				return true;
			}
			else
			{
				return device->paint();
			}
		}

		bool fullscreen()
		{
			if ( device )
			{
				if ( output() == Output::Fullscreen )
					return true;

				delete device;
				device = NULL;
			}

			window->fullscreen( width(), height() );

			device = new WindowsDevice( direct3d, window->handle(), width(), height(), mode(), false );

			if ( !device->valid() )
			{
				close();
				return false;
			}

			DisplayAdapter::fullscreen();

			window->update();

			return true;
		}

		bool windowed()
		{
			if ( device )
			{
				if ( output() == Output::Windowed )
					return true;

				delete device;
				device = NULL;
			}

			window->windowed( width(), height() );

			device = new WindowsDevice( direct3d, window->handle(), width(), height(), mode(), true );

			if ( !device->valid() )
			{
				close();
				return false;
			}

			DisplayAdapter::windowed();

			return true;
		}

		void toggle()
		{
			pendingToggle = true;
		}

		void exit()
		{
			shutdown = true;
		}

	protected:

		void defaults()
		{
			DisplayAdapter::defaults();
			window = NULL;
			device = NULL;
			shutdown = false;
			pendingToggle = false;
		}

	private:

		LPDIRECT3D9 direct3d;
		WindowsWindow * window;
		WindowsDevice * device;
		bool shutdown;
		bool pendingToggle;
	};

	// ********************* Windows High Resolution Timer Implementation ***************************

	class WindowsTimer : public PixelToaster::TimerInterface
	{
	public:
		
		WindowsTimer()
		{
			QueryPerformanceFrequency( (LARGE_INTEGER*) &_frequency );
			reset();
		}
		
		void reset()
		{
			QueryPerformanceCounter( (LARGE_INTEGER*) &_timeCounter );
			_deltaCounter = _timeCounter;
			_time = 0.0;
		}
		
		double time()
		{
			__int64 counter;
			QueryPerformanceCounter( (LARGE_INTEGER*) &counter );
			_time += ( counter - _timeCounter ) / (double) _frequency;
			_timeCounter = counter;
			return _time;
		}
		
		double delta()
		{
			__int64 counter;
			QueryPerformanceCounter( (LARGE_INTEGER*) &counter );
			double delta = (counter - _deltaCounter) / (double) _frequency;
			_deltaCounter = counter;
			return delta;
		}
		
		double resolution()
		{
			return 1.0 / (double) _frequency;
		}
		
		void wait( double seconds )
		{
			Sleep( int(seconds*1000) );
		}
		
	private:

		double _time;               ///< current time in seconds
		__int64 _timeCounter;       ///< raw 64bit timer counter for time
		__int64 _deltaCounter;      ///< raw 64bit timer counter for delta
		__int64 _frequency;         ///< raw 64bit timer frequency
	};
}

