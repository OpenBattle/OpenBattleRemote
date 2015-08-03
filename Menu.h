#ifndef _MENU
#define _MENU

#include "Arduino.h"

class IMenuInterface;

enum MenuFlags
{
	MENU_DEFAULT = 0x00,
	MENU_DISPOSE = 0x01
};

class Menu
{
	static bool navigateBack(IMenuInterface* nav, void* ctx);
public:
	MenuFlags flags = MENU_DEFAULT;
	Menu** submenus = 0;
	uint8_t numSubmenus = 0;
	const char* title = 0;
	bool(*callback)(IMenuInterface*, void*) = 0;
	void* ctx = 0;

	Menu(MenuFlags flags = MENU_DEFAULT) : flags(flags) { }
	Menu(const char *title) : title(title) { }
	Menu(Menu** submenus, uint8_t numSubmenus) : submenus(submenus), numSubmenus(numSubmenus) { }
	Menu(const char *title, Menu** submenus, uint8_t numSubmenus, MenuFlags flags = MENU_DEFAULT) : title(title), submenus(submenus), numSubmenus(numSubmenus), flags(flags) { }
	Menu(const char *title, bool(*callback)(IMenuInterface*, void*), void* ctx = 0, MenuFlags flags = MENU_DEFAULT) : title(title), callback(callback), ctx(ctx), flags(flags) { }
	Menu(const char *title, Menu** submenus, uint8_t numSubmenus, bool(*callback)(IMenuInterface*, void*), void* ctx = 0, MenuFlags flags = MENU_DEFAULT) : 
		title(title), submenus(submenus), numSubmenus(numSubmenus), callback(callback), ctx(ctx), flags(flags) { }
	~Menu();
	void dispose();

	static Menu Back;
};

class IMenuInterface {
private:
	Menu** stack;
	uint8_t stackPos;
	uint8_t stackLen;
	Menu** submenus = 0;
	uint8_t numSubmenus = 0;
	uint8_t submenusLen = 0;
public:
	uint8_t selectedIndex = 0;
	IMenuInterface(Menu* menu, uint8_t stackSize = 8) : stackLen(stackSize), stackPos(0) {
		this->stack = new Menu*[stackSize];
		this->stack[0] = menu;
	}
	Menu *current();
	Menu *selected();
	bool next();
	bool prev();
	bool back();
	bool select();
	void push(Menu *menu);
};

#endif
