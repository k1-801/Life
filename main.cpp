#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "UTF8.h"

/* Path divider */
#ifdef __WIN32
	const char d = '\\';
#else
	const char d = '/';
#endif // __WIN32
/* Life save file signature */

const std::string mark("&lif$\0");

typedef std::vector<bool> row;
typedef std::vector<row> field;

void SDL_f()
{
	printf("SDL has crashed: %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
}

std::string AppPath()
{
	static std::string path;
    if(path.empty())
	{
		char* raw = SDL_GetBasePath();
		if(!raw)
			SDL_f();
		path = raw;
		SDL_free(raw);
	}
	return path;
}

SDL_Texture* GetTex(SDL_Renderer* renderer, std::string tn)
{
	std::string fname = AppPath() + tn;
	SDL_Surface* surf = NULL;
	surf = IMG_Load(fname.c_str());
	if(!surf)
		SDL_f();
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if(!tex)
		SDL_f();
	return tex;
}

SDL_Texture* TexTex(SDL_Renderer* renderer, std::string text, TTF_Font* font, uint8_t size)
{
	SDL_Surface* surf = NULL;
	SDL_Color color = {255, 255, 255, 255};
	surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if(!surf)
		SDL_f();
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	if(!tex)
		SDL_f();
	return tex;
}

void Iconify(SDL_Window *window, SDL_Renderer *renderer)
{
	SDL_Surface* ca = NULL;
	SDL_Surface* cd = NULL;
	std::string fname = AppPath() + "Resources/ca.png";
	ca = IMG_Load(fname.c_str());
	if(!ca)
		SDL_f();
	fname = AppPath() + "Resources/cd.png";
	cd = IMG_Load(fname.c_str());
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* ic = SDL_CreateRGBSurface(0, 32, 32, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
    SDL_Surface* ic = SDL_CreateRGBSurface(0, 32, 32, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
	uint8_t i, j;
	for(i = 0; i < 5; ++i)
	{
		for(j = 0; j < 5; ++j)
		{
			if((i != 0 && i != 4) || (j != 0 && j != 4))
			{
				SDL_Rect ir = {i * 8 -4, j * 8 - 4, 8, 8};
				SDL_Surface* ct;
				if((j == 1 && i == 2) || (j == 2 && i == 3) || (j == 3 && i >= 1 && i <= 3))
					ct = ca;
				else
					ct = cd;
				SDL_BlitSurface(ct, NULL, ic, &ir);
			}
		}
	}
	SDL_SetWindowIcon(window, ic);
	SDL_FreeSurface(ca);
	SDL_FreeSurface(cd);
	SDL_FreeSurface(ic);
}

void ClearField(field &f)
{
	uint64_t i;
	uint64_t n = f.size(), m = 0;
	if(n) m = f[0].size();
	row tmp;
	for(i = 0; i < m; ++i)
	{
        tmp.push_back(false);
	}
	for(i = 0; i < n; ++i)
	{
        f[i] = tmp;
	}
}

void ResizeField(field &f, uint64_t x, uint64_t y)
{
	row tmp;
	field b;
	uint64_t i, j;
	uint64_t n = f.size(), m = 0;
	if(n) m = f[0].size();
    for(i = 0; i < n && i < x; ++i)
	{
		for(j = 0; j < m && j < y; ++j)
		{
			tmp.push_back(f[i][j]);
		}
		for(; j < y; ++j)
		{
			tmp.push_back(false);
		}
		b.push_back(tmp);
		tmp.clear();
	}
	for(i = 0; i < y; ++i)
	{
		tmp.push_back(false);
	}
	for(i = n; i < x; ++i)
	{
		b.push_back(tmp);
	}
	f = b;
}

bool LifeStep(field &f, bool &ch)
{
	row tmp;
	field b;
	int64_t i, j;
	int64_t n = f.size(), m = 0;
	bool ne = false;
	if(n) m = f[0].size();

	uint8_t c, k;
	int8_t mx[8] = { 0,  1, 1, 1, 0, -1, -1, -1};
	int8_t my[8] = {-1, -1, 0, 1, 1,  1,  0, -1};

	ResizeField(b, n, m);

	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < m; ++j)
		{
			c = 0;
			for(k = 0; k < 8; ++k)
			{
                if(i + mx[k] >= 0 && i + mx[k] < n && j + my[k] >= 0 && j + my[k] < m)
				{
					if(f[i + mx[k]][j + my[k]])
					{
						++c;
					}
				}
			}
			if(c == 3 || (c == 2 && f[i][j]))
			{
				b[i][j] = true;
				ne = true;
			}
			if(b[i][j] != f[i][j])
			{
				ch = true;
			}
		}
	}
	f = b;
	return ne;
}

void Display(SDL_Renderer *renderer, TTF_Font* font, field &f, int rp, uint8_t speed, bool run, bool s1r, bool s2r, bool s3r, bool s4r, bool s5r, bool s6r)
{
	/* Cell */
	SDL_Texture *ct;
	static SDL_Texture *ca;
	static SDL_Texture *cd;
	if(!ca) ca = GetTex(renderer, "Resources/ca.png");
	if(!cd) cd = GetTex(renderer, "Resources/cd.png");

	/* Button 1 - run/stop */
	SDL_Texture *s1;
	static const SDL_Rect sr1 = {0, 0, 32, 32};
	static SDL_Texture *b1bs;
	static SDL_Texture *b1rs;
	static SDL_Texture *b1b;
	static SDL_Texture *b1r;
	if(!b1bs) b1bs = GetTex(renderer, "Resources/b1bs.png");
	if(!b1rs) b1rs = GetTex(renderer, "Resources/b1rs.png");
	if(!b1b ) b1b  = GetTex(renderer, "Resources/b1b.png" );
	if(!b1r ) b1r  = GetTex(renderer, "Resources/b1r.png" );
	if(s1r)
		if(run)
			s1 = b1rs;
		else
			s1 = b1r;
	else
		if(run)
			s1 = b1bs;
		else
			s1 = b1b;

	/* Button 2 - open */
	SDL_Texture *s2;
	static const SDL_Rect sr2 = {32, 0, 32, 32};
	static SDL_Texture *b2b;
	static SDL_Texture *b2r;
	if(!b2b) b2b = GetTex(renderer, "Resources/b2b.png");
	if(!b2r) b2r = GetTex(renderer, "Resources/b2r.png");
	if(s2r)
		s2 = b2r;
	else
		s2 = b2b;

	/* Button 3 - save */
	SDL_Texture *s3;
	static const SDL_Rect sr3 = {64, 0, 32, 32};
	static SDL_Texture *b3b;
	static SDL_Texture *b3r;
	if(!b3b) b3b = GetTex(renderer, "Resources/b3b.png");
	if(!b3r) b3r = GetTex(renderer, "Resources/b3r.png");
	if(s3r)
		s3 = b3r;
	else
		s3 = b3b;

	/* Button 4 - save as */
	SDL_Texture *s4;
	static const SDL_Rect sr4 = {96, 0, 32, 32};
	static SDL_Texture *b4b;
	static SDL_Texture *b4r;
	if(!b4b) b4b = GetTex(renderer, "Resources/b4b.png");
	if(!b4r) b4r = GetTex(renderer, "Resources/b4r.png");
	if(s4r)
		s4 = b4r;
	else
		s4 = b4b;

	/* Button 5 - config */
	SDL_Texture *s5;
	static const SDL_Rect sr5 = {128, 0, 32, 32};
	static SDL_Texture *b5b;
	static SDL_Texture *b5r;
	if(!b5b) b5b = GetTex(renderer, "Resources/b5b.png");
	if(!b5r) b5r = GetTex(renderer, "Resources/b5r.png");
	if(s5r)
		s5 = b5r;
	else
		s5 = b5b;

	/* Button 6 - clear */
	SDL_Texture *s6;
	static const SDL_Rect sr6 = {160, 0, 32, 32};
	static SDL_Texture *b6b;
	static SDL_Texture *b6r;
	if(!b6b) b6b = GetTex(renderer, "Resources/b6b.png");
	if(!b6r) b6r = GetTex(renderer, "Resources/b6r.png");
	if(s6r)
		s6 = b6r;
	else
		s6 = b6b;

	/* Speed regulator */
	static SDL_Texture *regulator;
	if(!regulator) regulator = GetTex(renderer, "Resources/regulator.png");
	SDL_Rect sra = {rp, 0, 128, 32};
	SDL_Rect srb = {rp + speed * 16 + 4, 12, 8, 8};

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, s1, NULL, &sr1);
	SDL_RenderCopy(renderer, s2, NULL, &sr2);
	SDL_RenderCopy(renderer, s3, NULL, &sr3);
	SDL_RenderCopy(renderer, s4, NULL, &sr4);
	SDL_RenderCopy(renderer, s5, NULL, &sr5);
	SDL_RenderCopy(renderer, s6, NULL, &sr6);
	SDL_RenderCopy(renderer, regulator, NULL, &sra);
	SDL_RenderCopy(renderer, ca, NULL, &srb);

    int i, j;
	int64_t n = f.size(), m = 0;
	if(n) m = f[0].size();

	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < m; ++j)
		{
			SDL_Rect cr = {i * 8, j * 8 + 32, 8, 8};
			if(f[i][j])
				ct = ca;
			else
				ct = cd;
            SDL_RenderCopy(renderer, ct, NULL, &cr);
		}
	}
	SDL_RenderPresent(renderer);
}

