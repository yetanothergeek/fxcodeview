/*
  FXCodeView - Source code viewer and class browser.
  Copyright (c) 2011 Jeffrey Pohlmeyer <yetanothergeek@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License version 3 as
  published by the Free Software Foundation.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <fx.h>
#include <fxkeys.h>
#include "compat.h"
#include "widgets.h"


FXDEFMAP(MyTreeList) MyTreeListMap[]={
  FXMAPFUNC(SEL_ENTER, 0, MyTreeList::onEnterLeave),
  FXMAPFUNC(SEL_LEAVE, 0, MyTreeList::onEnterLeave),
};

FXIMPLEMENT(MyTreeList,FXTreeList,MyTreeListMap,ARRAYNUMBER(MyTreeListMap));

MyTreeList::MyTreeList(FXComposite*o,FXObject*trg,FXSelector sel,FXuint opts):FXTreeList(o,trg,sel,opts){
  DefaultWheelLines=getApp()->getWheelLines();
  DefaultScrollSpeed=getApp()->getScrollSpeed();
}



long MyTreeList::onEnterLeave(FXObject*o, FXSelector sel, void*p) {
  switch (FXSELTYPE(sel)) {
    case SEL_ENTER: {
      getApp()->setWheelLines(DefaultWheelLines*8);
      getApp()->setScrollSpeed(DefaultScrollSpeed/16);
      break;
    }
    case SEL_LEAVE: {
      getApp()->setWheelLines(DefaultWheelLines);
      getApp()->setScrollSpeed(DefaultScrollSpeed);
      break;
    }
  }
  return FXTreeList::handle(0,sel,p);
}




FXDEFMAP(MyTextField) MyTextFieldMap[]={
  FXMAPFUNC(SEL_KEYPRESS,0,MyTextField::onKeyPress),
};

FXIMPLEMENT(MyTextField,FXTextField,MyTextFieldMap,ARRAYNUMBER(MyTextFieldMap));



long MyTextField::onKeyPress(FXObject*o, FXSelector sel, void*p) {
  FXEvent*ev=(FXEvent*)p;
  if (ev && ev->code==KEY_Tab && !(ev->state&SHIFTMASK)) {
  if (tabdown) {
    FXWindow*tab=tabdown->childAtIndex(tabdown->getCurrent()*2);
    tabdown->setFocus();
    if (tab) { tab->setFocus(); }
    return 1;
  }
  }
  return FXTextField::onKeyPress(o,sel,p);
}



MyTextField::MyTextField(FXComposite*o, 
  FXint ncols, FXObject*trg, FXSelector sel, FXuint opts):FXTextField(o,ncols,trg,sel,opts)
{
  tabdown=NULL;
}



// manually controls tab navigation
void MyTextField::SetTabFocusDn(FXTabBook*down)
{
  tabdown=down;
}




FXDEFMAP(MyTabItem) MyTabItemMap[]={
  FXMAPFUNC(SEL_KEYPRESS,0,MyTabItem::onKeyPress),
};

FXIMPLEMENT(MyTabItem,FXTabItem,MyTabItemMap,ARRAYNUMBER(MyTabItemMap));



long MyTabItem::onKeyPress(FXObject*o, FXSelector sel, void*p) {
  FXEvent*ev=(FXEvent*)p;
  if (ev && ev->code==KEY_Tab) {
    if (ev->state&SHIFTMASK) {
      if (getPrev()) {
        FXTabBook*tb=(FXTabBook*)getParent();
        tb->setCurrent(tb->getCurrent()-1);
      }
    } else {
      getNext()->setFocus();
      if (getNext()->getFirst()) { getNext()->getFirst()->setFocus(); }
    }
    return 1;
  }
  return FXTabItem::onKeyPress(o,sel,p);
}



MyTabItem::MyTabItem(FXTabBar*o, FXString txt):FXTabItem(o,txt){}




FXDEFMAP(MyText) MyTextMap[]={
  FXMAPFUNC(SEL_FOCUSIN, 0, MyText::onMyTextFocusIn),
  FXMAPFUNC(SEL_KEYPRESS,0,MyText::onKeyPress),
  FXMAPFUNC(SEL_TIMEOUT,MyText::ID_TEXT_CURSOR,MyText::onTextCursor),
  FXMAPFUNC(SEL_ENTER, 0, MyText::onEnterLeave),
  FXMAPFUNC(SEL_LEAVE, 0, MyText::onEnterLeave),  
};

FXIMPLEMENT(MyText,FXText,MyTextMap,ARRAYNUMBER(MyTextMap));



MyText::MyText(FXComposite*o):FXText(o,NULL,0,TEXT_READONLY|TEXT_SHOWACTIVE|LAYOUT_FILL)
{
  tabup=NULL;
  tabdown=NULL;
  DefaultWheelLines=getApp()->getWheelLines();
  DefaultScrollSpeed=getApp()->getScrollSpeed();
  DefaultScrollDelay=getApp()->getScrollDelay();
  getApp()->addTimeout(this,ID_TEXT_CURSOR,ONE_SECOND);
}



long MyText::onEnterLeave(FXObject*o, FXSelector sel, void*p)
{
  switch (FXSELTYPE(sel)) {
    case SEL_ENTER: 
      getApp()->setWheelLines(4);
      getApp()->setScrollSpeed(DefaultScrollSpeed*16);
      getApp()->setScrollDelay(0);
      break;
    case SEL_LEAVE:
      getApp()->setScrollSpeed(DefaultScrollSpeed);
      getApp()->setScrollDelay(DefaultScrollDelay);
      break;
  }
  return FXText::handle(0,sel,p);
}



// manually controls tab navigation
void MyText::SetTabFocusUpDn(FXWindow*up, FXWindow*down)
{
  tabup=up;
  tabdown=down;
}



long MyText::onMyTextFocusIn(FXObject*o, FXSelector sel, void*p)
{
  long rv=FXText::handle(0,sel,p);
  onTextCursor(this, FXSEL(SEL_TIMEOUT,ID_TEXT_CURSOR), this);
  return rv;
}



long MyText::onKeyPress(FXObject*o, FXSelector sel, void*p)
{
  FXEvent*ev=(FXEvent*)p;
  if (ev) {
    switch (ev->code) {
      case KEY_ISO_Left_Tab: { 
        if (tabup) {
          tabup->setFocus();
          ClearCursor(getCursorPos());
          return 1;
        }
      }
      case KEY_Tab: {
        if (tabdown) {
          tabdown->setFocus();
          ClearCursor(getCursorPos());
          return 1;
        }
      }
    }
  }
  return FXText::onKeyPress(o,sel,p);
}



void MyText::ClearCursor(FXint pos)
{
  FXint x=getXOfPos(pos)+getContentX();
  FXint y=getYOfPos(pos)+getContentY();
  update(x-4,y-16,8,32);
}



long MyText::onTextCursor(FXObject*o, FXSelector sel, void*p)
{
  if (hasFocus()||(p==this)) {
    static FXint oldpos=0;
    FXint newpos=getCursorPos();
    if (oldpos!=newpos) {
      ClearCursor(oldpos);
      oldpos=newpos;
    }
    FXDCWindow dc(this);
    paintCursor(dc);
    getApp()->addTimeout(this,ID_TEXT_CURSOR,ONE_SECOND/30);
  } else {
    getApp()->addTimeout(this,ID_TEXT_CURSOR,ONE_SECOND);
  }
  return 1;
}




FXDEFMAP(MyIconList) MyIconListMap[]={
  FXMAPFUNC(SEL_FOCUSIN, 0, MyIconList::onFocusInOut),
  FXMAPFUNC(SEL_FOCUSOUT, 0, MyIconList::onFocusInOut),
  FXMAPFUNC(SEL_KEYPRESS,0,MyIconList::onKeyPress),
};

FXIMPLEMENT(MyIconList,FXIconList,MyIconListMap,ARRAYNUMBER(MyIconListMap));



long MyIconList::onFocusInOut(FXObject*o, FXSelector sel, void*p)
{
  long rv=FXIconList::handle(o,sel,p);
  switch (FXSELTYPE(sel)) {
    case SEL_FOCUSIN: {
      if (getNumItems()>0) {
        if (!isItemSelected(getCurrentItem())) {
          setCurrentItem(0);
          selectItem(0, true);
          target->handle(this,FXSEL(SEL_COMMAND,message),0);
        }
        setListStyle(ICONLIST_BROWSESELECT);
      }
      break; 
    }
    case SEL_FOCUSOUT: {
      setListStyle(ICONLIST_SINGLESELECT);
      break; 
    }
  }
  return rv;
}



long MyIconList::onKeyPress(FXObject*o, FXSelector sel, void*p) {
  FXEvent*ev=(FXEvent*)p;
  if (ev && ev->code==KEY_Tab) {
    if (tabdown) {
      tabdown->setFocus();
      tabdown->childAtIndex(tabdown->getCurrent())->setFocus();
      return 1;
    }
  }
  return FXIconList::onKeyPress(o,sel,p);
}



MyIconList::MyIconList(FXComposite*o, FXObject *trg, FXSelector sel):
  FXIconList(o,trg,sel,ICONLIST_SINGLESELECT|LAYOUT_FILL)
{
  tabdown=NULL;
}


