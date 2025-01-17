/*
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
 *
 * ET: Legacy
 * Copyright (C) 2012 Jan Simek <mail@etlegacy.com>
 *
 * This file is part of ET: Legacy - http://www.etlegacy.com
 *
 * ET: Legacy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ET: Legacy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ET: Legacy. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, Wolfenstein: Enemy Territory GPL Source Code is also
 * subject to certain additional terms. You should have received a copy
 * of these additional terms immediately following the terms and conditions
 * of the GNU General Public License which accompanied the source code.
 * If not, please request a copy in writing from id Software at the address below.
 *
 * id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
 */
/**
 * @file ui_menu.c
 * @brief String allocation/managment
 */

#include "ui_shared.h"
#include "ui_local.h"

void Menu_FadeMenuByName(const char *p, qboolean *bAbort, qboolean fadeOut)
{
	menuDef_t *menu = Menus_FindByName(p);

	if (menu)
	{
		int       i;
		itemDef_t *item;

		for (i = 0; i < menu->itemCount; i++)
		{
			item = menu->items[i];
			if (fadeOut)
			{
				item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
				item->window.flags &= ~WINDOW_FADINGIN;
			}
			else
			{
				item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
				item->window.flags &= ~WINDOW_FADINGOUT;
			}
		}
	}
}

void Menu_TransitionItemByName(menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, int time, float amt)
{
	itemDef_t *item;
	int       i;
	int       count = Menu_ItemsMatchingGroup(menu, p);

	for (i = 0; i < count; i++)
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL)
		{
			item->window.flags     |= (WINDOW_INTRANSITION | WINDOW_VISIBLE);
			item->window.offsetTime = time;
			memcpy(&item->window.rectClient, &rectFrom, sizeof(rectDef_t));
			memcpy(&item->window.rectEffects, &rectTo, sizeof(rectDef_t));
			item->window.rectEffects2.x = fabsf(rectTo.x - rectFrom.x) / amt;
			item->window.rectEffects2.y = fabsf(rectTo.y - rectFrom.y) / amt;
			item->window.rectEffects2.w = fabsf(rectTo.w - rectFrom.w) / amt;
			item->window.rectEffects2.h = fabsf(rectTo.h - rectFrom.h) / amt;
			Item_UpdatePosition(item);
		}
	}
}

void Menu_OrbitItemByName(menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time)
{
	itemDef_t *item;
	int       i;
	int       count = Menu_ItemsMatchingGroup(menu, p);

	for (i = 0; i < count; i++)
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL)
		{
			item->window.flags        |= (WINDOW_ORBITING | WINDOW_VISIBLE);
			item->window.offsetTime    = time;
			item->window.rectEffects.x = cx;
			item->window.rectEffects.y = cy;
			item->window.rectClient.x  = x;
			item->window.rectClient.y  = y;
			Item_UpdatePosition(item);
		}
	}
}

void Menu_Init(menuDef_t *menu)
{
	memset(menu, 0, sizeof(menuDef_t));
	menu->cursorItem = -1;
	menu->fadeAmount = DC->Assets.fadeAmount;
	menu->fadeClamp  = DC->Assets.fadeClamp;
	menu->fadeCycle  = DC->Assets.fadeCycle;
	// by default, do NOT use item hotkey mode
	menu->itemHotkeyMode = qfalse;
	Window_Init(&menu->window);
}

// @note Unused
itemDef_t *Menu_GetFocusedItem(menuDef_t *menu)
{
	if (menu)
	{
		int i;

		for (i = 0; i < menu->itemCount; i++)
		{
			if (menu->items[i]->window.flags & WINDOW_HASFOCUS)
			{
				return menu->items[i];
			}
		}
	}
	return NULL;
}

menuDef_t *Menu_GetFocused(void)
{
	int i;

	for (i = 0; i < menuCount; i++)
	{
		if ((Menus[i].window.flags & WINDOW_HASFOCUS) && (Menus[i].window.flags & WINDOW_VISIBLE))
		{
			return &Menus[i];
		}
	}
	return NULL;
}

