#include "MyDirectX.h"
#include<iostream>

using namespace std;

//Direct3D variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DSURFACE9 backbuffer = NULL;

//DirectInput variables
LPDIRECTINPUT8 dinput = NULL;
LPDIRECTINPUTDEVICE8 dimouse = NULL;
LPDIRECTINPUTDEVICE8 dikeyboard = NULL;
DIMOUSESTATE mouse_state;
char Keys[256];
XINPUT_GAMEPAD controllers[4];


//Direct3d initialization
bool Direct3D_Init(HWND window, int width, int height, bool fullscreen)
{
	//initialize Direct 3D
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)return false;

	//set Direct3D presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = (!fullscreen);
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.hDeviceWindow = window;

	//Create Direct3D device
	d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (!d3ddev)return false;

	//get a pointer to the back buffer surface 
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

	D3DXCreateSprite(d3ddev, &obj);
	return true;
}

//Direct3D shutdown
void Direct3D_Shutdown()
{
	if (obj)obj->Release();
	if (d3ddev)d3ddev->Release();
	if (d3d)d3d->Release();
}

//Draws a surface to the screen using StretchRect
void DrawSurface(LPDIRECT3DSURFACE9 dest, float x, float y, LPDIRECT3DSURFACE9 source)
{
	//get width/height from source surface
	D3DSURFACE_DESC desc;
	source->GetDesc(&desc);

	//create rects for drawing
	RECT source_rect = { 0,0,(long)desc.Width,(long)desc.Height };
	RECT dest_rect = { (long)x,(long)y,(long)x + desc.Width,(long)y + desc.Height };

	//draw the source surface onto the dest 
	d3ddev->StretchRect(source, &source_rect, dest, &dest_rect, D3DTEXF_NONE);
}

//Load a bitmap file into a surface 
LPDIRECT3DSURFACE9 LoadSurface(string filename)
{
	LPDIRECT3DSURFACE9 image = NULL;

	//get width adn height from bitmap file
	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK)return NULL;

	//create surface 
	result = d3ddev->CreateOffscreenPlainSurface(
		info.Width,
		info.Height,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&image,
		NULL);
	if (result != D3D_OK)return NULL;

	//load surface from file into nwely created surface 
	result = D3DXLoadSurfaceFromFile(
		image,
		NULL,
		NULL,
		filename.c_str(),
		NULL,
		D3DX_DEFAULT,
		D3DCOLOR_XRGB(0, 0, 0),
		NULL);
	if (result != D3D_OK)return NULL;
	return image;
}

void Sprite_Draw(LPDIRECT3DTEXTURE9 image, int left, int top, int right, int bottom,int destx,int desty)
{
	D3DXVECTOR3 position((float)destx, (float)desty, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	RECT rect = { left,top,right,bottom };
	obj->Draw(image, &rect, NULL, &position, white);
}

void Sprite_Draw_Frame(LPDIRECT3DTEXTURE9 texture, int destx, int desty, int framew, int frameh, int framenum, int columns)
{
	D3DXVECTOR3 position((float)destx, (float)desty, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	RECT rect;
	rect.left = (framenum%columns)*framew;
	rect.top = (framenum / columns)*frameh;
	rect.right = rect.left + framew;
	rect.bottom = rect.top + frameh;
	obj->Draw(texture, &rect, NULL, &position, white);
}

void Sprite_Animate(int &frame, int startframe, int endframe, int direction, int &starttime, int delay)
{
	if ((int)GetTickCount() > starttime + delay)
	{
		starttime = GetTickCount();
		frame += direction;
		if (frame > endframe)frame = startframe;
		if (frame < startframe)frame = endframe;
	}
}


//DirectInput initalization 
bool DirectInput_Init(HWND hwnd)
{
	//initalization DirectInput object
	HRESULT result = DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&dinput,
		NULL);

	//initalize the keyboard
	dinput->CreateDevice(GUID_SysKeyboard, &dikeyboard, NULL);
	dikeyboard->SetDataFormat(&c_dfDIKeyboard);
	dikeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dikeyboard->Acquire();

	//initalize mouse
	dinput->CreateDevice(GUID_SysMouse, &dimouse, NULL);
	dimouse->SetDataFormat(&c_dfDIMouse);
	dimouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dimouse->Acquire();

	d3ddev->ShowCursor(false);
	return true;
}

//directinput update 
void DirectInput_Update()
{
	//update mouse
	dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);

	//update keyboard
	dikeyboard->GetDeviceState(sizeof(Keys), (LPVOID)Keys);

	//update controllers
	for (int i = 0; i < 4; i++)
	{
		ZeroMemory(&controllers[i], sizeof(XINPUT_STATE));

		//get the state of the controllers
		XINPUT_STATE state;
		DWORD result = XInputGetState(i, &state);

		//store state in global controllers array
		if (result == 0)controllers[i] = state.Gamepad;
	}
}

//return mouse X move
int Mouse_X()
{
	return mouse_state.lX;
}
//return mouse Y move
int Mouse_Y()
{
	return mouse_state.lY;
}
//return mouse button state
int Mouse_Button(int button)
{
	return mouse_state.rgbButtons[button] & 0x80;
}
//return key down 
int Key_Down(int key)
{
	return (Keys[key] & 0x80);
}

