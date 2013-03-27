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


// For some reason the mouse wheel feels like it is spinning its tires in the TreeList.
// This subclass only serves to turbocharge the TreeList's mouse wheel and scroll buttons.
class MyTreeList: public FXTreeList {
  FXDECLARE(MyTreeList)
  MyTreeList() {}
  FXint DefaultWheelLines;
  FXTime DefaultScrollSpeed;
public:
  MyTreeList(FXComposite*o,FXObject*trg,FXSelector sel,FXuint opts);
  long onEnterLeave(FXObject*o, FXSelector sel, void*p);
};




// FXText does not show the cursor when it is in read-only mode. (why?)
// Also, we want the TAB key to do navigation instead of editing.
class MyText: public FXText {
  MyText(){}
  FXDECLARE(MyText)
  FXWindow*tabup;
  FXWindow*tabdown;
  FXint DefaultWheelLines;
  FXTime DefaultScrollSpeed;
  FXTime DefaultScrollDelay;
public:
  MyText(FXComposite*o);
  void SetTabFocusUpDn(FXWindow*up, FXWindow*down);
  long onMyTextFocusIn(FXObject*o, FXSelector sel, void*p);
  long onKeyPress(FXObject*o, FXSelector sel, void*p);
  void ClearCursor(FXint pos);
  long onTextCursor(FXObject*o, FXSelector sel, void*p);
  long onEnterLeave(FXObject*o, FXSelector sel, void*p);
  enum { ID_TEXT_CURSOR=FXText::ID_LAST, ID_LAST };
};



class MyTextField: public FXTextField {
  MyTextField(){}
  FXDECLARE(MyTextField)
  FXTabBook *tabdown;
public:
  long onKeyPress(FXObject*o, FXSelector sel, void*p);
  MyTextField(FXComposite*o, FXint ncols, FXObject*trg, FXSelector sel,FXuint opts=TEXTFIELD_NORMAL);
  void SetTabFocusDn(FXTabBook*down);
};



class MyTabItem: public FXTabItem {
  MyTabItem(){}
  FXDECLARE(MyTabItem)
public:
  long onKeyPress(FXObject*o, FXSelector sel, void*p);
  MyTabItem(FXTabBar*o, FXString txt);
};



class MyIconList: public FXIconList {
  MyIconList(){}
  FXDECLARE(MyIconList)
  FXSwitcher *tabdown;
public:
  long onFocusInOut(FXObject*o, FXSelector sel, void*p);
  long onKeyPress(FXObject*o, FXSelector sel, void*p);
  MyIconList(FXComposite*o, FXObject *trg, FXSelector sel);
  void SetTabFocusDn(FXSwitcher*down) { tabdown=down; }
};




// The source code viewer uses various font weights and styles
// internally, so we only allow the user to select a font with 
// normal weight and regular style. This class simply disables
// those options.
class MyFontSel:public FXFontSelector {
public:
  void setup() {
    weightlist->disable();
    weightlist->setBackColor(getApp()->getBaseColor());
    stylelist->disable();
    stylelist->setBackColor(getApp()->getBaseColor());
    weight->setBackColor(getApp()->getBaseColor());
    style->setBackColor(getApp()->getBaseColor());
  }
};



class MyFontDlg: public FXFontDialog {
public:
  void setup() { ((MyFontSel*)fontbox)->setup(); }
};
