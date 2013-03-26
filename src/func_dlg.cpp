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

#include "tagger.h"

#include "func_dlg.h"



#define LIST_OPTS LIST_BROWSESELECT|LIST_AUTOSELECT|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y|SCROLLERS_TRACK|HSCROLLER_NEVER

/* A text field with a drop-down auto-complete list */
class AutoEdit:public ClipTextField {
  FXDECLARE(AutoEdit)
  AutoEdit() {}
  const FXString*choices;
  FXPopup*pane;
  FXList*list;
  FXint nitems;
  FXint nchars;
  static FXint SortFunc(const FXListItem*a,const FXListItem*b);
public:
  AutoEdit(FXComposite*p,FXint ncols,FXObject*tgt,FXSelector sel):
    ClipTextField(p,ncols,tgt,sel),pane(NULL),choices(NULL) {}
  ~AutoEdit() { delete pane; }
  void setChoices(const FXString*c);
  long onKeyPress(FXObject*o, FXSelector sel, void*p);
  long onComplete(FXObject*o, FXSelector sel, void*p);
  void ShowChoices(const char*txt);
  enum {
    ID_COMPLETE=FXTextField::ID_LAST,
    ID_LAST
  };
};


FXDEFMAP(AutoEdit) AutoEditMap[]={
   FXMAPFUNC(SEL_COMMAND,  AutoEdit::ID_COMPLETE, AutoEdit::onComplete),
   FXMAPFUNC(SEL_KEYPRESS, 0,AutoEdit::onKeyPress),
};
FXIMPLEMENT(AutoEdit,ClipTextField,AutoEditMap,ARRAYNUMBER(AutoEditMap));



FXint AutoEdit::SortFunc(const FXListItem*a,const FXListItem*b)
{
  return compare(a->getText(),b->getText());
}



void AutoEdit::setChoices(const FXString*c) { 
  choices=c;
  nitems=c->contains('\n');
  pane=new FXPopup(this,FRAME_LINE|LAYOUT_EXPLICIT);
  list=new FXList(pane,this,ID_COMPLETE,LIST_OPTS);
}



long AutoEdit::onComplete(FXObject*o, FXSelector sel, void*p)
{
  appendText((const char*)&(list->getItemText(list->getCurrentItem()).text())[nchars]);
  pane->popdown();
  return 1;
}



long AutoEdit::onKeyPress(FXObject*o,FXSelector sel, void*p)
{
  if (choices) {
    FXEvent*ev=(FXEvent*)p;
    switch (ev->code) {
      case KEY_Down: {
        FXint n=list->getCurrentItem()+1;
        if (n<list->getNumItems()) list->setCurrentItem(n);
        return 1;
      }
      case KEY_Up: {
        FXint n=list->getCurrentItem()-1;
        if (n>=0) list->setCurrentItem(n);
        return 1;
      }
      case KEY_KP_Enter:
      case KEY_Return: {
        handle(list,FXSEL(SEL_COMMAND,ID_COMPLETE),(void*)(FXival)list->getCurrentItem());
        return 1;
      }
      case KEY_Escape: {
        pane->popdown();
        return 1;
      }
    }
  }
  return FXTextField::onKeyPress(o,sel,p);
}



void AutoEdit::ShowChoices(const char*txt)
{
  if (!choices) { return; }
  pane->popdown();
  list->clearItems();
  if (!(txt&&*txt)) { return; }
  const char*p1=strchr(txt,'\0')-1;
  while ((p1>txt)&&(Ascii::isAlphaNumeric(*p1)||*p1=='_')) { p1--; }
  if (p1>txt) { p1++; }
  nchars=strlen(p1);
  if (nchars>1) {
    for (FXint i=0; i<=nitems; i++) {
      const FXString sect=choices->section('\n',i);
      if ((compare(sect,p1,nchars)==0)&&(list->findItem(sect)==-1)) {
        list->appendItem(sect);
      }
    }
    if (list->getNumItems()>0) {
      FXint x,y,w,h;
      list->setSortFunc(SortFunc);
      list->sortItems();
      list->setNumVisible(list->getNumItems()>24?24:list->getNumItems());
      list->setWidth(getWidth());
      list->setCurrentItem(0);
      pane->create();
      w=pane->getDefaultWidth();
      h=pane->getDefaultHeight();
      translateCoordinatesTo(x,y,getRoot(),0,0);
      y+=getHeight()+2;
      pane->popup(this,x,y,w,h);
    } else {
      pane->popdown();
    }
  }
}