void Up(std::string &p)
{
	int64_t i;
	for(i = p.length() - 2; i >= 0; --i)
		if(p[i] == d)
			break;
	if(i < 0)
		return;
	p = p.substr(0, i + 1);
}

void Scroll(int64_t &counter, int32_t n, uint64_t size, uint64_t wh)
{
    if(n > counter)
	{
		counter = 0;
		return;
	}
	counter -= n;
	int64_t mx = size - (wh - 48) / 16;
	if(mx < 0)
		mx = 0;
	if(counter > mx)
		counter = mx;
}

bool Read(std::string fname, field &f)
{
	SDL_RWops* file = SDL_RWFromFile(fname.c_str(), "rb");
	if(!file)
		return false;
    char t_m[7] = "";
    SDL_RWread(file, t_m, 6, 1);
	bool f_m = (mark == t_m);
	uint64_t n, m, i, j, k;
	char raw;
	if(f_m)
	{
        ClearField(f);
        if(!SDL_RWread(file, &n, 8, 1)) return false;
        if(!SDL_RWread(file, &m, 8, 1)) return false;
        ResizeField(f, n, m);
        for(j = 0; j < m; ++j)
		{
            for(i = 0; i < n / 8; ++i)
			{
				if(!SDL_RWread(file, &raw, 1, 1)) return false;
				for(k = 0; k < 8 && i * 8 + k < n; ++k)
				{
					f[i * 8 + k][j] = (raw & (1 << k)) >> k;
				}
			}
		}
	}
    SDL_RWclose(file);
	return f_m;
}