void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down)
{
	if (menu)
	{
		int i;

		for (i = 0; i < menu->itemCount; i++)
		{
			if (menu->items[i]->special == feeder)
			{
				Item_ListBox_HandleKey(menu->items[i], (down) ? K_DOWNARROW : K_UPARROW, qtrue, qtrue);
				return;
			}
		}
	}
}

void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index, const char *name)
{
	if (menu == NULL)
	{
		if (name == NULL)
		{
			menu = Menu_GetFocused();
		}
		else
		{
			menu = Menus_FindByName(name);
		}
	}

	if (menu)
	{
		int i;

		for (i = 0; i < menu->itemCount; i++)
		{
			if (menu->items[i]->special == feeder)
			{
				if (index == 0)
				{
					listBoxDef_t *listPtr = (listBoxDef_t *)menu->items[i]->typeData;

					listPtr->cursorPos = 0;
					listPtr->startPos  = 0;
				}
				menu->items[i]->cursorPos = index;
				DC->feederSelection(menu->items[i]->special, menu->items[i]->cursorPos);
				return;
			}
		}
	}
}

qboolean Menus_AnyFullScreenVisible(void)
{
	int i;

	for (i = 0; i < menuCount; i++)
	{
		if ((Menus[i].window.flags & WINDOW_VISIBLE) && Menus[i].fullScreen)
		{
			return qtrue;
		}
	}
	return qfalse;
}

menuDef_t *Menus_ActivateByName(const char *p, qboolean modalStack)
{
	int       i;
	menuDef_t *m     = NULL;
	menuDef_t *focus = Menu_GetFocused();

	for (i = 0; i < menuCount; i++)
	{
		if (Q_stricmp(Menus[i].window.name, p) == 0)
		{
			m = &Menus[i];
			Menus_Activate(m);
			if (modalStack && (m->window.flags & WINDOW_MODAL))
			{
				if (modalMenuCount >= MAX_MODAL_MENUS)
				{
					Com_Error(ERR_DROP, "MAX_MODAL_MENUS exceeded");
				}
				modalMenuStack[modalMenuCount++] = focus;
			}
			break;  // found it, don't continue searching as we might unfocus the menu we just activated again.
		}
		else
		{
			Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_MOUSEOVER);
		}
	}
	Display_CloseCinematics();
	return m;
}

// menus
void Menu_UpdatePosition(menuDef_t *menu)
{
	int        i;
	float      x, y;
	float      xoffset = Cui_WideXoffset();
	Rectangle  *r;
	qboolean   fullscreenItem = qfalse;
	qboolean   fullscreenMenu = qfalse;
	qboolean   centered       = qfalse;
	const char *menuName      = NULL;
	const char *itemName      = NULL;

	if (menu == NULL)
	{
		return;
	}

	x = menu->window.rect.x;
	y = menu->window.rect.y;

	r              = &menu->window.rect;
	fullscreenMenu = (r->x == 0 && r->y == 0 && r->w == SCREEN_WIDTH && r->h == SCREEN_HEIGHT);
	centered       = (r->x == 16 && r->w == 608);
	menuName       = menu->window.name;

	Cui_WideRect(&menu->window.rect);

	for (i = 0; i < menu->itemCount; ++i)
	{
		itemName = menu->items[i]->window.name;
		// fullscreen menu/item..
		r              = &menu->items[i]->window.rectClient;
		fullscreenItem = (r->x == 0 && r->y == 0 && r->w == SCREEN_WIDTH && r->h == SCREEN_HEIGHT);

		// exclude background clouds as fullscreen item from Cui_WideRect(r) and adjust rect width
		if (!Q_stricmp(itemName, "clouds"))
		{
			r->w = r->w + 2 * xoffset;
		}
		else // all other items
		{
			if (fullscreenItem)
			{
				Cui_WideRect(r);
			}
		}

		// alignment..
		if ((fullscreenMenu && !fullscreenItem) || !Q_stricmp(menuName, "main") || !Q_stricmp(menuName, "ingame_main") || centered)
		{
			// align to right of screen..
			if (!Q_stricmp(itemName, "atvi_logo") ||
			    !Q_stricmp(itemName, "id_logo"))
			{
				Item_SetScreenCoords(menu->items[i], x + 2 * xoffset, y);
			}
			// horizontally centered..
			else if (!Q_stricmp(itemName, "etl_logo") ||
			         !Q_stricmp(itemName, "credits_etlegacy"))
			{
				Item_SetScreenCoords(menu->items[i], x + xoffset, y);
			}
			// normal (left aligned)..
			else if (!Q_stricmp(menuName, "main") || !Q_stricmp(menuName, "ingame_main"))
			{
				Item_SetScreenCoords(menu->items[i], x, y);

			}
			else
			{
				// horizontally centered..
				Item_SetScreenCoords(menu->items[i], x + xoffset, y);
			}
		}
		// normal (left aligned)..
		else
		{
			Item_SetScreenCoords(menu->items[i], x, y);
		}
	}
}

