#include"MyDirectX.h"

const string APPTITLE = "Anti Virus game";
const int SCREENW = 960;
const int SCREENH = 768;
const bool  FULLSCREEN = false;

LPD3DXSPRITE obj;
int n = 0;

//游戏变量
enum GAME_STATES
{
	BRIEFING = 0,
    PALYING = 1
};
GAME_STATES game_state = BRIEFING;

//文字变量
LPD3DXFONT font;
LPD3DXFONT hugefont;
LPD3DXFONT debugfont;

//时间变量
DWORD refresh = 0;
DWORD screentime = 0;
double screenfps = 0.0;
double screencount = 0.0;
DWORD coretime = 0;
double corefps = 0.0;
double corecount = 0.0;
DWORD currenttime;

//背景滚动变量
const int BUFFERW = SCREENW * 2;
const int BUFFERH = SCREENH;
LPDIRECT3DSURFACE9 background = NULL;
double scrollx = 0;
double scrolly = 0;
const double virtual_level_size = BUFFERW * 5;
double virtual_scrollx = 0;

//玩家变量
LPDIRECT3DTEXTURE9 player_ship;
SPRITE player;
enum PLAYER_STATES
{
	NORMAL = 0,
	PHASING = 1,
	OVERLOADING = 2
};
PLAYER_STATES player_state = NORMAL;
PLAYER_STATES player_state_previous = NORMAL;

D3DXVECTOR2 position_history[8];
int position_history_index = 0;
DWORD position_history_timer = 0;
double charge_angle = 0.0;
double charge_tweak = 0.0;
double charge_twak_dir = 1.0;
int energy = 100;
int healthy = 100;
int lives = 3;
int score = 0;


//敌人
const int VIRUSES = 200;
LPDIRECT3DTEXTURE9 virus_image;
SPRITE viruses[VIRUSES];
const int FRAGMENTS = 300;
LPDIRECT3DTEXTURE9 fragment_image;
SPRITE fragments[FRAGMENTS];

//子弹
LPDIRECT3DTEXTURE9 purple_fire;
const int BULLETS = 300;
SPRITE bullets[BULLETS];
int player_shoot_timer = 0;
int firepower = 5;
int bulletcount = 0;

//用户界面元素
LPDIRECT3DTEXTURE9 energy_slice;
LPDIRECT3DTEXTURE9 health_slice;

//控制变量
int vibrating = 0;
int vibration = 100;

//允许string再任何地方转化
template <class T>
std::string static ToString(const T &t, int places = 2)
{
	ostringstream oss;
	oss.precision(places);
	oss.setf(ios_base::fixed);
	oss << t;
	return oss.str();
}
bool Create_Viruses()
{
	virus_image = LoadTexture("source/virus.png");
	if (!virus_image)
	{
		MessageBox(NULL, "Error Virus photo", "Error", MB_OK);
		return false;
	}
	for (int n = 0; n < VIRUSES; n++)
	{
		D3DCOLOR color = D3DCOLOR_ARGB(170 + rand() % 80, 150 + rand() % 100, 25 + rand() % 50, 25 + rand() % 50);
		viruses[n].color = color;
		viruses[n].scaling = (float)((rand() % 25 + 50) / 100.0f);
		viruses[n].alive = true;
		viruses[n].width = 96;
		viruses[n].height = 96;
		viruses[n].x = (float)(1000 + rand() % BUFFERW);
		viruses[n].y = (float)(rand() % SCREENH);
		viruses[n].velx = (float)((rand() % 8)*-1);
		viruses[n].vely = (float)(rand() % 2 - 1);
	}
	return true;
}

bool Create_Fragment()
{
	fragment_image = LoadTexture("source/fragment.png");
	if (!fragment_image)
	{
		MessageBox(NULL, "Error fragment photot", "Error", MB_OK);
		return false;
	}
	for (int n = 0; n < FRAGMENTS; n++)
	{
		D3DCOLOR fragmentcolor = D3DCOLOR_ARGB(125 + rand() % 50, 150 + rand() % 100, 150 + rand() % 100, 150 + rand() % 100);
		fragments[n].color = fragmentcolor;
		fragments[n].alive = true;
		fragments[n].width = 128;
		fragments[n].height = 128;
		fragments[n].scaling = (float)((rand() % 8 + 6) / 100.0f);
		fragments[n].rotation = (float)(rand() % 360);
		fragments[n].velx = (float)(rand() % 4 + 1)*-1.0f;
		fragments[n].vely = (float)(rand() % 10 - 5) / 10.0f;
		fragments[n].x = (float)(rand() % BUFFERW);
		fragments[n].y = (float)(rand() % BUFFERH);
	}
	return true;
}

