#include "Menu.h"

bool Menu::navigateBack(IMenuInterface* nav, void*) {
	return nav->back();
}

Menu Menu::Back = Menu("BACK", &Menu::navigateBack);

Menu::~Menu() {
	this->dispose();
}

void Menu::dispose() {
	if ((this->flags & MENU_DISPOSE) == MENU_DISPOSE) {
		Serial.println("Disposing " + String(this->title));
		if (this->submenus != 0) {
			for (uint8_t i = 0; i < this->numSubmenus; i++) {
				if ((this->submenus[i]->flags & MENU_DISPOSE) == MENU_DISPOSE) {
					delete this->submenus[i];
				}
			}
			delete[] this->submenus;
			this->submenus = 0;
		}
	}
}

bool IMenuInterface::next() {
	if (this->selectedIndex < this->stack[this->stackPos]->numSubmenus - 1) {
		this->selectedIndex++;
	}
	else {
		this->selectedIndex = 0;
	}
	return true;
}

bool IMenuInterface::prev() {
	if (this->selectedIndex > 0) {
		this->selectedIndex--;
	}
	else {
		this->selectedIndex = this->stack[this->stackPos]->numSubmenus - 1;
	}
	return true;
}

bool IMenuInterface::back() {
	if (this->stackPos == 0) {
		return false;
	}
	Menu *current = this->current();
	if ((current->flags & MENU_DISPOSE) == MENU_DISPOSE) {
		delete current;
	}
	this->stackPos--;
	return true;
}

void IMenuInterface::push(Menu *menu) {
	this->stack[this->stackPos + 1] = menu;
	this->stackPos++;
}

Menu *IMenuInterface::selected() {
	return this->stack[this->stackPos]->submenus[this->selectedIndex];
}

bool IMenuInterface::select()  {
	Menu *menu = this->selected();
	if (menu->callback != 0) {
		menu->callback(this, menu->ctx);
	}
	else {
		this->push(menu);
	}
	this->selectedIndex = 0;
	return true;
}

Menu *IMenuInterface::current() {
	return this->stack[this->stackPos];
}