void Menu_PostParse(menuDef_t *menu)
{
	if (menu == NULL)
	{
		return;
	}
	if (menu->fullScreen)
	{
		menu->window.rect.x = 0;
		menu->window.rect.y = 0;
		menu->window.rect.w = SCREEN_WIDTH;
		menu->window.rect.h = SCREEN_HEIGHT;
	}
	Menu_UpdatePosition(menu);
}

itemDef_t *Menu_ClearFocus(menuDef_t *menu)
{
	int       i;
	itemDef_t *ret = NULL;

	if (menu == NULL)
	{
		return NULL;
	}

	for (i = 0; i < menu->itemCount; i++)
	{
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS)
		{
			ret                           = menu->items[i];
			menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;
		}

		if (menu->items[i]->window.flags & WINDOW_MOUSEOVER)
		{
			Item_MouseLeave(menu->items[i]);
			Item_SetMouseOver(menu->items[i], qfalse);
		}

		if (menu->items[i]->leaveFocus)
		{
			Item_RunScript(menu->items[i], NULL, menu->items[i]->leaveFocus);
		}
	}

	return(ret);
}

int Menu_ItemsMatchingGroup(menuDef_t *menu, const char *name)
{
	int  i;
	int  count = 0;
	char *pdest;
	int  wildcard = -1; // if wildcard is set, it's value is the number of characters to compare

	pdest = strstr(name, "*");   // allow wildcard strings (ex.  "hide nb_*" would translate to "hide nb_pg1; hide nb_extra" etc)
	if (pdest)
	{
		wildcard = pdest - name;
	}

	for (i = 0; i < menu->itemCount; i++)
	{
		if (wildcard != -1)
		{
			if (Q_strncmp(menu->items[i]->window.name, name, wildcard) == 0 || (menu->items[i]->window.group && Q_strncmp(menu->items[i]->window.group, name, wildcard) == 0))
			{
				count++;
			}
		}
		else
		{
			if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0))
			{
				count++;
			}
		}
	}

	return count;
}

itemDef_t *Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, const char *name)
{
	int  i;
	int  count = 0;
	char *pdest;
	int  wildcard = -1; // if wildcard is set, it's value is the number of characters to compare

	pdest = strstr(name, "*");   // allow wildcard strings (ex.  "hide nb_*" would translate to "hide nb_pg1; hide nb_extra" etc)
	if (pdest)
	{
		wildcard = pdest - name;
	}

	for (i = 0; i < menu->itemCount; i++)
	{
		if (wildcard != -1)
		{
			if (Q_strncmp(menu->items[i]->window.name, name, wildcard) == 0 || (menu->items[i]->window.group && Q_strncmp(menu->items[i]->window.group, name, wildcard) == 0))
			{
				if (count == index)
				{
					return menu->items[i];
				}
				count++;
			}
		}
		else
		{
			if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0))
			{
				if (count == index)
				{
					return menu->items[i];
				}
				count++;
			}
		}
	}
	return NULL;
}

itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p)
{
	int i;

	if (menu == NULL || p == NULL)
	{
		return NULL;
	}

	for (i = 0; i < menu->itemCount; i++)
	{
		if (Q_stricmp(p, menu->items[i]->window.name) == 0)
		{
			return menu->items[i];
		}
	}

	return NULL;
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow)
{
	itemDef_t *item;
	int       i;
	int       count = Menu_ItemsMatchingGroup(menu, p);

	for (i = 0; i < count; i++)
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL)
		{
			if (bShow)
			{
				item->window.flags |= WINDOW_VISIBLE;
			}
			else
			{
				if (item->window.flags & WINDOW_MOUSEOVER)
				{
					Item_MouseLeave(item);
					Item_SetMouseOver(item, qfalse);
				}

				item->window.flags &= ~WINDOW_VISIBLE;

				// stop cinematics playing in the window
				if (item->window.cinematic >= 0)
				{
					DC->stopCinematic(item->window.cinematic);
					item->window.cinematic = -1;
				}
			}
		}
	}
}

void Menu_FadeItemByName(menuDef_t *menu, const char *p, qboolean fadeOut)
{
	itemDef_t *item;
	int       i;
	int       count = Menu_ItemsMatchingGroup(menu, p);

	for (i = 0; i < count; i++)
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL)
		{
			if (fadeOut)
			{
				item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
				item->window.flags &= ~WINDOW_FADINGIN;
			}
			else
			{
				item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
				item->window.flags &= ~WINDOW_FADINGOUT;
			}
		}
	}
}

menuDef_t *Menus_FindByName(const char *p)
{
	int i;

	for (i = 0; i < menuCount; i++)
	{
		if (Q_stricmp(Menus[i].window.name, p) == 0)
		{
			return &Menus[i];
		}
	}
	return NULL;
}

void Menus_ShowByName(const char *p)
{
	menuDef_t *menu = Menus_FindByName(p);

	if (menu)
	{
		Menus_Activate(menu);
	}
}

void Menus_OpenByName(const char *p)
{
	Menus_ActivateByName(p, qtrue);
}

void Menu_RunCloseScript(menuDef_t *menu)
{
	if (menu && (menu->window.flags & WINDOW_VISIBLE) && menu->onClose)
	{
		itemDef_t item;

		item.parent = menu;
		Item_RunScript(&item, NULL, menu->onClose);
	}
}

void Menus_CloseByName(const char *p)
{
	menuDef_t *menu = Menus_FindByName(p);

	if (menu != NULL)
	{
		int i;

		// Gordon: make sure no edit fields are left hanging
		for (i = 0; i < menu->itemCount; i++)
		{
			if (g_editItem == menu->items[i])
			{
				g_editingField = qfalse;
				g_editItem     = NULL;
			}
		}

		menu->cursorItem = -1;
		Menu_ClearFocus(menu);
		Menu_RunCloseScript(menu);
		menu->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS | WINDOW_MOUSEOVER);
		if (menu->window.flags & WINDOW_MODAL)
		{
			if (modalMenuCount <= 0)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: tried closing a modal window with an empty modal stack!\n");
			}
			else
			{
				modalMenuCount--;
				// if modal doesn't have a parent, the stack item may be NULL .. just go back to the main menu then
				if (modalMenuStack[modalMenuCount])
				{
					Menus_ActivateByName(modalMenuStack[modalMenuCount]->window.name, qfalse);   // don't try to push the one we are opening to the stack
				}
			}
		}
	}
}

void Menus_CloseAll(void)
{
	int i;

	for (i = 0; i < menuCount; i++)
	{
		Menu_RunCloseScript(&Menus[i]);
		Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE | WINDOW_MOUSEOVER);
	}
}

// @note Unused
itemDef_t *Menu_HitTest(menuDef_t *menu, float x, float y)
{
	int i;

	for (i = 0; i < menu->itemCount; i++)
	{
		if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y))
		{
			return menu->items[i];
		}
	}
	return NULL;
}

itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu)
{
	qboolean wrapped   = qfalse;
	int      oldCursor = menu->cursorItem;

	if (menu->cursorItem < 0)
	{
		menu->cursorItem = menu->itemCount - 1;
		wrapped          = qtrue;
	}

	while (menu->cursorItem > -1)
	{
		menu->cursorItem--;
		if (menu->cursorItem < 0 && !wrapped)
		{
			wrapped          = qtrue;
			menu->cursorItem = menu->itemCount - 1;
		}

		if (menu->cursorItem < 0)
		{
			menu->cursorItem = oldCursor;
			return NULL;
		}

		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory))
		{
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}
	}
	menu->cursorItem = oldCursor;
	return NULL;

}

itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu)
{
	qboolean wrapped = qfalse;
	int      oldCursor;

	if (!menu)
	{
		return NULL;
	}

	oldCursor = menu->cursorItem;

	if (menu->cursorItem == -1)
	{
		menu->cursorItem = 0;
		wrapped          = qtrue;
	}

	while (menu->cursorItem < menu->itemCount)
	{
		menu->cursorItem++;
		if (menu->cursorItem >= menu->itemCount)      // had a problem 'tabbing' in dialogs with only one possible button
		{
			if (!wrapped)
			{
				wrapped          = qtrue;
				menu->cursorItem = 0;
			}
			else
			{
				return menu->items[oldCursor];
			}
		}

		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory))
		{
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}
	}

	menu->cursorItem = oldCursor;
	return NULL;
}

void Menu_CloseCinematics(menuDef_t *menu)
{
	if (menu)
	{
		int i;

		Window_CloseCinematic(&menu->window);
		for (i = 0; i < menu->itemCount; i++)
		{
			Window_CloseCinematic(&menu->items[i]->window);
			if (menu->items[i]->type == ITEM_TYPE_OWNERDRAW)
			{
				DC->stopCinematic(0 - menu->items[i]->window.ownerDraw);
			}
		}
	}
}

void  Menus_Activate(menuDef_t *menu)
{
	int i;

	for (i = 0; i < menuCount; i++)
	{
		Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_MOUSEOVER);
	}

	menu->window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);

	if (menu->onOpen)
	{
		itemDef_t item;
		item.parent = menu;
		Item_RunScript(&item, NULL, menu->onOpen);
	}

	// set open time (note dc time may be 0, in which case refresh code sets this)
	menu->openTime = DC->realTime;

	if (menu->soundName && *menu->soundName)
	{
		//DC->stopBackgroundTrack();                  // you don't want to do this since it will reset s_rawend
		DC->startBackgroundTrack(menu->soundName, menu->soundName, 0);
	}

	Display_CloseCinematics();

}

qboolean Menus_CaptureFuncActive(void)
{
	if (captureFunc)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

void Menus_HandleOOBClick(menuDef_t *menu, int key, qboolean down)
{
	if (menu)
	{
		int i;
		// basically the behaviour we are looking for is if there are windows in the stack.. see if
		// the cursor is within any of them.. if not close them otherwise activate them and pass the
		// key on.. force a mouse move to activate focus and script stuff
		if (down && (menu->window.flags & WINDOW_OOB_CLICK))
		{
			Menu_RunCloseScript(menu);
			menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE | WINDOW_MOUSEOVER);
		}

		for (i = 0; i < menuCount; i++)
		{
			if (Menu_OverActiveItem(&Menus[i], DC->cursorx, DC->cursory))
			{
				//Menu_RunCloseScript(menu);          // why do we close the calling menu instead of just removing the focus?
				//menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE | WINDOW_MOUSEOVER);

				menu->window.flags    &= ~(WINDOW_HASFOCUS | WINDOW_MOUSEOVER);
				Menus[i].window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);

				//Menus_Activate(&Menus[i]);
				Menu_HandleMouseMove(&Menus[i], DC->cursorx, DC->cursory);
				Menu_HandleKey(&Menus[i], key, down);
			}
		}

		if (Display_VisibleMenuCount() == 0)
		{
			if (DC->Pause)
			{
				DC->Pause(qfalse);
			}
		}
		Display_CloseCinematics();
	}
}