bool Create_Background()
{
	LPDIRECT3DSURFACE9 image = NULL;
	image = LoadSurface("source/background.tga");
	if (!image)
	{
		MessageBox(NULL, "Error Background photo", "ERROR", MB_OK);
		return false;
	}
	HRESULT result = d3ddev->CreateOffscreenPlainSurface(BUFFERW, BUFFERH, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &background, NULL);
	if (result != D3D_OK)
	{
		MessageBox(NULL, "Error background  result", "Error", MB_OK);
		return false;
	}
	RECT source_rectul = { 0,0,SCREENW,SCREENH };
	RECT source_rectur = { SCREENW,0,SCREENW * 2,SCREENH };
	RECT dest_ul = { 0,0,SCREENW,SCREENH };
	RECT dest_ur = { SCREENW,0,SCREENW * 2,SCREENH };
	d3ddev->StretchRect(image, &source_rectur, background, &dest_ur, D3DTEXF_NONE);
	d3ddev->StretchRect(image, &source_rectul, background, &dest_ul, D3DTEXF_NONE);

	//d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &background);

	image->Release();
	return true;
}

bool Game_Init(HWND window)
{
	Direct3D_Init(window, SCREENW, SCREENH, FULLSCREEN);
	DirectInput_Init(window);

	font = MakeFont("Arial Bold", 24);
	debugfont = MakeFont("Arial", 14);
	hugefont = MakeFont("Arial Bold", 80);

	player_ship = LoadTexture("source/ships.png");
	if (!player_ship)
	{
		MessageBox(NULL, "Error player_ship photo", "Error", MB_OK);
		return false;
	}
	player.x = 100;
	player.y = 350;

	player.width = player.height = 64;
	for (int n = 0; n < 4; n++)
	{
		position_history[n] = D3DXVECTOR2(-100, 0);
	}

	purple_fire = LoadTexture("source/purplefire.png");
	for (int n = 0; n < BULLETS; n++)
	{
		bullets[n].alive = false;
		bullets[n].x = 0;
		bullets[n].y = 0;
		bullets[n].width = 20;
		bullets[n].height = 8;
	}
	if (!Create_Viruses())
	{
		MessageBox(NULL, "Error GreateViruse", "Error", MB_OK);
		return false;
	}
	energy_slice = LoadTexture("source/energyslice.png");
	health_slice = LoadTexture("source/healthslice.png");
	
	if (!Create_Fragment())
	{
		MessageBox(NULL, "Error CreateFragment", "Error", MB_OK);
		return false;
	}

	if (!Create_Background())
	{
		MessageBox(NULL, "Erroe Create  background", "Error", MB_OK);
		return false;
	}
	return true;

}

void Game_End()
{
	if (background)
	{
		background->Release();
		background = NULL;
	}
	if (font)
	{
		font->Release();
		font = NULL;
	}
	if (debugfont)
	{
		debugfont->Release();
		font = NULL;
	}
	if (hugefont)
	{
		hugefont->Release();
		font = NULL;
	}
	if (fragment_image)
	{
		fragment_image->Release();
		fragment_image = NULL;
	}
	if (player_ship)
	{
		player_ship->Release();
		player_ship = NULL;
	}
	if (virus_image)
	{
		virus_image->Release();
		virus_image = NULL;
	}
	if (purple_fire)
	{
		purple_fire->Release();
		purple_fire = NULL;
	}
	if (health_slice)
	{
		health_slice->Release();
		health_slice = NULL;
	}
	if (energy_slice)
	{
		energy_slice->Release();
		energy_slice = NULL;
	}

	DirectInput_Shutdown();
	Direct3D_Shutdown();
}


