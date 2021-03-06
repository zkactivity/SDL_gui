//
//  GUI_Menu.cpp
//  GUI_EditText
//
//  Created by Panutat Tejasen on 18/1/2562 BE.
//  Copyright © 2562 Jimmy Software Co., Ltd. All rights reserved.
//

#include "GUI_Menu.h"
#include "GUI_shapes.h"
#include "GUI_Fonts.h"
#include <iterator>

GUI_MenuItem *GUI_MenuItem::create( GUI_View *parent, const char *title, int x, int y, int width, int height,
                                   std::function<void(GUI_View*)>callbackFunction ) {
    return new GUI_MenuItem( parent, title, x, y, width, height, callbackFunction );
}

GUI_MenuItem::GUI_MenuItem(GUI_View *parent, const char *title, int x, int y, int width, int height,
                           std::function<void(GUI_View*)>callbackFunction ) :
    GUI_View( parent, title, x, y, width, height ),
separator(true)
{
    dragable = false;
    clickable = false;
    capture_on_click = false;
    callback_on_mouse_up = true;
    focusable = true;
    showInteract = true;
    mouseReceive = true;
    focusBorder = 0;

    setBackgroundColor(cWhite);
	label = NULL;
	labelTextColor = cBlack;

    setLayout(GUI_LAYOUT_HORIZONTAL);
    setBorder( 0 );

    submenu = NULL;
    selectedItem = NULL;
    iconView = NULL;
}

GUI_MenuItem::~GUI_MenuItem() {

}

void GUI_MenuItem::setSelected( bool s ) {
    selected = s;
    if( selected ) {
        setBackgroundColor( cHightLightSelection );
    }
    else {
        setBackgroundColor( cWhite );
    }
}

void GUI_MenuItem::postdraw() {
    GUI_View::postdraw();

    if( separator ) {
        GUI_DrawHLine( 0, rectView.width(), rectView.height()-1, cBlack );
    }
}

void GUI_MenuItem::enable() {
	GUI_View::enable();
	label->setTextColor(labelTextColor);
};

void GUI_MenuItem::disable() {
	GUI_View::disable();
	label->setTextColor(cGrey);
};

void GUI_MenuItem::setEnable(bool e) {
	if (e) {
		enable();
	} else {
		disable();
	}
};

void GUI_MenuItem::setSubmenu(GUI_View* view) {
    submenu = view;

    if (submenu) {
        submenu->hide();

        if (iconView == NULL) {
            iconView = GUI_IconView::create(this, kIcon_chevron_right, GUI_GetUIIconFontName().c_str(), GUI_GetUIIconFontSize(), 0, 0, 0, 0);
            iconView->setAlign(GUI_ALIGN_RIGHT | GUI_ALIGN_VCENTER);
            iconView->setBorder(0);
            iconView->setBackgroundColor(cClear);
            iconView->setColor(cBlack);
            iconView->setVisible(true);
        }
    } else {
        if (iconView) {
            iconView->setVisible(false);
        }
    }
}

GUI_View* GUI_MenuItem::getSubmenu() {
    return submenu;
}

// ------------------------------------------------------------------------------------------------

GUI_Menu *GUI_Menu::create( GUI_View *parent, const char *title, int x, int y, int width, int height,
                           std::function<void(GUI_View*)>callbackFunction, MenuAnimationMode mode ) {
    return new GUI_Menu( parent, title, x, y, width, height, callbackFunction, mode );
}

GUI_Menu::GUI_Menu(GUI_View *parent, const char *title, int x, int y, int width, int height,
                   std::function<void(GUI_View*)>callbackFunction, MenuAnimationMode mode ) :
GUI_View( parent, title, x, y, width, height ),
isOpen(1)
{
    clickable = false;
    capture_on_click = true;

    setCallback( callbackFunction );

    setAlign( GUI_ALIGN_ABSOLUTE );

    setBackgroundColor(cEmptyContent);
    setLayout(GUI_LAYOUT_VERTICAL);

    if( parent )
        parent->updateLayout();

    setMenuAnimationMode(mode);
}

GUI_Menu::~GUI_Menu() {

}

void GUI_Menu::add(GUI_MenuItem* child) {
    add_child(child);

    menuItems.push_back(child);
    child->setCallback( [=](GUI_View *v) {
        GUI_MenuItem *lit = (GUI_MenuItem *)v;
        int size(menuItems.size());
        std::vector<GUI_MenuItem *> items(menuItems);
        int i(0), dist(distance(menuItems.begin(), menuItems.end()));
        for (std::vector<GUI_MenuItem *>::iterator it = menuItems.begin() ; it != menuItems.end(); ++it) {
            GUI_MenuItem *c = *it;
            if( lit == c ) {
                c->setSelected(true);
                //GUI_Log( "%s\n", c->title.c_str());
                this->selectedItem = c;
                if( this->callback ) {
                    this->callback(this);
                }
            }
            else {
                c->setSelected(false);
            }
            ++i;
        }
    });
}