FXDEFMAP(ParamsDlg) ParamsDlgMap[]={
  FXMAPFUNC(SEL_CHANGED,  ParamsDlg::ID_EDIT,ParamsDlg::onEdit),
};

FXIMPLEMENT(ParamsDlg,FXDialogBox,ParamsDlgMap,ARRAYNUMBER(ParamsDlgMap));



long ParamsDlg::onEdit(FXObject*o, FXSelector sel, void*p)
{
  ((AutoEdit*)o)->ShowChoices((const char*)p);
  return 1;
}


#define SetPadLRTB(o,l,r,t,b) \
  o->setPadLeft(l); \
  o->setPadRight(r); \
  o->setPadTop(t); \
  o->setPadBottom(b);


ParamsDlg::ParamsDlg(MainWinWithClipBrd*owner, FXStringDict*d, const FXString &opt_names, const FXString &id_names):FXDialogBox(owner,FXString::null)
{
  dict=d;
  mw=owner;
  SetPadLRTB(this,0,0,0,0);
  FXString sig=Key("signature");
  if (sig.length()>2) {
    sig.erase(0);
    sig.trunc(sig.rfind(')'));
    setTitle(Key("kind")+" "+Key("name")+"(");
    FXVerticalFrame*vframe=new FXVerticalFrame(this);
    SetPadLRTB(vframe,0,0,0,0);
    mtx=new FXMatrix(vframe,2,MATRIX_BY_COLUMNS|LAYOUT_FILL);
    for (FXint i=0; i<=sig.contains(','); i++) {
      FXLabel*param=new FXLabel(mtx,sig.section(',',i).substitute("&", "&&"));
      AutoEdit*arg=new AutoEdit(mtx,96,this,ID_EDIT);
      if (compare(param->getText(),"FXuint opts=",12)==0) {
        arg->setChoices(&opt_names);
      } else if (compare(param->getText(),"FXSelector sel=",12)==0) {
        arg->setChoices(&id_names);
      }
    }
    FXHorizontalFrame*hframe=new FXHorizontalFrame(vframe,FRAME_GROOVE|FRAME_THICK|LAYOUT_FILL|PACK_UNIFORM_WIDTH);
    FXHorizontalFrame*btns=new FXHorizontalFrame(hframe,FRAME_NONE|PACK_UNIFORM_WIDTH|LAYOUT_RIGHT);
    SetPadLRTB(hframe,0,0,0,0);
    btns->setHSpacing(8);
    new FXButton(btns, "Cop&y", NULL, this, ID_ACCEPT, BUTTON_NORMAL|LAYOUT_CENTER_X);
    new FXButton(btns, "Ca&ncel", NULL, this, ID_CANCEL, BUTTON_NORMAL|LAYOUT_CENTER_X);
  } else { mtx=NULL; }
}



FXuint ParamsDlg::execute(FXuint placement)
{
  if (mtx && FXDialogBox::execute(placement)) {
    FXString s;
    bool opts_reqd=false;
    FXTextField*arg;
    for (FXint i=mtx->getNumRows()-1; i>=0; i--) {
      arg=dynamic_cast<FXTextField*>(mtx->childAtRowCol(i,1));
      FXLabel*param=dynamic_cast<FXLabel*>(mtx->childAtRowCol(i,0));
      bool optarg=param->getText().contains('=');
      if (arg->getText().empty()) {
        if (!optarg) {
          FXMessageBox::error(this,MBOX_OK,"Argument required","Required argument \"%s\" is missing", param->getText().text());
          arg->setFocus();
          return execute(placement);
        } else {
          if (opts_reqd) {
            arg->setText(param->getText().section('=',1));
          }
        }
      } else {
        if (optarg) { opts_reqd=true; }
      }
      if (!s.empty()) { s.prepend(", "); }
      s.prepend(arg->getText());
    }
    s.prepend('(');
    s.prepend(Key("name"));
    s.append(')');
    arg->setText(s);
    arg->selectAll();
    arg->copySelection();
    mw->SaveClipboard();
    return 1;
  }
  return 0;
}