bool Save(std::string fname, field &f)
{
	SDL_RWops* file = SDL_RWFromFile(fname.c_str(), "wb");
	if(!file)
		return false;
    bool f_m = SDL_RWwrite(file, mark.c_str(), 6, 1);
	if(f_m)
	{
		uint64_t n = f.size(), m = 0, i, j, k;
		char raw;
		if(n)
			m = f[0].size();
        if(!SDL_RWwrite(file, &n, 8, 1)) return false;
        if(!SDL_RWwrite(file, &m, 8, 1)) return false;
        for(j = 0; j < m; ++j)
		{
			for(i = 0; i < n / 8; ++i)
			{
				raw = 0;
				for(k = 0; k < 8 && i * 8 + k < n; ++k)
					if(f[i * 8 + k][j])
						raw |= (1 << k);
                if(!SDL_RWwrite(file, &raw, 1, 1)) return false;
			}
		}
	}
	uint64_t en = 0;
	if(!SDL_RWwrite(file, &en, 1, 1)) return false;
	SDL_RWclose(file);
	return f_m;
}

std::string FileDialog(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font, bool save, field &f, std::string s_path)
{
	static SDL_Texture* dir ; // Directory image
	static SDL_Texture* lif ; // Life file image
	static SDL_Texture* file; // File image
	static SDL_Texture* bf1b; // Blue close button
	static SDL_Texture* bf1r; // Red
	static SDL_Texture* bf2b; // Blue up scroll arrow
	static SDL_Texture* bf2r; // Red
	static SDL_Texture* bf3b; // Blue down scroll arrow
	static SDL_Texture* bf3r; // Red
	static SDL_Texture* bf4b; // Blue scroller
	static SDL_Texture* bf4r; // Red
	static SDL_Texture* bf5b; // Blue OK button (start button)
	static SDL_Texture* bf5r; // Red
	static SDL_Texture* bf6b; // Blue CANCEL button (stop button)
	static SDL_Texture* bf6r; // Red
	static SDL_Texture* frow; // File row
	static SDL_Texture* srow; // Selected file row

	if(!dir ) dir  = GetTex(renderer, "Resources/dir.png" );
	if(!lif ) lif  = GetTex(renderer, "Resources/lif.png" );
	if(!file) file = GetTex(renderer, "Resources/file.png");
	if(!bf1b) bf1b = GetTex(renderer, "Resources/bf1b.png");
	if(!bf1r) bf1r = GetTex(renderer, "Resources/bf1r.png");
	if(!bf2b) bf2b = GetTex(renderer, "Resources/bf2b.png");
	if(!bf2r) bf2r = GetTex(renderer, "Resources/bf2r.png");
	if(!bf3b) bf3b = GetTex(renderer, "Resources/bf3b.png");
	if(!bf3r) bf3r = GetTex(renderer, "Resources/bf3r.png");
	if(!bf4b) bf4b = GetTex(renderer, "Resources/bf4b.png");
	if(!bf4r) bf4r = GetTex(renderer, "Resources/bf4r.png");
	if(!bf5b) bf5b = GetTex(renderer, "Resources/b1b.png" );
	if(!bf5r) bf5r = GetTex(renderer, "Resources/b1r.png" );
	if(!bf6b) bf6b = GetTex(renderer, "Resources/b1bs.png");
	if(!bf6r) bf6r = GetTex(renderer, "Resources/b1rs.png");
	if(!frow) frow = GetTex(renderer, "Resources/frow.png");
	if(!srow) srow = GetTex(renderer, "Resources/srow.png");

	bool run = true, s = false, se = false, quit = false; // run is true while FD is opened, if quit is true - after closing FD the program will stop.
	bool edp = false, edn = true;
	bool s1r = false, s2r = false, s3r = false, s4r = false, s5r = false, s6r = false;
	int64_t scroll = 0, i;
	int64_t mx = 0, my = 0;
	int ww, wh, lww, lwh;
	SDL_GetWindowSize(window, &ww, &wh);
	lww = ww;
	lwh = wh;

	std::string path;
	std::string name;
	if(s_path.empty())
	{
		if(save)
		{
			path = AppPath();
			name = "New.lif";
		}
		else
		{
			path = AppPath() + "Examples" + d;
			name = "*.lif";
		}
	}
	else
	{
		// Get the path and name from s_path
		for(i = s_path.length() - 2; i >= 0; --i)
			if(s_path[i] == d)
				break;
		path = s_path.substr(0, i + 1);
		name = s_path.substr(i + 1, s_path.length() - i);
	}

	/* Start text input for name */
	SDL_StopTextInput();
	SDL_Rect textfield = {16, wh - 14, ww - 32, 12};
	SDL_SetTextInputRect(&textfield);
	SDL_Event tmpe;
	tmpe.type = SDL_TEXTEDITING;
	snprintf(tmpe.edit.text, 32, "%s", name.c_str());
	SDL_PushEvent(&tmpe);
	SDL_StartTextInput();

	while(run)
	{
		std::vector <std::string> files(1, "..");
		std::vector <bool> is_dir(1, true), is_lif(1, false);

		/* Read target directory */
		DIR* pdir = opendir(path.c_str());
		while(!pdir)
		{
            std::string op = path;
            Up(path);
            if(path == op)
				exit(EXIT_FAILURE);
            pdir = opendir(path.c_str());
		}

		dirent* de;
		while(de = readdir(pdir))
		{
			std::string tmp = GetDname(de);
			if(tmp == "." || tmp == "..")
				continue;

			files.push_back(tmp);
			std::string sub_p = path + tmp;
            DIR* sub_d = opendir(sub_p.c_str());
            if(sub_d)
			{
				is_dir.push_back(true);
				is_lif.push_back(false);
				closedir(sub_d);
			}
			else
			{
				is_dir.push_back(false);
				if(tmp.length() > 5)
				{
					std::string ext = tmp.substr(tmp.length() - 4, 4);
					is_lif.push_back(ext == ".lif");
				}
			}
		}
		closedir(pdir);
		Scroll(scroll, 0, files.size(), wh);

		int sbp = 32;
		int sbs = wh - 80;

		bool co = files.size() > ((wh - 48) / 16);
		if(co)
		{
			sbp = (float)((wh - 80) * scroll   ) / (float)(files.size()) + 32;
			sbs = (float)((wh - 80) * (wh - 48)) / (float)(files.size()) / 16 + 1;
		}

		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
            switch(e.type)
            {
				case SDL_QUIT:
					quit = true;
					run = false;
					break;
				case SDL_WINDOWEVENT:
					if(e.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						quit = true;
						run = false;
					}
					if(e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
                        ww = e.window.data1;
                        wh = e.window.data2;
					}
					break;
				case SDL_MOUSEMOTION:
					s1r = false;
					s2r = false;
					s3r = false;
					s4r = false;
					s5r = false;
					s6r = false;
					mx = e.motion.x;
					my = e.motion.y;
					if(mx > ww - 16 && mx < ww)
					{
						/* Close button */
						if(my < 16 && my > 0)
							s1r = true;
						/* Scrollbar */
						if(co && my >= 16 && my < 32)
							s2r = true;
						if(co && my >= sbp && my < sbp + sbs)
							s3r = true;
						if(co && se && my >= 16 && my < wh - 48)
							Scroll(scroll, (float)(sbp - my + sbs / 2) / (float)(wh - 80) * files.size(), files.size(), wh);
						if(co && my >= wh - 48 && my < wh - 32)
							s4r = true;
					}
					if(my >= wh - 32)
					{
						if(mx >= ww - 64 && mx < ww - 32)
							s5r = true;
						if(mx >= ww - 32 && mx < ww)
							s6r = true;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					mx = e.button.x;
					my = e.button.y;
					/* Catch text input start!!! */
					if(my >= 0 && my < 16 && mx >= 0 && mx <= ww - 16)
					{
						/* Path input */
						SDL_StopTextInput();
						SDL_Rect textfield = {16, 2, ww - 32, 12};
						SDL_SetTextInputRect(&textfield);
						SDL_Event tmpe;
						tmpe.type = SDL_TEXTEDITING;
						snprintf(tmpe.edit.text, 32, "%s", name.c_str());
						SDL_PushEvent(&tmpe);
						SDL_StartTextInput();
						edp = true;
						edn = false;
					}
					else
					{
						if(my >= wh - 16 && my < wh && mx >= 0 && mx < ww - 64)
						{
							/* Name input */
							SDL_StopTextInput();
							SDL_Rect textfield = {16, wh - 14, ww - 32, 12};
							SDL_SetTextInputRect(&textfield);
							SDL_Event tmpe;
							tmpe.type = SDL_TEXTEDITING;
							snprintf(tmpe.edit.text, 32, "%s", path.c_str());
							SDL_PushEvent(&tmpe);
							SDL_StartTextInput();
							edp = false;
							edn = true;
						}
						else
						{
							/* No input :) */
							SDL_StopTextInput();
							SDL_SetTextInputRect(NULL);
							edp = false;
							edn = false;
						}
					}
					/* Close button and scrollbar */
					if(mx > ww - 16)
					{
						if(my < 16)
							run = false;
						if(my >= 16 && my < 32)
							Scroll(scroll, 1, files.size(), wh);;
						if(my >= 32 && my < wh - 48)
						{
							se = true;
							Scroll(scroll, (float)(sbp - my + sbs / 2) / (float)(wh - 80) * files.size(), files.size(), wh);
						}
						if(my >= wh - 48 && my < wh - 32)
							Scroll(scroll, -1, files.size(), wh);
					}
					else
					{
                        if(my >= 16 && my < wh - 32)
						{
                            uint64_t sel = (my - 16) / 16 + scroll;
                            if(files.size() > sel)
							{
								if(s && name == files[sel])
								{
									if(is_dir[sel])
									{
										if(name == "..")
											Up(path);
										else
											path += name + d;
										s = false;
										scroll = 0;
									}
									else
									{
                                        bool r = false;
                                        if(save)
										{
											r = Save(path + name, f);
										}
										else
										{
											field backup = f;
											r = Read(path + name, f);
											if(r)
											{
												uint64_t n = f.size(), m = 0;
												if(n) m = f[0].size();
												SDL_SetWindowSize(window, n * 8, m * 8 + 32);
											}
											else
											{
												f = backup;
											}
										}
										if(r)
										{
											return path + name;
										}
										else
										{
                                            s = false;
										}
									}
								}
								else
								{
									name = files[sel];
									s = true;
								}
							}
						}
						if(my >= wh - 32 && my < wh)
						{
							/* OK button */
							if(mx >= ww - 64 && mx < ww - 32)
							{
								bool found = false, r = false;
                                for(i = 0; i < files.size(); ++i)
								{
                                    if(files[i] == name)
                                    {
                                    	found = true;
										break;
                                    }
								}
								if(found && is_dir[i])
								{
									if(!i)
										Up(path);
									else
                                        path += name + d;
									s = false;
									scroll = 0;
								}
								if(!found)
								{
									for(i = 1; i < name.length(); ++i)
									{
                                        if(name[i] == '.')
										{
											found = true;
											break;
										}
									}
								}
								if(!found)
								{
									name += ".lif";
								}
								if(save)
								{
									r = Save(path + name, f);
								}
								else
								{
                                    field backup = f;
                                    r = Read(path + name, f);
                                    if(r)
									{
										uint64_t n = f.size(), m = 0;
										if(n) m = f[0].size();
										SDL_SetWindowSize(window, n * 8, m * 8 + 32);
									}
                                    if(!r)
									{
										f = backup;
									}
								}
								if(r)
								{
									return path + name;
								}
							}
							/* Cancel button */
							if(mx >= ww - 32 && mx < ww)
							{
								run = false;
							}
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					se = false;
					break;
				case SDL_MOUSEWHEEL:
					if(e.wheel.y)
						Scroll(scroll, e.wheel.y, files.size(), wh);
					break;
				case SDL_TEXTINPUT:
					if(edp)
						path += e.text.text;
					if(edn)
						name += e.text.text;
					break;
				case SDL_TEXTEDITING:
					if(edp)
						path = e.edit.text;
					if(edn)
						name = e.edit.text;
					break;
            }
		}
		SDL_RenderClear(renderer);

		/* Button 1 - close button */
		SDL_Rect sr1 = {ww - 16, 0, 16, 16};
		SDL_Texture* b1;
		if(s1r)
			b1 = bf1r;
		else
			b1 = bf1b;
		SDL_RenderCopy(renderer, b1, NULL, &sr1);

		/* Button 2 - scroll up */
		SDL_Rect sr2 = {ww - 16, 16, 16, 16};
		SDL_Texture* b2;
		if(s2r)
			b2 = bf2r;
		else
			b2 = bf2b;
		SDL_RenderCopy(renderer, b2, NULL, &sr2);

		/* Button 3 - scroll bar */
		SDL_Rect sr3 = {ww - 16, sbp, 16, sbs};
		SDL_Texture* b3;
		if(s3r)
			b3 = bf3r;
		else
			b3 = bf3b;
		SDL_RenderCopy(renderer, b3, NULL, &sr3);

		/* Button 4 - scroll down*/
		SDL_Rect sr4 = {ww - 16, wh - 48, 16, 16};
		SDL_Texture* b4;
		if(s4r)
			b4 = bf4r;
		else
			b4 = bf4b;
		SDL_RenderCopy(renderer, b4, NULL, &sr4);

		/* Button 5 - OK button*/
		SDL_Rect sr5 = {ww - 64, wh - 32, 32, 32};
		SDL_Texture* b5;
		if(s5r)
			b5 = bf5r;
		else
			b5 = bf5b;
		SDL_RenderCopy(renderer, b5, NULL, &sr5);

		/* Button 6 - CANCEL button*/
		SDL_Rect sr6 = {ww - 32, wh - 32, 32, 32};
		SDL_Texture* b6;
		if(s6r)
			b6 = bf6r;
		else
			b6 = bf6b;
		SDL_RenderCopy(renderer, b6, NULL, &sr6);

		for(i = scroll; i < files.size() && i < (wh - 48) / 16 + scroll; ++i)
		{
			SDL_Texture* ft;
			SDL_Texture* fi;
			SDL_Texture* fn;

			fn = TexTex(renderer, files[i], font, 10);
			int nw, nh;
			SDL_QueryTexture(fn, 0, 0, &nw, &nh);

			SDL_Rect tr = {0, 16 * (i - scroll) + 16, ww - 16, 16};
			SDL_Rect ir = {0, 16 * (i - scroll) + 16, 16, 16};
			SDL_Rect nr = {16, 16 * (i - scroll) + 18, nw, nh};

			if(s && files[i] == name)
				ft = srow;
			else
				ft = frow;
			if(is_dir[i])
				fi = dir;
			else
				if(is_lif[i])
					fi = lif;
				else
					fi = file;
			SDL_RenderCopy(renderer, ft, NULL, &tr);
			SDL_RenderCopy(renderer, fi, NULL, &ir);
			SDL_RenderCopy(renderer, fn, NULL, &nr);
			SDL_DestroyTexture(fn);
		}

		SDL_Texture* fp = TexTex(renderer, path, font, 10);
		SDL_Texture* fn = TexTex(renderer, name, font, 10);
		int pw, ph, nw, nh;
		SDL_QueryTexture(fp, 0, 0, &pw, &ph);
		SDL_QueryTexture(fn, 0, 0, &nw, &nh);

		SDL_Rect dpr = {0, 0, ww - 16, 16};
		SDL_Rect tpr = {16, 2, pw, ph};
		SDL_Rect dnr = {0, wh - 16, ww - 64, 16};
		SDL_Rect tnr = {16, wh - 14, nw, nh};
		SDL_RenderCopy(renderer, srow, NULL, &dpr);
		SDL_RenderCopy(renderer, fp, NULL, &tpr);
		SDL_RenderCopy(renderer, srow, NULL, &dnr);
		SDL_RenderCopy(renderer, fn, NULL, &tnr);
		SDL_RenderPresent(renderer);
	}
	if(quit)
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        exit(EXIT_SUCCESS);
	}
	SDL_SetWindowSize(window, lww, lwh);
	return "";
}

int main(int argc, char** argv)
{
	bool run = true, sim = false, se = false, fe = false, fet = false, changed = false;
	bool s1r = false, s2r = false, s3r = false, s4r = false, s5r = false, s6r = false; // When mouse gets over one of buttons, it's variable gets true
	uint64_t mx = 0, my = 0, ww = 640, wh = 480, tick = 0, rp = 512;
	uint8_t speed = 0;
	std::string file = "";
	std::string title = "Life";
	if(SDL_Init(SDL_INIT_EVERYTHING))
		SDL_f();
	if(!IMG_Init(IMG_INIT_PNG))
		SDL_f();
	if(TTF_Init())
		SDL_f();
	SDL_Window* window = NULL;
	window = SDL_CreateWindow("Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(!window)
		SDL_f();
	SDL_Renderer* renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(!renderer)
		SDL_f();
	Iconify(window, renderer);
	SDL_SetWindowMinimumSize(window, 360, 96);
	TTF_Font* font = TTF_OpenFont((AppPath() + "Resources/font.ttf").c_str(), 10);
	if(!font)
		SDL_f();

	field f;
	ResizeField(f, 80, 56);
	while(run)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					run = false;
					break;
				case SDL_WINDOWEVENT:
                    switch(event.window.event)
                    {
						case SDL_WINDOWEVENT_CLOSE:
							run = false;
							break;
						case SDL_WINDOWEVENT_RESIZED:
							ww = event.window.data1;
							wh = event.window.data2;
							ResizeField(f, ww / 8, (wh - 32) / 8);
							rp = ww - 128;
							break;
                    }
				case SDL_MOUSEMOTION:
					mx = event.motion.x;
					my = event.motion.y;
					s1r = false;
					s2r = false;
					s3r = false;
					s4r = false;
					s5r = false;
					s6r = false;
					if(my < 32 && !fe && !se)
					{
						if(mx < 32)
						{
							s1r = true;
						}
						if(mx >= 32 && mx < 64)
						{
							s2r = true;
						}
						if(mx >= 64 && mx < 96)
						{
							s3r = true;
						}
						if(mx >= 96 && mx < 128)
						{
							s4r = true;
						}
						if(mx >= 128 && mx < 160)
						{
							s5r = true;
						}
						if(mx >= 160 && mx < 192)
						{
							s6r = true;
						}
					}
					if(fe && my >= 32)
					{
						uint64_t x = mx / 8;
						uint64_t y = (my - 32) / 8;
						uint64_t n = f.size(), m = 0;
						if(n)
							m = f[0].size();
						if(x < n && y < m)
						{
							f[x][y] = fet;
						}
					}
					if(se && mx >= rp + 6 && mx < ww)
					{
						speed = (mx - rp - 4) / 16;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					mx = event.button.x;
					my = event.button.y;
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						if(my < 32)
						{
							if(mx < 32)
							{
								sim = !sim;
							}
							if(mx >= 32 && mx < 64)
							{
								sim = false;
								std::string tmp = FileDialog(window, renderer, font, false, f, file);
								if(!tmp.empty())
								{
									file = tmp;
									changed = false;
								}
							}
							if(mx >= 64 && mx < 96)
							{
								sim = false;
								bool r = false;
								if(!file.empty())
									r = Save(file, f);
								if(!r)
								{
									std::string tmp = FileDialog(window, renderer, font, true, f, file);
									if(!tmp.empty())
									{
										file = tmp;
										r = true;
									}
								}
								if(r)
									changed = false;
							}
							if(mx >= 96 && mx < 128)
							{
								sim = false;
                                std::string tmp = FileDialog(window, renderer, font, true, f, file);
                                if(!tmp.empty())
								{
									file = tmp;
									changed = false;
								}
							}
							if(mx >= 160 && mx < 192)
							{
								sim = false;
								ClearField(f);
							}
							if(mx > rp + 6)
							{
								speed = (mx - rp - 4) / 16;
                                se = true;
							}
						}
						else
						{
							uint64_t x = mx / 8;
							uint64_t y = (my - 32) / 8;
							uint64_t n = f.size(), m = 0;
							if(n)
								m = f[0].size();
							if(x < n && y < m)
							{
								changed = true;
								fe = true;
								fet = !f[x][y];
								f[x][y] = fet;
							}
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
						se = false;
						fe = false;
					break;
			}
		}
		if(sim && !fe)
		{
			++tick;
			if(tick >= speed)
			{
				if(!LifeStep(f, changed))
					sim = false;
				tick = 0;
			}
		}
		std::string ntitle;
		if(!file.empty())
			ntitle += '[' + file + ']';
		if(changed)
			ntitle += '*';
		if(changed || !file.empty())
			ntitle += " - ";
		ntitle += "Life";
		if(title != ntitle)
		{
			title = ntitle;
			SDL_SetWindowTitle(window, title.c_str());
		}
		Display(renderer, font, f, rp, speed, sim, s1r, s2r, s3r, s4r, s5r, s6r);
		SDL_Delay(10);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return EXIT_SUCCESS;
}