void move_player(float movex, float movey)
{
	if (player_state == OVERLOADING || player_state_previous == OVERLOADING || player_state == PHASING || player_state_previous == PHASING)
	{
		return;
	}

	float multi = 4.0f;
	player.x += movex*multi;
	player.y += movey*multi;
	if (player.x < 0.0f) player.x = 0.0f;
	else if (player.y > SCREENW - (player.width*player.scaling)) player.x = SCREENW - (player.width*player.scaling);
	if (player.y < 0.0f)player.y = 0.0f;
	else if (player.y > SCREENH - (player.height*player.scaling))player.y = SCREENH - (player.height*player.scaling);
}



const double PI = 3.1415926535;
const double PI_under_180 = 180.0f / PI;
const double PI_over_180 = PI / 180.0f;
double toRadians(double degrees)
{
	return degrees *PI_over_180;
}
double toDregees(double radians)
{
	return radians*PI_under_180;
}
double wrap(double value, double bounds)
{
	double result = fmod(value, bounds);
	if (result < 0)result += bounds;
	return result;
}
double wrapAngleDegs(double degs)
{
	return wrap(degs, 360.0);
}
double LinearVelocityX(double angle)
{
	if (angle < 0)angle = 360 + angle;
	return cos(angle*PI_over_180);
}
double LinearVelocityY(double angle)
{
	if (angle < 0)angle = angle + 360;
	return sin(angle*PI_over_180);
}

void add_energy(double value)
{
	energy += value;
	if (energy < 0.0)energy = 0.0;
	if (energy > 100.0)energy = 100.0;
}

void Vibrate(int contnum, int amount, int length)
{
	vibrating = 1;
	vibration = length;
	XInput_Vibrate(contnum, amount);
}
int find_bullet()
{
	int bullet = -1;
	for (int n = 0; n < BULLETS; n++)
	{
		if (!bullets[n].alive)
		{
			bullet = n;
			break;
		}
	}
	return bullet;
}



bool player_overload()
{
	if (energy < 50.0)return false;
	add_energy(-0.5);

	Vibrate(0, 20000, 20);
	int b1 = find_bullet();
	if (b1 == -1)return true;
	bullets[b1].alive = true;
	bullets[b1].velx = 0.0f;
	bullets[b1].vely = 0.0f;
	bullets[b1].rotation = (float)(rand() % 360);
	bullets[b1].x = player.x + player.width;
	bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
	bullets[b1].y += (float)(rand() % 20 - 10);
	return true;
}

