#include "Menu.h"

enum Mode {
	MODE_NORMAL = 0,
	MODE_TEST_RADIO,
	MODE_TEST_INFRARED
};

Mode mode = MODE_NORMAL;

bool assignBase(IMenuInterface *nav, void* ctx) {
	BaseInfo *info = (BaseInfo*)ctx;
	//Serial.println("Assign " + String(info->id));
	nav->back();
}

bool assignPlayer(IMenuInterface *nav, void* ctx) {
	PlayerInfo *info = (PlayerInfo*)ctx;
	//Serial.println("Assign " + String(info->name));
	nav->back();
}

bool assignTeam(IMenuInterface *nav, void* ctx) {
	TeamInfo *info = (TeamInfo*)ctx;
	//Serial.println("Assign " + String(info->name));
	nav->back();
}

bool getBases(IMenuInterface *nav, void*) {
	Menu **submenus = new Menu*[gameParams.bases.length() + 1];
	submenus[0] = &Menu::Back;

	for (uint8_t i = 0; i < gameParams.bases.length(); i++) {
		BaseInfo *info = gameParams.bases.get(i);
		submenus[i + 1] = new Menu(("Base" + String(i)).c_str(), new Menu*[2] {
			&Menu::Back,
			new Menu("Assign", &assignBase, info, MENU_DISPOSE)
		}, 2, MENU_DISPOSE);
	}

	Menu *menu = new Menu("TEMP_BASES", submenus, gameParams.bases.length() + 1, MENU_DISPOSE);
	nav->push(menu);
	return true;
}

bool getPlayers(IMenuInterface *nav, void*) {
	Menu **submenus = new Menu*[gameParams.players.length() + 1];
	submenus[0] = &Menu::Back;

	for (uint8_t i = 0; i < gameParams.players.length(); i++) {
		PlayerInfo *info = gameParams.players.get(i);
		submenus[i + 1] = new Menu(info->name, new Menu*[2] {
			&Menu::Back,
			new Menu("Assign", &assignPlayer, info, MENU_DISPOSE)
		}, 2, MENU_DISPOSE);
	}

	Menu *menu = new Menu("TEMP_PLAYERS", submenus, gameParams.players.length() + 1, MENU_DISPOSE);
	nav->push(menu);
	return true;
}

bool getTeams(IMenuInterface *nav, void*) {
	Menu **submenus = new Menu*[gameParams.teams.length() + 1];
	submenus[0] = &Menu::Back;

	for (uint8_t i = 0; i < gameParams.teams.length(); i++) {
		TeamInfo *info = gameParams.teams.get(i);
		submenus[i + 1] = new Menu(info->name, new Menu*[2] {
			&Menu::Back,
			new Menu("Assign", &assignTeam, info, MENU_DISPOSE)
		}, 2, MENU_DISPOSE);
	}

	Menu *menu = new Menu("TEMP_TEAMS", submenus, gameParams.teams.length() + 1, MENU_DISPOSE);
	nav->push(menu);
	return true;
}

bool dumpLogs(IMenuInterface *nav, void*) {
	nav->back();
}

bool testRadio(IMenuInterface *nav, void*) {
	mode = MODE_TEST_RADIO;
}

bool testInfrared(IMenuInterface *nav, void*) {
	mode = MODE_TEST_INFRARED;
}

bool testOff(IMenuInterface *nav, void*) {
	nav->back();
	mode = MODE_NORMAL;
}

/* The menu structure */

Menu *gameMenu[] = {
	&Menu::Back
};
Menu *rulesMenu[] = {
	&Menu::Back
};
Menu *basesMenu[] {
	&Menu::Back
};
Menu *playersMenu[] = {
	&Menu::Back
};
Menu *teamsMenu[] = {
	&Menu::Back
};
Menu *logsMenu[] = {
	&Menu::Back,
	new Menu("Dump", &dumpLogs)
};
Menu *debugMenu[] = {
	&Menu::Back,
	new Menu("Test radio", &testRadio),
	new Menu("Test IR", &testInfrared),
	new Menu("Test off", &testOff)
};
Menu *root[] = {
	new Menu("Game", gameMenu, 1),
	new Menu("Rules", rulesMenu, 1),
	new Menu("Bases", basesMenu, 1, &getBases),
	new Menu("Players", playersMenu, 1, &getPlayers),
	new Menu("Teams", teamsMenu, 1, &getTeams),
	new Menu("Logs", logsMenu, 2),
	new Menu("Debug", debugMenu, 4)
};
Menu menu(root, 7);