void GUI_Menu::remove(GUI_MenuItem* child) {
    remove_child(child);
    for (std::vector<GUI_MenuItem *>::iterator it = menuItems.begin() ; it != menuItems.end();) {
        if( child == *it ) {
            it = menuItems.erase( it );
            break;
        }
        else
            ++it;
    }
    child->setCallback(NULL);
}

GUI_MenuItem * GUI_Menu::addSimpleMenu( const char *title, bool separator ) {
    GUI_MenuItem *item1 = GUI_MenuItem::create( NULL, title, 0, 0, -1, 0 );
    item1->setPadding( 8, 10, 8, 10 );
    item1->separator = separator;

    GUI_Label *lb1 = GUI_Label::create( item1, title, 0, 0, -1, 0 );
    lb1->setAlign( GUI_ALIGN_LEFT | GUI_ALIGN_VCENTER );
    lb1->setBackgroundColor(cClear);

    add( item1 );
    updateLayout();
    return item1;
}

void GUI_Menu::close( int duration ) {
    if( isMoving )
        return;
    isOpen = false;
    moveTo(hiddenX, hiddenY, duration );
//    moveTo( 0-GUI_GetAppMenuWidth(), getAbsolutePosition().y(), duration );

    GUI_SetMouseCapture( NULL );
    //GUI_Log( "Menu close\n" );

}

void GUI_Menu::open( int duration ) {
    if( isMoving )
        return;
    if(selectedItem)
        selectedItem->setSelected(false);
    isOpen = true;
#ifdef __IPHONEOS__
    moveTo( getContentSaftyMargin()[3], getAbsolutePosition().y(), duration );
#else
    moveTo(visibleX, visibleY, duration );
        //moveTo(0, getAbsolutePosition().y(), duration );
#endif

    GUI_SetMouseCapture( this );
    //GUI_Log( "Menu open\n" );
}

void GUI_Menu::predraw() {
    GUI_View::predraw();
    //GUI_Log( "TopLeft: %i, %i\n", topLeft.x, topLeft.y );
}

bool GUI_Menu::eventHandler(SDL_Event*event) {
    SDL_Scancode scancode;

    switch (event->type) {
        case GUI_UpdateSize:
        {
            oy = GUI_GetAppTopBarHeight() + GUI_GetStatusBarHeight();
            ox = 0;
#ifdef __IPHONEOS__
            ox = getContentSaftyMargin()[3];
#endif
            setMenuAnimationMode(menuAnimationMode);
            open();
            bool ret = GUI_View::eventHandler(event);
            close();
            return ret;
            break;
        }
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        {
            SDL_TouchFingerEvent e = event->tfinger;

            int x = (int)(e.x*GUI_windowWidth*GUI_mouseScale);
            int y = (int)(e.y*GUI_windowHeight*GUI_mouseScale);

            if( isOpen ) {
                if( activateView && event->type == SDL_FINGERDOWN) {
                    if( activateView->hitTest(x, y, false) ) {
                        return true;
                    }
                }
                if( !hitTest(x, y, false) ) {
                    close(GUI_AppMenuCollapseTime);
                    GUI_SetMouseCapture(NULL);
                }
            }
            return GUI_View::eventHandler(event);
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            SDL_MouseButtonEvent e = event->button;

            int x = (int)(e.x*GUI_mouseScale);
            int y = (int)(e.y*GUI_mouseScale);

            if( isOpen ) {
                if( activateView && event->type == SDL_MOUSEBUTTONDOWN) {
                    if( activateView->hitTest(x, y, false) ) {
                        return true;
                    }
                }
                if( !hitTest(x, y, false) ) {
                    close(GUI_AppMenuCollapseTime);
                    GUI_SetMouseCapture(NULL);
                    return true;
                }
            }
            return GUI_View::eventHandler(event);
        }

        default:
        {
            return GUI_View::eventHandler(event);
        }
    }

    return false;
}

MenuAnimationMode GUI_Menu::getMenuAnimationMode()
{
    return menuAnimationMode;
}

void GUI_Menu::setMenuAnimationMode(MenuAnimationMode mode)
{
    menuAnimationMode = mode;
    switch(menuAnimationMode)
    {
    case eFromLeft:
        hiddenX = -GUI_GetAppMenuWidth();
        hiddenY = getAbsolutePosition().y;
        visibleX = 0;
        visibleY = hiddenY;
        break;
    case eFromRight:
        hiddenX = parent->getWidth() * GUI_scale;
        hiddenY = getAbsolutePosition().y;
        visibleX = (parent->getWidth() * GUI_scale) - GUI_GetAppMenuWidth();
        visibleY = hiddenY;
        break;
    case eFromTop:
        hiddenX = ((parent->getWidth() * GUI_scale) - GUI_GetAppMenuWidth()) / 2;
        hiddenY = (- getHeight()) * GUI_scale;
        visibleX = hiddenX;
        visibleY = getAbsolutePosition().y * GUI_scale;
        break;
    case eFromBottom:
        hiddenX = ((parent->getWidth() - getWidth()) / 2) * GUI_scale;
        hiddenY = parent->getHeight() * GUI_scale;
        visibleX = hiddenX;
        visibleY = getAbsolutePosition().y;
        break;
    }
}