void player_shoot()
{
	if ((int)timeGetTime() < player_shoot_timer + 100)return;
	player_shoot_timer = timeGetTime();

	add_energy(-1.0);
	if (energy < 0.0)
	{
		energy = 0.0;
		return;
	}
	Vibrate(0, 20000, 10);
	switch (firepower)
	{
	case 1:
	{
		int b1 = find_bullet();
		if (b1 == -1)return;
		bullets[b1].alive = true;
		bullets[b1].rotation = 0.0f;
		bullets[b1].velx = 12.0f;
		bullets[b1].vely = 0.0f;
		bullets[b1].x = player.x + player.width;
		bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
	}
	break;
	case 2:
	{
		int b1 = find_bullet();
		if (b1 == -1)return;
		bullets[b1].alive = true;
		bullets[b1].rotation = 0.0f;
		bullets[b1].velx = 12.0f;
		bullets[b1].vely = 0.0f;
		bullets[b1].x = player.x + player.width;
		bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
		bullets[b1].y += 10;

		int b2 = find_bullet();
		if (b2 == -1)return;
		bullets[b2].alive = true;
		bullets[b2].rotation = 0.0f;
		bullets[b2].velx = 12.0f;
		bullets[b2].vely = 0.0f;
		bullets[b2].x = player.x + player.width;
		bullets[b2].y = player.y + player.height / 2 - bullets[b2].height / 2;
		bullets[b2].y -= 10;
	}
	break;
	case 3:
	{
		int b1 = find_bullet();
		if (b1 == -1)return;
		bullets[b1].alive = true;
		bullets[b1].rotation = 0.0f;
		bullets[b1].velx = 12.0f;
		bullets[b1].vely = 0.0f;
		bullets[b1].x = player.x + player.width;
		bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;

		int b2 = find_bullet();
		if (b2 == -1)return;
		bullets[b2].alive = true;
		bullets[b2].rotation = 0.0f;
		bullets[b2].velx = 12.0f;
		bullets[b2].vely = 0.0f;
		bullets[b2].x = player.x + player.width;
		bullets[b2].y = player.y + player.height / 2 - bullets[b2].height / 2;
		bullets[b2].y -= 16;
	
		int b3 = find_bullet();
		if (b3 == -1)return;
		bullets[b3].alive = true;
		bullets[b3].rotation = 0.0f;
		bullets[b3].velx = 12.0f;
		bullets[b3].vely = 0.0f;
		bullets[b3].x = player.x + player.width;
		bullets[b3].y = player.y + player.height / 2 - bullets[b3].height / 2;
		bullets[b3].y += 16;
	}
	break;
	case 4:
	{
		int b1 = find_bullet();
		if (b1 == -1)return;
		bullets[b1].alive = true;
		bullets[b1].rotation = 0.0f;
		bullets[b1].velx = 12.0f;
		bullets[b1].vely = 0.0f;
		bullets[b1].x = player.x + player.width;
		bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
		bullets[b1].y += 12;

		int b2 = find_bullet();
		if (b2 == -1)return;
		bullets[b2].alive = true;
		bullets[b2].rotation = 0.0f;
		bullets[b2].velx = 12.0f;
		bullets[b2].vely = 0.0f;
		bullets[b2].x = player.x + player.width;
		bullets[b2].y = player.y + player.height / 2 - bullets[b2].height / 2;
		bullets[b2].y -= 12;

		int b3 = find_bullet();
		if (b3 == -1)return;
		bullets[b3].alive = true;
		bullets[b3].rotation = 0.0f;
		bullets[b3].velx = 12.0f;
		bullets[b3].vely = 0.0f;
		bullets[b3].x = player.x + player.width;
		bullets[b3].y = player.y + player.height / 2 - bullets[b3].height / 2;
		bullets[b3].y += 32;
		
		int b4 = find_bullet();
		if (b4 == -1)return;
		bullets[b4].alive = true;
		bullets[b4].rotation = 0.0f;
		bullets[b4].velx = 12.0f;
		bullets[b4].vely = 0.0f;
		bullets[b4].x = player.x + player.width;
		bullets[b4].y = player.y + player.height / 2 - bullets[b4].height / 2;
		bullets[b4].y -= 32;
	}
	break;
	case 5:
	{
		int b1 = find_bullet();
		if (b1 == -1)return;
		bullets[b1].alive = true;
		bullets[b1].rotation = 3.0f;
		bullets[b1].velx = (float)(12.0f*LinearVelocityX(bullets[b1].rotation));
		bullets[b1].vely = (float)(12.0f*LinearVelocityY(bullets[b1].rotation));
		bullets[b1].x = player.x + player.width;
		bullets[b1].y = player.y + player.height / 2 - bullets[b1].height / 2;
		bullets[b1].y += 12;

		int b2 = find_bullet();
		if (b2 == -1)return;
		bullets[b2].alive = true;
		bullets[b2].rotation = -3.0f;
		bullets[b2].velx = (float)(12.0f*LinearVelocityX(bullets[b2].rotation));
		bullets[b2].vely = (float)(12.0f*LinearVelocityY(bullets[b2].rotation));
		bullets[b2].x = player.x + player.width;
		bullets[b2].y = player.y + player.height / 2 - bullets[b2].height / 2;
		bullets[b2].y -= 12;

		int b3 = find_bullet();
		if (b3 == -1)return;
		bullets[b3].alive = true;
		bullets[b3].rotation = 6.0f;
		bullets[b3].velx = (float)(12.0f*LinearVelocityX(bullets[b3].rotation));
		bullets[b3].vely = (float)(12.0f*LinearVelocityY(bullets[b3].rotation));
		bullets[b3].x = player.x + player.width;
		bullets[b3].y = player.y + player.height / 2 - bullets[b3].height / 2;
		bullets[b3].y += 20;

		int b4 = find_bullet();
		if (b4 == -1)return;
		bullets[b4].alive = true;
		bullets[b4].rotation = -6.0f;
		bullets[b4].velx = (float)(12.0f*LinearVelocityX(bullets[b4].rotation));
		bullets[b4].vely = (float)(12.0f*LinearVelocityY(bullets[b4].rotation));
		bullets[b4].x = player.x + player.width;
		bullets[b4].y = player.y + player.height / 2 - bullets[b4].height / 2;
		bullets[b4].y -= 20;

		int b5 = find_bullet();
		if (b5 == -1)return;
		bullets[b5].alive = true;
		bullets[b5].rotation = 0.0f;
		bullets[b5].velx = 12.0f;
		bullets[b5].vely = 0.0f;
		bullets[b5].x = player.x + player.width;
		bullets[b5].y = player.y + player.height / 2 - bullets[b5].height / 2;
	}
	break;
	}
}