void Menu_HandleKey(menuDef_t *menu, int key, qboolean down)
{
	int       i;
	itemDef_t *item = NULL;

	Menu_HandleMouseMove(menu, DC->cursorx, DC->cursory);       // fix for focus not resetting on unhidden buttons

	// enter key handling for the window supercedes item enter handling
	if (down && ((key == K_ENTER || key == K_KP_ENTER) && menu->onEnter))
	{
		itemDef_t it;

		it.parent = menu;
		Item_RunScript(&it, NULL, menu->onEnter);
		return;
	}

	if (g_waitingForKey && down)
	{
		Item_Bind_HandleKey(g_bindItem, key, down);
		return;
	}

	if (g_editingField && down)
	{
		if (g_editItem->type == ITEM_TYPE_COMBO)
		{
			Item_Combo_HandleKey(g_editItem, key);
			Item_ComboDeSelect(g_editItem);
			return;
		}
		else
		{
			if (!Item_TextField_HandleKey(g_editItem, key))
			{
				Item_HandleTextFieldDeSelect(g_editItem);
				return;
			}
			else if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3)
			{
				Item_HandleTextFieldDeSelect(g_editItem);
				Display_MouseMove(NULL, DC->cursorx, DC->cursory);
			}
			else if (key == K_TAB || key == K_UPARROW || key == K_DOWNARROW)
			{
				return;
			}
		}
	}

	if (menu == NULL)
	{
		return;
	}

	// see if the mouse is within the window bounds and if so is this a mouse click
	if (down && !(menu->window.flags & WINDOW_POPUP) && !Rect_ContainsPoint(&menu->window.rect, DC->cursorx, DC->cursory))
	{
		static qboolean inHandleKey = qfalse;
		// bk001206 - parentheses
		if (!inHandleKey && (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3))
		{
			inHandleKey = qtrue;
			Menus_HandleOOBClick(menu, key, down);
			inHandleKey = qfalse;
			return;
		}
	}

	// get the item with focus
	for (i = 0; i < menu->itemCount; i++)
	{
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS)
		{
			item = menu->items[i];
		}
	}

	// handle clipboard event if the menu has it set and there is no item selected which has the action
	if (K_CLIPBOARD(key) && down && menu->onPaste && (item == NULL || (item != NULL && !item->onPaste)) && !g_editingField)
	{
		itemDef_t it;
		it.parent = menu;
		Item_RunScript(&it, NULL, menu->onPaste);
		return;
	}

	if (item != NULL)
	{
		if (Item_HandleKey(item, key, down))
		{
			Item_Action(item);
			return;
		}
	}

	if (!down)
	{
		return;
	}

	// we need to check and see if we're supposed to loop through the items to find the key press func
	if (!menu->itemHotkeyMode)
	{
		if (key >= 0 && key < MAX_KEYS && menu->onKey[key])
		{
			itemDef_t it;
			it.parent = menu;
			Item_RunScript(&it, NULL, menu->onKey[key]);
			return;
		}
	}
	else if (key >= 0 && key < MAX_KEYS)
	{
		itemDef_t *it;

		// we're using the item hotkey mode, so we want to loop through all the items in this menu
		for (i = 0; i < menu->itemCount; i++)
		{
			it = menu->items[i];

			// is the hotkey for this the same as what was pressed?
			if (it->hotkey == key
			    // and is this item visible?
			    && Item_EnableShowViaCvar(it, CVAR_SHOW))
			{
				Item_RunScript(it, NULL, it->onKey);
				return;
			}
		}
	}

	// default handling
	switch (key)
	{
	case K_F11:
		if (DC->getCVarValue("developer"))
		{
			debugMode ^= 1;
		}
		break;
	case K_F12:
		if (DC->getCVarValue("developer"))
		{
			DC->executeText(EXEC_APPEND, "screenshot\n");
		}
		break;
	case K_KP_UPARROW:
	case K_UPARROW:
		Menu_SetPrevCursorItem(menu);
		break;
	case K_ESCAPE:
		if (!g_waitingForKey && menu->onESC)
		{
			itemDef_t it;
			it.parent = menu;
			Item_RunScript(&it, NULL, menu->onESC);
		}
		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_MOUSE3:
		Item_KeyboardActivate(item);
		break;
	case K_TAB:
		if (DC->keyIsDown(K_SHIFT))
		{
			Menu_SetPrevCursorItem(menu);
		}
		else
		{
			Menu_SetNextCursorItem(menu);
		}
		break;
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		Menu_SetNextCursorItem(menu);
		break;
	case K_MOUSE1:
	case K_MOUSE2:
		Item_MouseActivate(item);
		break;
	default:
		//case K_JOY1:
		//case K_JOY2:
		//case K_JOY3:
		//case K_JOY4:
		//case K_AUX1:
		//case K_AUX2:
		//case K_AUX3:
		//case K_AUX4:
		//case K_AUX5:
		//case K_AUX6:
		//case K_AUX7:
		//case K_AUX8:
		//case K_AUX9:
		//case K_AUX10:
		//case K_AUX11:
		//case K_AUX12:
		//case K_AUX13:
		//case K_AUX14:
		//case K_AUX15:
		//case K_AUX16:
		break;
	}
}