//DirectInput shutdown
void DirectInput_Shutdown()
{
	if (dikeyboard)
	{
		dikeyboard->Unacquire();
		dikeyboard->Release();
		dikeyboard = NULL;
	}
	if (dimouse)
	{
		dimouse->Unacquire();
		dimouse->Release();
		dimouse = NULL;
	}
}

//return true if controller is plugged in(¸üÐÂ)
bool XInput_Controller_Found()
{
	XINPUT_CAPABILITIES caps;
	ZeroMemory(&caps, sizeof(XINPUT_CAPABILITIES));
	XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps);
	if (caps.Type != 0)return false;
	return true;
}

//Vibrats the controller
void XInput_Vibrate(int contNum, int amount)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = amount;
	vibration.wRightMotorSpeed = amount;
	XInputSetState(contNum, &vibration);
}

LPDIRECT3DTEXTURE9 LoadTexture(std::string filename, D3DCOLOR transcolor)
{
	LPDIRECT3DTEXTURE9 texture = NULL;

	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK)return NULL;
	D3DXCreateTextureFromFileEx(
		d3ddev,
		filename.c_str(),
		info.Width,
		info.Height,
		1,
		D3DPOOL_DEFAULT,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		transcolor,
		&info,
		NULL,
		&texture
	);
	if (result != D3D_OK)return NULL;
	return texture;
}

D3DXVECTOR2 GitBitmapSize(string filename)
{
	D3DXIMAGE_INFO info;
	D3DXVECTOR2 size = D3DXVECTOR2(0.0f, 0.0f);
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result == D3D_OK)
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);
	else
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);
	return size;
}



void Sprite_Transform_Draw(LPDIRECT3DTEXTURE9 image, int x, int y, int width, int height, int frame, int columns, float rotation, float scaling, D3DCOLOR color)
{
	//create new scale vector
	D3DXVECTOR2 scale(scaling, scaling);

	//create new translate vector 
	D3DXVECTOR2 trans(x, y);

	//set center by dividing width and height by two 
	D3DXVECTOR2 center((float)(width*scaling) / 2, (float)(height*scaling) / 2);

	//create 2d transformtion2D matrix
	D3DXMATRIX mat;
	D3DXMatrixTransformation2D(&mat, NULL, 0, &scale, &center, rotation, &trans);

	//tell sprite object to use the transform 
	obj->SetTransform(&mat);

	//calculate frame location in source image
	int fx = (frame%columns)*width;
	int fy = (frame / columns)*height;
	RECT srcRect = { fx,fy,fx + width,fy + height };

	//draw the sprite frame
	obj->Draw(image, &srcRect, NULL, NULL, color);
}

//bounding box collision detection
int Collision(SPRITE sprite1, SPRITE sprite2)
{
	RECT rect1;
	rect1.left = (long)sprite1.x;
	rect1.top = (long)sprite1.y;
	rect1.right = (long)sprite1.x + sprite1.width*sprite1.scaling;
	rect1.bottom = (long)sprite1.y + sprite1.height*sprite1.scaling;

	RECT rect2;
	rect2.left = (long)sprite2.x;
	rect2.top = (long)sprite2.y;
	rect2.right = (long)sprite2.x + sprite2.width*sprite2.scaling;
	rect2.bottom = (long)sprite2.y + sprite2.height*sprite2.scaling;

	RECT dest;
	return IntersectRect(&dest, &rect1, &rect2);
}

bool CollisionD(SPRITE sprite1, SPRITE sprite2)
{
	double radius1, radius2;
	if (sprite1.width > sprite1.height)
		radius1 = (sprite1.width*sprite1.scaling) / 2.0;
	else
		radius1 = (sprite1.height*sprite1.scaling) / 2.0;
	double x1 = sprite1.x + radius1;
	double y1 = sprite1.y + radius1;
	D3DXVECTOR2 vector1(x1, y1);

	if (sprite2.width > sprite2.height)
		radius2 = (sprite2.width * sprite2.scaling) / 2.0;
	else
		radius2 = (sprite2.height * sprite2.scaling) / 2.0;
	double x2 = sprite2.x + radius2;
	double y2 = sprite2.y + radius2;
	D3DXVECTOR2 vector2(x2, y2);

	double delatx = vector1.x - vector2.x;
	double delaty = vector1.y - vector2.y;
	double dist = sqrt((delatx*delatx) + (delaty*delaty));

	return (dist < radius1 + radius2);
}

LPD3DXFONT MakeFont(string name, int size)
{
	LPD3DXFONT font = NULL;
	D3DXFONT_DESC desc = {
		size,							//height
		0,								//width
		0,								//weight
		0,								//miplevels
		false,							//italic
		DEFAULT_CHARSET,				//charset
		OUT_TT_PRECIS,					//output precision
		CLIP_DEFAULT_PRECIS,			//quality
		DEFAULT_PITCH,					//pitch
		""								//font name
	};
	strcpy(desc.FaceName, name.c_str());
	D3DXCreateFontIndirect(d3ddev, &desc, &font);
	return font;
}

void FontPrint(LPD3DXFONT font, int x, int y, string text, D3DCOLOR color)
{
	RECT rect = { x, y, 0, 0 };
	font->DrawText(NULL, text.c_str(), text.length(), &rect, DT_CALCRECT, color);

	font->DrawText(obj, text.c_str(), text.length(), &rect, DT_LEFT, color);
}