void Update_Background()
{
	scrollx += 0.4;
	if (scrolly < 0)
	{
		scrolly = BUFFERH - SCREENH;
	}
	if (scrolly > BUFFERH - SCREENH)
	{
		scrolly = 0;
	}
	if (scrollx < 0)
	{
		scrollx = BUFFERW - SCREENW;
	}
	if (scrollx > BUFFERW - SCREENH)
	{
		scrollx = 0;
	}

	virtual_scrollx += 1.0;
	if (virtual_scrollx > virtual_level_size)
		virtual_scrollx = 0.0;
}

void Updata_bullets()
{
	if (player_state == NORMAL&&player_state_previous == OVERLOADING)
	{
		int bulletcount = 0;
		for (int n = 0; n < BULLETS; n++)
		{
			if (bullets[n].alive&&bullets[n].velx == 0.0f)
			{
				bulletcount++;
				bullets[n].rotation = (float)(rand() % 90 - 45);
				bullets[n].velx = (float)(20.0*LinearVelocityX(bullets[n].rotation));
				bullets[n].vely = (float)(20.0*LinearVelocityY(bullets[n].rotation));
			}
		}
		if (bulletcount > 0)
		{
			Vibrate(0, 40000, 30);
		}
		player_state_previous = NORMAL;
	}

	bulletcount = 0;
	for (int n = 0; n < BULLETS; n++)
	{
		if (bullets[n].alive)
		{
			bulletcount++;
			bullets[n].x += bullets[n].velx;
			bullets[n].y += bullets[n].vely;
			if (bullets[n].x<0 || bullets[n].x>SCREENW || bullets[n].y<0 || bullets[n].y>SCREENH)
			{
				bullets[n].alive = false;
			}
		}
	}
}

void Damage_Player()
{
	healthy -= 10;
	if (healthy <= 0)
	{
		lives -= 1;
		healthy = 100;
		if (lives <= 0)
		{
			game_state = GAME_STATES::BRIEFING;
		}
	}
}

void Update_Viruses()
{
	for (int n = 0; n < VIRUSES; n++)
	{
		if (viruses[n].alive)
		{
			viruses[n].x += viruses[n].velx;
			if (viruses[n].x < -96.0f)viruses[n].x = (float)virtual_level_size;
			if (viruses[n].x > (float)virtual_level_size)viruses[n].x = -96.0f;

			viruses[n].y += viruses[n].vely;
			if (viruses[n].y < -96.0f)viruses[n].y = SCREENH;
			if (viruses[n].y > SCREENH)viruses[n].y = -96.0f;

			if (Collision(player, viruses[n]))
			{
				viruses[n].alive = false;
				Damage_Player();
			}
		}
	}
}