void Menu_HandleMouseMove(menuDef_t *menu, float x, float y)
{
	int       i, pass;
	qboolean  focusSet = qfalse;
	itemDef_t *overItem;

	if (menu == NULL)
	{
		return;
	}

	if (!(menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)))
	{
		return;
	}

	if (itemCapture)
	{
		if (itemCapture->type == ITEM_TYPE_LISTBOX)
		{
			// lose capture if out of client rect
			if (!Rect_ContainsPoint(&itemCapture->window.rect, x, y))
			{
				itemCapture = NULL;
				captureFunc = NULL;
				captureData = NULL;
			}

		}
		//Item_MouseMove(itemCapture, x, y);
		return;
	}

	if (g_waitingForKey || g_editingField)
	{
		return;
	}

	// FIXME: this is the whole issue of focus vs. mouse over..
	// need a better overall solution as i don't like going through everything twice
	for (pass = 0; pass < 2; pass++)
	{
		for (i = 0; i < menu->itemCount; i++)
		{
			// turn off focus each item
			// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

			if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)))
			{
				continue;
			}

			// items can be enabled and disabled based on cvars
			if ((menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE)) && !Item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE))
			{
				continue;
			}

			if ((menu->items[i]->cvarFlags & (CVAR_SHOW | CVAR_HIDE)) && !Item_EnableShowViaCvar(menu->items[i], CVAR_SHOW))
			{
				continue;
			}

			// server settings too
			if ((menu->items[i]->settingFlags & (SVS_ENABLED_SHOW | SVS_DISABLED_SHOW)) && !Item_SettingShow(menu->items[i], qfalse))
			{
				continue;
			}
			if (menu->items[i]->voteFlag != 0 && !Item_SettingShow(menu->items[i], qtrue))
			{
				continue;
			}

			if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y))
			{
				if (pass == 1)
				{
					overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text)
					{
						if (!Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y))
						{
							continue;
						}
					}
					// if we are over an item
					if (IsVisible(overItem->window.flags))
					{
						// different one
						Item_MouseEnter(overItem, x, y);
						// Item_SetMouseOver(overItem, qtrue);

						// if item is not a decoration see if it can take focus
						if (!focusSet)
						{
							focusSet = Item_SetFocus(overItem, x, y);
						}
					}
				}
			}
			else if (menu->items[i]->window.flags & WINDOW_MOUSEOVER)
			{
				Item_MouseLeave(menu->items[i]);
				Item_SetMouseOver(menu->items[i], qfalse);
			}
		}
	}
}