void Update_Fragments()
{
	for (int n = 0; n < FRAGMENTS; n++)
	{
		if (fragments[n].alive)
		{
			fragments[n].x += fragments[n].velx;
			if (fragments[n].x < 0.0 - fragments[n].width)fragments[n].x = BUFFERW;
			if (fragments[n].x > virtual_level_size)fragments[n].x = 0.0;
			if (fragments[n].y < 0.0 - fragments[n].height)fragments[n].y = SCREENH;
			if (fragments[n].y > SCREENH)fragments[n].y = 0.0;

			fragments[n].rotation += 0.01f;

			float oldscale = fragments[n].scaling;
			fragments[n].scaling *= 10.0;
			if (CollisionD(player, fragments[n]))
			{
				float playerx = player.x + player.width / 2.0f;
				float playery = player.y + player.height / 2.0f;

				float fragmentx = fragments[n].x;
				float fragmenty = fragments[n].y;

				if (fragmentx < playerx)fragments[n].x += 6.0f;
				if (fragmentx > playerx)fragments[n].x -= 6.0f;
				if (fragmenty < playery)fragments[n].y += 6.0f;
				if (fragmenty > playery)fragments[n].y -= 6.0f;
			}
			fragments[n].scaling = oldscale;
			if (CollisionD(player, fragments[n]))
			{
				add_energy(2.0);
				fragments[n].x = (float)(3000 + rand() % 1000);
				fragments[n].y = (float)(rand() % SCREENH);
			}
		}
	}
}


void Test_Virus_Collisions()
{
	for (int v = 0; v < VIRUSES; v++)
	{
		if (viruses[v].alive)
		{
			for (int b = 0; b < BULLETS; b++)
			{
				if (bullets[b].alive)
				{
					if (Collision(viruses[v], bullets[b]))
					{
						bullets[b].alive = false;
						viruses[v].alive = false;
						score += viruses[v].scaling*10.0f;
					}
				}
			}
		}
	}
}

void Draw_Background()
{
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	RECT source_rect = { (long)scrollx,(long)scrolly,(long)SCREENW + scrollx,(long)SCREENH + scrolly };
	RECT dest_rect = { 0,0,SCREENW,SCREENH };
	d3ddev->StretchRect(background, &source_rect, backbuffer, &dest_rect, D3DTEXF_NONE);
}

void Draw_Phased_Ship()
{
	for (int n = 0; n < 4; n++)
	{
		D3DCOLOR phasecolor = D3DCOLOR_ARGB(rand() % 150, 0, 255, 255);
		int x = (int)player.x + rand() % 6 - 3;
		int y = (int)player.y + rand() % 6 - 3;
		Sprite_Transform_Draw(player_ship, x, y, player.width, player.height, 0, 1, 0.0f, 1.0f, phasecolor);
	}
}

void Draw_Overloading_Ship()
{
	for (int n = 0; n < 4; n++)
	{
		D3DCOLOR overload = D3DCOLOR_ARGB(150 + rand() % 100, 80, 255, 255);
		int x = (int)player.x + rand() % 12 - 6;
		int y = (int)player.y;
		Sprite_Transform_Draw(player_ship, x, y, player.width, player.height, 0, 1, 0.0f, 1.0f, overload);
	}
}

void Draw_Player_Shadows()
{
	D3DCOLOR shadowcolor = D3DCOLOR_ARGB(60, 0, 240, 240);
	if (currenttime > position_history_timer + 40)
	{
		position_history_timer = currenttime;
		position_history_index++;
		if (position_history_index > 7)
		{
			position_history_index = 7;
			for (int a = 1; a < 8; a++)
			{
				position_history[a - 1] = position_history[a];
			}
		}
		position_history[position_history_index].x = player.x;
		position_history[position_history_index].y = player.y;
	}
	for (int n = 0; n < 8; n++)
	{
		shadowcolor = D3DCOLOR_ARGB(20 + n * 10, 0, 240, 240);
		Sprite_Transform_Draw(player_ship, (int)position_history[n].x, (int)position_history[n].y, player.width, player.height, 1,1, 0.0f, 1.0f, shadowcolor);
	}
}

void Draw_Normal_Ship()
{
	if (player_state_previous != player_state)
	{
		for (int n = 0; n < 8; n++)
		{
			position_history[n].x = player.x;
			position_history[n].y = player.y;
		}
	}
	Draw_Player_Shadows();
	D3DCOLOR shipcolor = D3DCOLOR_ARGB(255, 0, 255, 255);
	Sprite_Transform_Draw(player_ship, (int)player.x, (int)player.y, player.width, player.height, 0,1, 0.0f, 1.0f, shipcolor);
}