void Menu_Paint(menuDef_t *menu, qboolean forcePaint)
{
	int       i;
	itemDef_t *item = NULL;

	if (menu == NULL)
	{
		return;
	}

	if (!(menu->window.flags & WINDOW_VISIBLE) && !forcePaint)
	{
		return;
	}

	if (menu->window.ownerDrawFlags && DC->ownerDrawVisible && !DC->ownerDrawVisible(menu->window.ownerDrawFlags))
	{
		return;
	}

	if (forcePaint)
	{
		menu->window.flags |= WINDOW_FORCED;
	}

	// draw the background if necessary
	if (menu->fullScreen)
	{
		// implies a background shader
		// FIXME: make sure we have a default shader if fullscreen is set with no background
		DC->drawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menu->window.background);
	}

	// paint the background and or border
	Window_Paint(&menu->window, menu->fadeAmount, menu->fadeClamp, menu->fadeCycle);

	for (i = 0; i < menu->itemCount; i++)
	{
		if (menu->items[i]->window.flags & WINDOW_MOUSEOVER)
		{
			item = menu->items[i];
			if (!IS_EDITMODE(menu->items[i]))
			{
				Item_Paint(menu->items[i]);
			}
		}
		else
		{
			Item_Paint(menu->items[i]);
		}
	}

	// Draw the active item last and dont draw tooltop at this point..
	if (item && IS_EDITMODE(item))
	{
		Item_Paint(item);
	}
	// draw tooltip data if we have it
	else if (DC->getCVarValue("ui_showtooltips") &&
	         item != NULL &&
	         item->toolTipData != NULL &&
	         item->toolTipData->text != NULL &&
	         *item->toolTipData->text)
	{
		Item_Paint(item->toolTipData);
	}

	// handle timeout here
	if (menu->openTime == 0)
	{
		menu->openTime = DC->realTime;
	}
	else if ((menu->window.flags & WINDOW_VISIBLE) &&
	         menu->timeout > 0 && menu->onTimeout != NULL &&
	         menu->openTime + menu->timeout <= DC->realTime)
	{
		itemDef_t it;
		it.parent = menu;
		Item_RunScript(&it, NULL, menu->onTimeout);
	}

	if (debugMode)
	{
		vec4_t color;
		color[0] = color[2] = color[3] = 1;
		color[1] = 0;
		DC->drawRect(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, 1, color);
	}
}

void Menu_PaintAll(void)
{
	int i;

	if (captureFunc)
	{
		captureFunc(captureData);
	}

	for (i = 0; i < menuCount; i++)
	{
		if (Menus[i].window.flags & WINDOW_DRAWALWAYSONTOP)
		{
			continue;
		}
		Menu_Paint(&Menus[i], qfalse);
	}

	for (i = 0; i < menuCount; i++)
	{
		if (Menus[i].window.flags & WINDOW_DRAWALWAYSONTOP)
		{
			Menu_Paint(&Menus[i], qfalse);
		}
	}

	if (debugMode)
	{
		vec4_t v = { 1, 1, 1, 1 };
		DC->textFont(UI_FONT_COURBD_21);
		DC->drawText(5, 10, .2, v, va("fps: %.2f", DC->FPS), 0, 0, 0);
		DC->drawText(5, 20, .2, v, va("mouse: %i %i", DC->cursorx, DC->cursory), 0, 0, 0);
	}
}

/*
===============
Menu_New
===============
*/
void Menu_New(int handle)
{
	menuDef_t *menu = &Menus[menuCount];

	if (menuCount < MAX_MENUS)
	{
		Menu_Init(menu);
		if (Menu_Parse(handle, menu))
		{
			Menu_PostParse(menu);
			menuCount++;
		}
	}
}

int Menu_Count(void)
{
	return menuCount;
}

menuDef_t *Menu_Get(int handle)
{
	if (handle >= 0 && handle < menuCount)
	{
		return &Menus[handle];
	}
	else
	{
		return NULL;
	}
}

void Menu_Reset(void)
{
	menuCount = 0;
}

qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y)
{
	if (menu && (menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)))
	{
		if (Rect_ContainsPoint(&menu->window.rect, x, y))
		{
			int i;

			for (i = 0; i < menu->itemCount; i++)
			{
				// turn off focus each item
				// menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;

				if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)))
				{
					continue;
				}

				if (menu->items[i]->window.flags & WINDOW_DECORATION)
				{
					continue;
				}

				if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y))
				{
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text)
					{
						if (Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y))
						{
							return qtrue;
						}
						else
						{
							continue;
						}
					}
					else
					{
						return qtrue;
					}
				}
			}
		}
	}
	return qfalse;
}