void Draw_Viruses()
{
	for (int n = 0; n < VIRUSES; n++)
	{
		if (viruses[n].alive)
		{
			if (viruses[n].x > -96.0f&&viruses[n].x < SCREENW)
			{
				Sprite_Transform_Draw(virus_image, (int)viruses[n].x, viruses[n].y, viruses[n].width, viruses[n].height, 0, 1, 0.0f, viruses[n].scaling, viruses[n].color);
			}
		}
	}
}

void Draw_Bullets()
{
	D3DCOLOR bulletcolor = D3DCOLOR_ARGB(255, 255, 255, 255);
	for (int n = 0; n < BULLETS; n++)
	{
		if (bullets[n].alive)
		{
			Sprite_Transform_Draw(purple_fire, (int)bullets[n].x, (int)bullets[n].y, bullets[n].width, bullets[n].height, 0, 1, (float)toRadians(bullets[n].rotation), 1.0f, bulletcolor);

		}
	}
}

void Draw_Fragments()
{
	for (int n = 0; n < FRAGMENTS; n++)
	{
		if (fragments[n].alive)
		{
			Sprite_Transform_Draw(fragment_image, (int)fragments[n].x, (int)fragments[n].y, fragments[n].width, fragments[n].height, 0, 1, fragments[n].rotation, fragments[n].scaling, fragments[n].color);
		}
	}
}

void Draw_HUD()
{
	int y = SCREENH - 12;
	D3DCOLOR color = D3DCOLOR_ARGB(200, 255, 255, 255);
	D3DCOLOR debugcolor = D3DCOLOR_ARGB(255, 255, 255, 255);
	D3DCOLOR energycolor = D3DCOLOR_ARGB(200, 255, 255, 255);

	for (int n = 0; n < energy; n++)
	{
		Sprite_Transform_Draw(energy_slice, 10 + n * 2, 0, 1, 32, 0, 1, 0.0f, 1.0f, energycolor);
	}
	D3DCOLOR healthycolor = D3DCOLOR_ARGB(200, 255, 255, 255);
	for (int n = 0; n < healthy; n++)
	{
		Sprite_Transform_Draw(health_slice, 10 + n * 2, SCREENH - 21, 1, 20, 0, 1, 0.0f, 1.0f, healthycolor);
	}
	FontPrint(font, 900, 0, "SCORE" + ToString(score), color);
	FontPrint(font, 10, 0, "LIVES" + ToString(lives), color);
	FontPrint(debugfont, 0, y, "", debugcolor);
	FontPrint(debugfont, 0, y - 12, "Core FPS = " + ToString(corefps) + "(" + ToString(1000.0 / corefps) + "ms)",debugcolor);
	FontPrint(debugfont, 0, y - 24, "Screen FPS = " + ToString(screenfps), debugcolor);
	FontPrint(debugfont, 0, y - 36, "Ship X,Y = " + ToString(player.x) + "," + ToString(player.y), debugcolor);
	FontPrint(debugfont, 0, y - 48, "Bullets = " + ToString(bulletcount));
	FontPrint(debugfont, 0, y - 60, "Bullets scroll = " + ToString(scrollx), debugcolor);
	FontPrint(debugfont, 0, y - 72, "Virtual scroll = " + ToString(virtual_scrollx) + "/" + ToString(virtual_level_size));
	FontPrint(debugfont, 0, y - 84, "Fragment[0] = " + ToString(fragments[0].x) + "," + ToString(fragments[0].y));
}

void Draw_Mission_Briefind()
{
	const string briefing[] = {
		"Tasfjiafaolgjapsjpa  agjsp[ojg[aj;lnmasg j[gajf[",
		"asdasfafasfafw",
		"fdbgfdgewf",
		"cxbdfhsaSFASFefDVGSGHSFDedgdsg"
	};
	D3DCOLOR black = D3DCOLOR_XRGB(0, 0, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	D3DCOLOR green = D3DCOLOR_XRGB(60, 255, 255);
	int x = 50, y = 20;
	int array_size = sizeof(briefing) / sizeof(briefing[0]);

	for (int line = 0; line < array_size; line++)
	{
		FontPrint(font, 52, y + 2, briefing[line], black);
		FontPrint(font, 52, y, briefing[line], white);
		y += 20;
	}
	const string controls[] = {
		"SPACE       Fire weapon",
		"LSHITE      Change Bomb",
		"LCTRAL      Phasing Shield",
		"UP/W        Move Up",
		"DOWN/S      Move Down",
		"LEFT/A      Move Left",
		"RIGHT/D     Move Right"
	};
	x = SCREENW - 270;
	y = 160;
	array_size = sizeof(controls) / sizeof(controls[0]);
	for (int line = 0; line < array_size; line++)
	{
		FontPrint(font, x + 2, y + 2, controls[line], black);
		FontPrint(font, x, y, controls[line], green);
		y += 60;
	}
}

void Game_Run(HWND window)
{
	static int space_state = 0, esc_state = 0;
	
	if (!d3ddev)return;
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 100), 1.0f, 0);
	currenttime = timeGetTime();
	corecount += 1.0;

	if (currenttime < coretime + 1000)
	{
		corefps = corecount;
		corecount = 0.0;
		coretime = currenttime;
	}
	
	Draw_Background();
	if (currenttime > refresh + 16)
	{
		refresh = currenttime;
		DirectInput_Update();
		switch (game_state)
		{
		case GAME_STATES::PALYING:
			player_state = NORMAL;
			if (Key_Down(DIK_UP) || Key_Down(DIK_W))
				move_player(0, -1);
			if (Key_Down(DIK_DOWN) || Key_Down(DIK_S))
				move_player(0, 1);
			if (Key_Down(DIK_RIGHT) || Key_Down(DIK_D))
				move_player(1, 0);
			if (Key_Down(DIK_LEFT) || Key_Down(DIK_A))
				move_player(-1, 0);
			if (Key_Down(DIK_LCONTROL))
				player_state = PHASING;
			if (Key_Down(DIK_LSHIFT))
			{
				if (!player_overload)
					player_state_previous = OVERLOADING;
				else 
					player_state = OVERLOADING;
			}
			if (Key_Down(DIK_SPACE))
				player_shoot();
			Update_Background();
			Updata_bullets();
			Update_Viruses();
			Update_Fragments();
			Test_Virus_Collisions();

			if (vibrating > 0)
			{
				vibrating++;
				if (vibrating > vibration)
				{
					XInput_Vibrate(0, 0);
					vibrating = 0;
				}
			}
			break;

		case GAME_STATES::BRIEFING:
			Update_Background();
			healthy = 100;
			energy = 100;
			lives = 3;
			if (Key_Down(DIK_SPACE))
			{
				space_state = 1;
			}
			else
			{
				if (space_state == 1)
				{
					game_state = GAME_STATES::PALYING;
					space_state = 0;
				}
			}
			break;
		}
		screencount += 1.0;
		if (currenttime > screentime + 1000)
		{
			screenfps = screencount;
			screencount = 0.0;
			screentime = currenttime;
		}
		if (Key_Down(DIK_F1))firepower = 1;
		if (Key_Down(DIK_F2))firepower = 2; 
		if (Key_Down(DIK_F3))firepower = 3;
		if (Key_Down(DIK_F4))firepower = 4;
		if (Key_Down(DIK_F5))firepower = 5;
		
		if (Key_Down(DIK_E))
		{
			add_energy(1.0);
		}
		if (Key_Down(VK_ESCAPE))
			gameover = true;
	}

	Draw_Background();
	if (d3ddev->BeginScene())
	{
		obj->Begin(D3DXSPRITE_ALPHABLEND);
		switch (game_state)
		{
		case GAME_STATES::PALYING:
			switch (player_state)
			{
			case PHASING: Draw_Phased_Ship(); break;
			case OVERLOADING: Draw_Overloading_Ship(); break;
			case NORMAL: Draw_Normal_Ship(); break;
			}
			player_state_previous = player_state;
			Draw_Viruses();
			Draw_Bullets();
			Draw_Fragments();
			Draw_HUD();
			break;
		case GAME_STATES::BRIEFING:
			Draw_Mission_Briefind();
			break;
		}
		//d3ddev->ColorFill(background, NULL, D3DCOLOR_XRGB(0, 0, 0));
		obj->End();
		d3ddev->EndScene();
		d3ddev->Present(NULL, NULL, NULL, NULL);
	}
}