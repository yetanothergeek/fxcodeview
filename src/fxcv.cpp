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


#ifdef WIN32
# include <windows.h>
#endif


#include <fx.h>
#include <fxkeys.h>
#include "compat.h"

#include "runcmd.h"
#include "interproc.h"
#include "styler.h"
#include "widgets.h"
#include "tagger.h"
#include "path_dlg.h"
#include "func_dlg.h"

#include "fxcv.h"



FXDEFMAP(MainWin) MainWinMap[]={
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SELECT_CLASS,     MainWin::onSelectClass),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SELECT_MEMBER,    MainWin::onSelectMember),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SELECT_RESULT,    MainWin::onSelectResult),
  FXMAPFUNC(SEL_CHANGED, MainWin::ID_SEARCH_CLASSES,   MainWin::onSearchClasses),
  FXMAPFUNC(SEL_CHANGED, MainWin::ID_SEARCH_MEMBERS,   MainWin::onSearchMembers),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SEARCH_ALL,       MainWin::onSearchAll),
  FXMAPFUNC(SEL_TIMEOUT, MainWin::ID_EDIT_TIMER,       MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_EDIT_TIMER,       MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_TOGGLE_ANCHORED,  MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_FIND_NEXT_CLASS,  MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_FIND_PREV_CLASS,  MainWin::onEditTimer),  
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_FIND_NEXT_MEMBER, MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_FIND_PREV_MEMBER, MainWin::onEditTimer),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_IPC_EXEC,         MainWin::onIpcExec),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SET_CODE_FONT,    MainWin::onSetCodeFont),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SOURCE_BUTTON,    MainWin::onSourceButton),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_OPEN_BUTTON,      MainWin::onOpenButton),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_PICK_DIRECTORY,   MainWin::onPickDirectory),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_PICK_FILE,        MainWin::onPickFile),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_LEFT_PANE_TABS,   MainWin::onLeftPaneTabs),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_SET_SEARCH_OPT,   MainWin::onSetSearchOpt),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_CLEAR_FIELD,      MainWin::onClearField),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_EDIT_PATHLIST,    MainWin::onEditPathList),
  FXMAPFUNC(SEL_COMMAND, MainWin::ID_ANCESTRY_BTNS,    MainWin::onAncestryBtns),
  FXMAPFUNC(SEL_DOUBLECLICKED, MainWin::ID_SELECT_MEMBER, MainWin::onActivateMember),
};


FXIMPLEMENT(MainWin,MainWinWithClipBrd,MainWinMap,ARRAYNUMBER(MainWinMap))


const
#include "fxcv.xpm"


static FXString initclass=FXString::null;

const FXString&MainWin::InitialClass() { return initclass; }

void MainWin::InitialClass(const FXString &classname) { initclass=classname; }




// Called by the InterProc object to process an IPC request.
long MainWin::onIpcExec(FXObject*o, FXSelector sel, void*p)
{
  hide();
  show();
  getApp()->runWhileEvents();
  classfinder->setText(*((FXString*)p),true);
  onSearchClasses( classfinder,FXSEL(SEL_CHANGED,ID_SEARCH_CLASSES),
                        (void*)(classfinder->getText().text()) );
  return 1;
}



long MainWin::onClearField(FXObject*o, FXSelector sel, void*p)
{
  FXTextField*tf=(FXTextField*)(((FXWindow*)o)->getPrev());
  MakeBoxRed(tf,false);
  tf->setText(FXString::null);
  tf->setFocus();
  return 1;
}



// Called when the user switches tabs in the left-hand panel
long MainWin::onLeftPaneTabs(FXObject*o, FXSelector sel, void*p)
{
  switch ((FXuval)p) {
    case 0: // List tab
    case 1: // Tree tab
      if (classlist->getNumItems()) {
        classfinder->setText(classlist->getItemText(classlist->getCurrentItem()));
      } else {
        classfinder->setText(FXString::null);
      }
      break;
    case 2: // Files tab
      if (filelist->getNumItems()) {
        classfinder->setText(filelist->getItemText(filelist->getCurrentItem()));
      } else {
        classfinder->setText(FXString::null);
      }
      break;
  }
  return 1;
}



// The user selected a file from the Files list
long MainWin::onPickFile(FXObject*o, FXSelector sel, void*p)
{
  FXString filename;
  if (filelist->getNumItems()<=0) { return 1; }
  filename.format("%s%c%s",
    dirbox->getItemText(dirbox->getCurrentItem()).text(),
    PATHSEP,
    filelist->getItemText((FXint)(FXuval)p).text()
  );
  if (o&&sel) { classfinder->setText(filelist->getItemText((FXint)(FXuval)p)); }
  filename=FXPath::simplify(filename);
  FXText *view;
  if (FXStat::isFile(filename)) {
    hdrfile=filename;
    if (Ascii::toLower(FXPath::extension(filename)[0])=='c') {
      view=src_view;
      ShowSource(filename);
    } else {
      view=hdr_view;
      ShowSource(FXString::null);
      filenamefield->setText(filename);
    }
    FXFile fh(filename,FXIO::Reading);
    if (fh.isOpen()) {
      tp->Contents().length(fh.size());
      fh.readBlock(tp->Contents().text(), fh.size());
      fh.close();
      Styler::ParseContents(view,stylebuf,tp->Contents());
    }
    tp->ParseSingle(filename,true);
    if (memberlist->getNumItems()==0) { members_tabs->setCurrent(1); }
  }
  return 1;
}



// The user switched directories in the Directory list drop-down
long MainWin::onPickDirectory(FXObject*o, FXSelector sel, void*p)
{
  if (!dirbox->getNumItems()) { 
  return 1;
  }
  filelist->clearItems();
  filelist->setPosition(0,0);
  FXString currdir=FXSystem::getCurrentDirectory();
  FXString thisdir=dirbox->getItemText((FXint)(FXuval)p);
  FileList*files=(FileList*)dirbox->getItemData((FXint)(FXuval)p);
  if (FXSystem::setCurrentDirectory(thisdir)) {
    for (FXint i=0; i<files->no(); i++) {
      const FXString &filename=files->at(i)->filename();
      if (FXStat::isFile(filename))filelist->appendItem(filename);
    }
  }
  FXSystem::setCurrentDirectory(currdir);
  if (filelist->getNumItems()) {
    filelist->sortItems();
    filelist->setCurrentItem(0);
    if (sel) { onPickFile(NULL,0,NULL); }
  }
  return 1;
}



void MainWin::LoadLocation(Location*locn, bool wholefile)
{
  if (locn) {
    ClearList(memberlist);
    ClearList(otherlist);
    tp->LocationToFullPath(hdrfile,locn);
    src_locns.clear();
    sourcebutton->disable();
    if (FXStat::isFile(hdrfile)) {
      dirbox->setCurrentItem(locn->idir);
      onPickDirectory(NULL,0,(void*)(FXival)locn->idir);
      for (FXint i=0; i<filelist->getNumItems(); i++) {
        if (filelist->getItemText(i)==tp->LocationToFilename(locn)) {
          filelist->setCurrentItem(i);
          filelist->makeItemVisible(i);
          break;
        }
      }
      FXFile fh(hdrfile,FXIO::Reading);
      if (fh.isOpen()) {
        tp->Contents().length(fh.size());
        fh.readBlock(tp->Contents().text(), fh.size());
        fh.close();
        ShowSource(FXString::null);
        Styler::ParseContents(hdr_view,stylebuf,tp->Contents());
        hdr_view->setCursorColumn(0);
        hdr_view->setCursorRow(locn->line-1,true);
        FXint pos=hdr_view->getCursorPos();
        if (!hdr_view->isPosVisible(pos)) { hdr_view->setBottomLine(pos); }
      }
    } else {
      fxmessage("Can't find file: %s\n", hdrfile.text());
    }
    tp->ParseSingle(hdrfile,wholefile);
  }
}



//  TRUE:   Turns the search field red to indicate a failed search.
//  FALSE:  Resets it to the default colors.
void MainWin::MakeBoxRed(FXTextField*box, bool iserror)
{
  if (iserror) {
    box->setTextColor(FXRGB(255,255,128));
    box->setBackColor(FXRGB(224,0,0));
  } else {
    box->setTextColor(getApp()->getForeColor());
    box->setBackColor(getApp()->getBackColor());    
  }
}



// When one of the "ancestry" buttons is clicked, de-select the other
// buttons and jump to the selected class in the tree list.
long MainWin::onAncestryBtns(FXObject*o, FXSelector sel, void*p)
{
  FXWindow*w=(FXWindow*)o;
  classlist->setCurrentItem(classlist->findItemByData(w->getUserData()));
  onSelectClass(classlist, 0, NULL);  
  for (w=w->getParent()->getFirst(); w; w=w->getNext()) {
    ((FXToggleButton*)w)->setState(w==o);
  }
  return 1;
}




// Called when the user selects a class in the classtree or classlist,
// or when the classfinder text field contains a partial class name.
long MainWin::onSelectClass(FXObject*o, FXSelector sel, void*p)
{
  memberfinder->setText(FXString::null);
  FXTreeItem *item=classtree->getCurrentItem();
  if (o==classlist) {
    item=classtree->findItemByData(classlist->getItemData(classlist->getCurrentItem()));
    if (item) {
      classtree->setCurrentItem(item);
      classtree->makeItemVisible(item);
    }
  }
  if (item) {
    Location*locn=(Location*)(item->getData());
    LoadLocation(locn);
    FXint n=classlist->findItemByData((void*)locn);
    if (n>=0) {
      classlist->setCurrentItem(n);
      classlist->selectItem(n);
      classlist->makeItemVisible(n);
    }
    if (sel) {
      classfinder->setText(item->getText());
      MakeBoxRed(classfinder,false);
      while (ancestry->getFirst()) { delete ancestry->getFirst(); }
      FXToggleButton*b;
      for (FXTreeItem *i=item; i; i=i->getParent()) {
        b=new FXToggleButton( ancestry, i->getText(), i->getText(), NULL, NULL, this,
                              ID_ANCESTRY_BTNS,TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE );
        b->create();
        b->reparent(ancestry,ancestry->getFirst());
        b->setUserData(i->getData());
      }
      ((FXToggleButton*)(ancestry->getLast()))->setState(true);
    }
  }
  return 1;
}




// When the user selects a function in the memberlist or otherlist, check
// to see if the implementation is defined in a source file rather than
// the header, and enable the "View Source" button if it is.
void MainWin::FindSourceFile(FXStringDictionary*dict)
{
  sourcebutton->disable();
  src_locns.clear();
  if ((Key("realkind")=="prototype")||(Key("kind")=="function")) {
    FXString impl_name=FXString::null;
    if (Key("class").empty()&&Key("namespace").empty()) {
      impl_name+=Key("name");
    } else {
      impl_name=classlist->getItemText(classlist->getCurrentItem()).section('@',0);
      impl_name+="::";
      impl_name+=Key("name");
    }
    impl_name+='\t';
    for (FXint idir=0; idir<tp->DirList().no(); idir++) {
      FileList*files=tp->DirList()[idir];
      for (FXint ifile=0; ifile<files->no(); ifile++) {
        const TagList*ft=files->at(ifile);
        for (FXint itag=0; itag<ft->no(); itag++) {
          const FXString* tagline=ft->at(itag);
          if (tagline->find(impl_name)==2) {
            const FXString src=ft->filename();
            if (src!=FXPath::name(filenamefield->getText())) {
              FXint line=tagline->section('\t',2).toInt();
              src_locns.append(idir,ifile,line);
            }
          }
        }
      }
    }
    if (src_locns.no()>0) {
      sourcebutton->enable();
    }
  }
}



// Called when the user selects something in the memberlist or otherlist,
// or when the memberfinder text field contains a partial tag name.
// Jumps to the source line in the hdr_view that contains the tag.
long MainWin::onSelectMember(FXObject*o, FXSelector sel, void*p)
{
  FXIconList*list=(FXIconList*)o;
  FXStringDictionary*dict=(FXStringDictionary*)list->getItemData(list->getCurrentItem());
  if (sel) {
    MakeBoxRed(memberfinder,false);
    memberfinder->setText(FXString::null);
  }
  if (dict) {
    FXString line=Key("line");
    if (sel) { memberfinder->setText(Key("name")); }
    if (!line.empty()) {
      FXbool ok=false;
      FXint linenum=line.toInt(10,&ok);
      if (ok) {
        FXText*view;
        if (classes_tabs->getCurrent()==2) {
          view=Ascii::toLower(FXPath::extension(hdrfile)[0])=='c'?src_view:hdr_view;
          ShowSource(view==src_view?hdrfile:FXString::null);
        } else {
          view=hdr_view;
          ShowSource(FXString::null);
        }
        linenum--;
        view->setCursorColumn(0);
        view->setCursorRow(linenum,true);
        FXint pos=view->getCursorPos();
        view->setCenterLine(pos);
        view->layout();
      }
    }
    FindSourceFile(dict);
  }
  return 1;
}



long MainWin::onActivateMember(FXObject*o, FXSelector sel, void*p)
{
  FXIconList*list=(FXIconList*)o;
  FXStringDictionary*dict=(FXStringDictionary*)list->getItemData(list->getCurrentItem());
  FXString opt_names = FXString::null;
  FXString id_names = FXString::null;
  if (dict) {
    FXToggleButton *tb=dynamic_cast<FXToggleButton*>(ancestry->getFirst());
    if (tb&&(compare(tb->getText(),"FXObject")==0)&&(compare(Key("kind"),"constructor")==0)) {
      DirectoryList *dirlist=&tp->DirList();
      FXRex rx("^[A-Z][A-Z0-9_]+$");
      for (FXint idir=0; idir<dirlist->no(); idir++) {
        FileList*filelist=dirlist->at(idir);
        for (FXint ifile=0; ifile<filelist->no(); ifile++) {
          TagList *tags=filelist->at(ifile);
          for (FXint itag=0; itag<tags->no(); itag++) {
            const FXString *tagtext=tags->at(itag);
            if ((*tagtext)[0]=='e') {
              FXString name=tagtext->section('\t',1);
              if (Ascii::isUpper(name[0])) {
                if (compare(name,"ID_",3)==0) {
                  id_names.append(name+'\n');
                } else {
                  if (rx.match(name)) {
                    opt_names.append(name+'\n');
                  }
                }
              }
            }
          }
        }
      }
    }
    ParamsDlg(this,dict,opt_names,id_names).execute(PLACEMENT_OWNER);
  }
  return 1;
}



void MainWin::ShowSource(const FXString &srcfile)
{
  if (!srcfile.empty()) {
    view_switch->setCurrent(1);
    filenamefield->setText(FXPath::simplify(srcfile));
    sourcebutton->setText("view header");
  } else {
    view_switch->setCurrent(0);
    sourcebutton->setText("view source");
    filenamefield->setText(FXPath::simplify(hdrfile));
  }
}



static const char* editors[]= {
  "fxite +%d %s",
  "adie -l %d %s",
  "nedit -line %d %s",
  "SciTE %s -goto:%d",
  "geany -l %d %s",
  NULL
};



// Open in external editor
long MainWin::onOpenButton(FXObject*o, FXSelector sel, void*p)
{
  FXString cmd=FXSystem::getEnvironment("FXCV_EDITOR");
  FXString arg=FXString::null;
  if (!cmd.empty()) {
    cmd.simplify();
    if (cmd.find(' ')>=0) {
      arg=cmd.section(' ', 1,1024);
      cmd=cmd.section(' ', 0);
    }
  } else {
    FXString path=FXSystem::getEnvironment("PATH");
    for (const char**editor=editors; *editor; editor++) {
      FXString ed=*editor;
      cmd=FXPath::search(path,ed.section(' ',0));
      if (!cmd.empty()) {
        if (ed.find(' ')>=0) { arg=ed.section(' ', 1,1024); }
        break;
      }
    }
  }
  if (cmd.empty()) { return 1; }
  if (arg.find("%s")>=0) {
    arg.substitute("%s", filenamefield->getText().text(), false);
  } else {
    arg+=" ";
    arg+=filenamefield->getText().text();
  }
  if (arg.find("%d")>=0) {
    FXint linenum=((FXText*)(view_switch->childAtIndex(view_switch->getCurrent())))->getCursorRow()+1;
    arg.substitute("%d",FXString::value(linenum).text());
  }
  cmd+=" "+arg+" &";
  system(cmd.text());
  return 1;
}



// Toggle between "view source" and "view header"
long MainWin::onSourceButton(FXObject*o, FXSelector sel, void*p)
{
  if (hdr_view->shown()) {
    FXint index=0;
    Location*locn=NULL;
    FXString srcfile=FXString::null;
    if (src_locns.no()>1) {
      FXString choices;
      for (FXint i=0; i<src_locns.no(); i++) {
        locn=src_locns[i];
        FXString path;
        tp->LocationToFullPath(path,locn);
        srcfile.format("%s:%d\n", path.text(),locn->line);
        choices.append(srcfile);
      }
      index=FXChoiceBox::ask(this, MBOX_OK_CANCEL,"Multiple definitions",
        "Please choose location",NULL,choices);
      if (index<0) {return 1;}
    }
    locn=src_locns[index];
    tp->LocationToFullPath(srcfile,locn);
    if (FXStat::isFile(srcfile)) {
      FXFile fh(srcfile,FXIO::Reading);
      if (fh.isOpen()) {
        tp->Contents().length(fh.size());
        fh.readBlock(tp->Contents().text(), fh.size());
        fh.close();
        Styler::ParseContents(src_view,stylebuf,tp->Contents());
        src_view->setCursorColumn(0);
        src_view->setCursorRow(locn->line-1,true);
        FXint pos=src_view->getCursorPos();
        src_view->setCenterLine(pos);
        src_view->setCursorColumn(0);
        ShowSource(srcfile);
      }
    }
  } else {
    ShowSource(FXString::null);
  }
  return 1;
}



//  We want to search for a matching class whenever the contents of the
//  classes search field changes. But to avoid excessive thrashing, we
//  wait until the typist has been idle for one second before starting
//  the search.
long MainWin::onSearchClasses(FXObject*o, FXSelector sel, void*p)
{
  const FXchar*text=(const FXchar*)p;
  MakeBoxRed(classfinder,false);
  if (text) {
    FXint len=strlen(text);
    if (len>=3) {
      getApp()->removeTimeout(this,ID_EDIT_TIMER);
      getApp()->addTimeout(this,ID_EDIT_TIMER,classfinder->hasFocus()?ONE_SECOND:1,classfinder);
    }
  }
  return 1;
}



// Similar to the onSearchClasses function above, but searches for an identifier
// in the active "members" or "others" description list.
long MainWin::onSearchMembers(FXObject*o, FXSelector sel, void*p)
{
  const FXchar*text=(const FXchar*)p;
  MakeBoxRed(memberfinder,false);
  if (text&&*text) {
    getApp()->removeTimeout(this,ID_EDIT_TIMER);
    getApp()->addTimeout(this,ID_EDIT_TIMER,memberfinder->hasFocus()?ONE_SECOND:1,memberfinder);
  }
  return 1;
}



// The user's flying fingers have settled down now, so try to find and select a matching item.
long MainWin::onEditTimer(FXObject*o, FXSelector sel, void*p)
{
  FXint start=0;
  FXint step=0;

  switch (FXSELID(sel)) {
    case ID_EDIT_TIMER:
    case ID_TOGGLE_ANCHORED: {
      start=FXSELTYPE(sel)==SEL_TIMEOUT?0:-3;
      step=FXSELTYPE(sel)==SEL_TIMEOUT?1:-1;
      break;
    }
    case ID_FIND_NEXT_CLASS: {
      start=-1;
      step=1;
      p=classfinder;
      break;
    }
    case ID_FIND_PREV_CLASS: {
      p=classfinder;
      start=-2;
      step=-1;
      break;
    }
    case ID_FIND_NEXT_MEMBER: {
      p=memberfinder;
      start=-1;
      step=1;
      break;
    }
    case ID_FIND_PREV_MEMBER: {
      p=memberfinder;
      start=-2;
      step=-1;
      break;
    }
  }


  if (p==classfinder) {
    const FXString text=classfinder->getText();
    if (text.length()>=3) {
      FXList *list=classes_tabs->getCurrent()==2?filelist:classlist;
      if (start==-3) { start=list->getNumItems()-1; }
      for (FXint i=start>=0?start:list->getCurrentItem()+step; (i>=0)&&(i<list->getNumItems()); i+=step) {
        if (comparecase(list->getItemText(i),text,text.length())==0) {
          list->selectItem(i,true);
          list->setCurrentItem(i,true);
          list->makeItemVisible(i);
          if (i>0) { list->makeItemVisible(i-1); }
          if (i<(list->getNumItems()-1)) { list->makeItemVisible(i+1); }
          if (list==classlist) {
            onSelectClass(list,FXSEL(SEL_COMMAND,ID_SELECT_CLASS),(void*)i);
          } else {
            onPickFile(list,0,(void*)i);
          }
          return 1;
        }
      }
    }
  } else {
    FXIconList *list=members_tabs->getCurrent()==0?memberlist:otherlist;
    const FXString needle=memberfinder->getText().lower();
    if (needle.empty()) { return 1; }
    MakeBoxRed(memberfinder,false);
    if (start==-3) { start=list->getNumItems()-1; }
    for (FXint i=start>=0?start:list->getCurrentItem()+step; (i>=0)&&(i<list->getNumItems()); i+=step) {
      const FXString sect=list->getItem(i)->getText().section('\t',3).lower();
      bool found=anchored->getCheck()?
             (comparecase(sect.text(),needle,needle.length())==0)
             : (sect.find(needle)>=0);
      if (found) {
         list->deselectItem(list->getCurrentItem());
         list->setCurrentItem(i,true);
         list->selectItem(i,true);
         list->makeItemVisible(i);
         if (i>0) { list->makeItemVisible(i-1); }
         if (i<(list->getNumItems()-1)) { list->makeItemVisible(i+1); }
         onSelectMember(list,0,(void*)i);
         return 1;
      }
    }
  }
  switch (start) { 
    case -1: {
      handle(this,FXSEL(SEL_TIMEOUT,ID_EDIT_TIMER),p);
      break;
    }
    case -2: { 
      handle(this,FXSEL(SEL_COMMAND,ID_EDIT_TIMER),p);
      break;
    }
    default:MakeBoxRed((FXTextField*)p,true);
  }
  return 1;
}



// Called when the user selects an item in the search results.
long MainWin::onSelectResult(FXObject*o, FXSelector sel, void*p)
{
  Location*locn=(Location*)(search_results->getItemData((FXuval)p));
  LoadLocation(locn,true);
  FXIconList*lists[]={memberlist,otherlist,NULL};
  for (FXIconList**list=lists; *list; list++) {
    for (FXint imemb=0; imemb<(*list)->getNumItems(); imemb++) {
      FXStringDictionary*dict=(FXStringDictionary*)(*list)->getItemData(imemb);
      if (Key("line").toInt()==locn->line) {
        FXString classname=Key("class");
        TagParser::StripNamespace(classname);
        FXint foundclass=-1;
        if (classname.empty()) {
          for (FXint iclass=0; iclass<classlist->getNumItems(); iclass++) {
            Location*locn2=(Location*)classlist->getItemData(iclass);
            if ((locn2->ifile==locn->ifile)&&(locn2->idir==locn->idir)) {
              foundclass=iclass;
              break;  
            }
          }
        } else {
          for (FXint iclass=0; iclass<classlist->getNumItems(); iclass++) {
            if (classlist->getItemText(iclass)==classname) {
              foundclass=iclass;
              break;
            }
          }          
        }
        if (foundclass>=0) {
          classlist->setCurrentItem(foundclass);
          classlist->makeItemVisible(foundclass);
          classtree->setCurrentItem(classtree->findItemByData(classlist->getItemData(foundclass)));
          classtree->makeItemVisible(classtree->getCurrentItem());
        }
        (*list)->setCurrentItem(imemb);
        (*list)->selectItem(imemb);
        (*list)->makeItemVisible(imemb);
        members_tabs->setCurrent((members_tabs->indexOfChild((*list)->getParent())-1)/2);
        return 1;      
      }
    }
  }
  return 1;
}



// Called when one of the checkboxes on the search panel gets toggled.
long MainWin::onSetSearchOpt(FXObject*o, FXSelector sel, void*p)
{
  bool chk=((FXCheckButton*)o)->getCheck();
  if (o==anchor_chk) {
    if (chk) {
      search_opts|=SRCH_ANCHORED;
      search_opts&=~SRCH_REGEX;
      search_opts&=~SRCH_WHOLEWORD;
    } else {
      search_opts&=~SRCH_ANCHORED; 
    }
  } else if (o==case_chk) {
    if (chk) {
      search_opts|=SRCH_CASEMATCH;
    } else {
      search_opts&=~SRCH_CASEMATCH; 
    }
  } else if (o==regex_chk) {
    if (chk) {
      search_opts|=SRCH_REGEX;
      search_opts|=SRCH_CASEMATCH;
      search_opts&=~SRCH_WHOLEWORD;
      search_opts&=~SRCH_ANCHORED;
    } else {
      search_opts&=~SRCH_REGEX;
    }
  } else if (o==whole_chk) { 
    if (chk) {
      search_opts|=SRCH_WHOLEWORD;
      search_opts&=~SRCH_ANCHORED;
      search_opts&=~SRCH_REGEX;
    } else {
      search_opts&=~SRCH_WHOLEWORD; 
    }
  }
  if (o != anchor_chk) { anchor_chk -> setCheck(search_opts&SRCH_ANCHORED?1:0); }
  if (o != case_chk)   { case_chk   -> setCheck(search_opts&SRCH_CASEMATCH?1:0); }
  if (o != regex_chk)  { regex_chk  -> setCheck(search_opts&SRCH_REGEX?1:0); }
  if (o != whole_chk)  { whole_chk  -> setCheck(search_opts&SRCH_WHOLEWORD?1:0); }
  return 1;
}




bool MainWin::SearchMatch(const FXString&needle, const FXString&haystack, FXRex*rx)
{
  if (search_opts&SRCH_REGEX) { 
    return (rx->match(haystack))?true:false;
  } else {
    FXString hs=haystack;
    if (!(search_opts&SRCH_CASEMATCH)) { hs=hs.lower(); }
    for (FXint i=0; i<=hs.contains(':'); i++) {
      FXString sect=hs.section(':',i);
      if (search_opts&SRCH_WHOLEWORD) {
        if (compare(needle,sect)==0) { return true; }
      } else if (search_opts&SRCH_ANCHORED) {
        if (compare(needle,sect,needle.length())==0) { return true; }
      } else {
        if (sect.find(needle)>=0) { return true; }
      }
    }
    return false;
  }
}



long MainWin::onSearchAll(FXObject*o, FXSelector sel, void*p)
{
  FXString srch=search_field->getText();
  if (!(search_opts&SRCH_CASEMATCH)) srch=srch.lower();
  FXRex*rx = NULL;
  if (search_opts&SRCH_REGEX) {
    FXRexError err;
    rx=new FXRex( search_field->getText(),
                    FXRex::Normal|(search_opts&SRCH_CASEMATCH?0:FXRex::IgnoreCase),&err );
    if (err!=FXRex::ErrOK) {
      FXMessageBox::error(this, MBOX_OK,
        APP_NAME,
        "Error parsing regular expression:\n%s",
        FXRex::getError(err)
      );
      delete rx;
      return 1;
    }
  }
  DirectoryList *dirlist=&tp->DirList();
  ClearLocationList(search_results);
  for (FXint idir=0; idir<dirlist->no(); idir++) {
    FileList*filelist=dirlist->at(idir);
    for (FXint ifile=0; ifile<filelist->no(); ifile++) {
      TagList *tags=filelist->at(ifile);
      for (FXint itag=0; itag<tags->no(); itag++) {
        const FXString *tagtext=tags->at(itag);
        if (SearchMatch(srch,tagtext->section('\t',1),rx)) {
          Location*locn=new Location();
          locn->kind=tagtext->at(0);
          locn->idir=idir;
          locn->ifile=ifile;
          locn->line=tagtext->section('\t',2).toInt();
          search_results->appendItem("("+
            tagtext->section('\t',0).upper()+") "+
            tagtext->section('\t',1)+"@"+
            tags->filename()+":"+
            tagtext->section('\t',2),
            NULL,
            (void*)locn
          );
        }
      }
    }
  }
  if (rx) { delete rx; }
  if (search_results->getNumItems()>0) { onSelectResult(NULL,0,NULL); }
  return 1;
}




void MainWin::BuildClassTree()
{
  ClearClassTree();
  tp->CreateTreeItems();
  if (classlist->getNumItems()==0) {
    classes_tabs->setCurrent(2);
    onPickDirectory(dirbox,0,NULL);
    onPickFile(filelist,0,NULL);
  }
  classlist->sortItems();
  classtree->sortItems();
}



void MainWin::ClearList(FXIconList*list)
{
  for (FXint i=0; i<list->getNumItems(); i++) {
    delete (FXStringDictionary*)(list->getItemData(i));
  }
  list->clearItems();
}



void MainWin::ClearLocationList(FXList*list)
{
  for (FXint i=0; i<list->getNumItems(); i++) {
    Location*locn=(Location*)list->getItemData(i);
    if (locn) {
      delete locn;
    }
  }
  list->clearItems();
}



void MainWin::ClearClassTree()
{
  ClearList(memberlist);
  ClearList(otherlist);
  ClearLocationList(classlist);
  ClearLocationList(search_results);
  classtree->clearItems();
}



void MainWin::UpdateDirBox()
{
  dirbox->clearItems();
  filelist->clearItems();
  for (FXint i=0; i<tp->DirList().no(); i++) {
    dirbox->appendItem(tp->DirList().at(i)->dirname(),NULL,(void*)tp->DirList()[i]);
  }
  dirbox->setNumVisible(dirbox->getNumItems()>16?16:dirbox->getNumItems());
  onPickDirectory(NULL,0,NULL);
}



long MainWin::onEditPathList(FXObject*o, FXSelector sel, void*p)
{
  PathDlg dlg(this,tp);
  if (dlg.execute()) {
    UpdateDirBox();
    while (ancestry->getFirst()) { delete ancestry->getFirst(); }
    onSearchClasses( classfinder,FXSEL(SEL_CHANGED,ID_SEARCH_CLASSES),
                     (void*)(classfinder->getText().text()) );
  }
  return 1;
}



void MainWin::SetCodeFont(const FXString &name, FXint size)
{
  FXFont *fnt=new FXFont(getApp(),name);
  fnt->create();
  FXFontDesc desc=fnt->getActualFontDesc();
  desc.size=size;
  fnt->destroy();
  fnt->setFontDesc(desc);
  fnt->create();
  hdr_view->setFont(fnt);
  src_view->setFont(fnt);
  delete mono;
  mono=fnt;
}



long MainWin::onSetCodeFont(FXObject*o, FXSelector sel, void*p)
{
  FXFontDialog dlg(this, "Set source viewer font");
  ((MyFontDlg*)&dlg)->setup();
  dlg.setFontDesc(hdr_view->getFont()->getActualFontDesc());
  if (dlg.execute(PLACEMENT_OWNER)) {
    FXFontDesc desc=dlg.getFontDesc();
    SetCodeFont(desc.face,desc.size);
  }
  return 1;
}



void MainWin::Splash(FXApp*a)
{
  static FXSplashWindow*splash = NULL;
  if (a) {
    FXIcon*ico=new FXXPMIcon(a,fxcv_xpm,0,IMAGE_OWNED);
    ico->scale(128,128);
    splash = new FXSplashWindow(a,ico,SPLASH_SIMPLE|SPLASH_OWNS_ICON,ONE_SECOND*15);
    a->getRootWindow()->create();
    FXint SplashX=(a->getRootWindow()->getWidth()-128)/2;
    FXint SplashY=(a->getRootWindow()->getHeight()-128)/2;
    a->getRootWindow()->destroy();
    splash->setX(SplashX);
    splash->setY(SplashY);
    splash->show(PLACEMENT_SCREEN);
  } else {
    delete splash;
    splash=NULL;
  }
}




static const char*guireg="GUI";


void MainWin::create()
{
  FXMainWindow::create();

  BuildClassTree();
  UpdateDirBox();
  Splash(NULL);

  classfinder->setFocus();

  FXRegistry *reg=&(getApp()->reg());
  FXint desk_wdt=getApp()->getRootWindow()->getWidth();
  FXint desk_hgt=getApp()->getRootWindow()->getHeight();

  setWidth(reg->readIntEntry(guireg,"window.width",desk_wdt*0.95));
  setHeight(reg->readIntEntry(guireg,"window.height",desk_hgt*0.80));

  if (reg->readIntEntry(guireg,"window.state",0)) { maximize(); }

  layout();

  setX(reg->readIntEntry(guireg,"window.left",desk_wdt/(0.95*2)));
  setY(reg->readIntEntry(guireg,"window.top",desk_hgt/(0.80*2)));

  hsplit->setSplit(0,reg->readIntEntry(guireg,"split.width",hsplit->getWidth()/3));
  vsplit->setSplit(0,reg->readIntEntry(guireg,"split.height",vsplit->getHeight()/2));


  FXfloat incr=memberlist->getWidth()/100.0;
  memberlist->setHeaderSize(0,incr*12);
  memberlist->setHeaderSize(1,incr*15);
  memberlist->setHeaderSize(2,incr*10);
  memberlist->setHeaderSize(3,incr*100);

  otherlist->setHeaderSize(0,incr*15);
  otherlist->setHeaderSize(1,incr*15);
  otherlist->setHeaderSize(2,incr*15);
  otherlist->setHeaderSize(3,incr*55);

  mono=NULL;
  SetCodeFont(reg->readStringEntry(guireg,"font.name","courier"),reg->readIntEntry(guireg,"font.size",100));

  hdr_view->setHiliteBackColor(FXRGB(255,255,128));
  src_view->setHiliteBackColor(FXRGB(255,255,128));

  if (initclass.empty()&&(classlist->getNumItems()>0)) {
    initclass=classlist->getItem(0)->getText();
  }
  classfinder->setText(initclass,true);
  onSearchClasses( classfinder,FXSEL(SEL_CHANGED,ID_SEARCH_CLASSES),
                        (void*)(classfinder->getText().text()) );

  classes_tabs->setCurrent(reg->readIntEntry(guireg,"tabs.classes",0));
  FXIcon*ico=new FXXPMIcon(getApp(),fxcv_xpm,0,IMAGE_KEEP);
  ico->create();
  setIcon(ico);
  SetupXAtoms(this,"FXCodeView","FXCodeView");
  show(PLACEMENT_SCREEN);
}



MainWin::~MainWin()
{
  ClearClassTree();
  delete tp;
  FXRegistry *reg=&(getApp()->reg());
  reg->writeIntEntry(guireg,"window.width", getWidth());
  reg->writeIntEntry(guireg,"window.height", getHeight());
  reg->writeIntEntry(guireg,"window.left", getX());
  reg->writeIntEntry(guireg,"window.top", getY());
  reg->writeIntEntry(guireg,"window.state", isMaximized());
  reg->writeIntEntry(guireg,"split.width", hsplit->getSplit(0));
  reg->writeIntEntry(guireg,"split.height", vsplit->getSplit(0));
  reg->writeIntEntry(guireg,"tabs.classes", classes_tabs->getCurrent());
  reg->writeIntEntry(guireg,"tabs.members", members_tabs->getCurrent());
  reg->writeIntEntry(guireg,"font.size", mono->getActualSize());
  reg->writeStringEntry(guireg,"font.name", mono->getActualName().text());
  reg->write();
  mono->destroy();
  delete mono;
}





#define SetPadLRTB(o,l,r,t,b) \
  o->setPadLeft(l); \
  o->setPadRight(r); \
  o->setPadTop(t); \
  o->setPadBottom(b);


#define SetSpaceHV(o,h,v) \
  o->setHSpacing(h); \
  o->setVSpacing(v);


#define LIST_OPTS \
  TREELIST_BROWSESELECT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_ROOT_BOXES


MainWin::MainWin(FXApp* a):MainWinWithClipBrd(a, APP_NAME, NULL, NULL,DECOR_ALL, 0,0,800,600)
{
  new FXToolTip(getApp());
  FXHorizontalFrame*hframe;
  FXVerticalFrame *vframe;
  vframe=new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);
  vframe->setVSpacing(0);

  hsplit=new FXSplitter(vframe,SPLITTER_NORMAL|LAYOUT_FILL);

  statbar=new FXHorizontalFrame(vframe,FRAME_RAISED|LAYOUT_FILL_X);
  SetPadLRTB(statbar,0,0,0,0);
  statbar->setHSpacing(1);
  new FXLabel(statbar,"Lineage:");

  ancestry=new FXHorizontalFrame(statbar,FRAME_NONE/*|LAYOUT_FILL_X*/);
  SetPadLRTB(ancestry,4,2,1,2);
  ancestry->setHSpacing(3);


  vframe=new FXVerticalFrame(hsplit,FRAME_SUNKEN|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);

  hframe=new FXHorizontalFrame(vframe,LAYOUT_FILL_X|FRAME_RAISED);

  SetPadLRTB(hframe,0,0,0,0);
  new FXLabel(hframe,"Find:");
  classfinder=new MyTextField(hframe, 32, this, ID_SEARCH_CLASSES,TEXTFIELD_NORMAL|LAYOUT_FILL_X);
  new FXButton(hframe, "!", NULL, this,ID_CLEAR_FIELD);
  new FXButton(hframe, " < ", NULL, this, ID_FIND_PREV_CLASS);
  new FXButton(hframe, " > ", NULL, this, ID_FIND_NEXT_CLASS);

  classes_tabs=new FXTabBook(vframe,this,ID_LEFT_PANE_TABS,LAYOUT_FILL);

  new MyTabItem(classes_tabs," List ");
  vframe=new FXVerticalFrame(classes_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);
  classlist=new FXList(vframe,this,ID_SELECT_CLASS,LIST_NORMAL|LIST_BROWSESELECT|LAYOUT_FILL);
  classlist->setSortFunc(ClassListSortFunc);

  new MyTabItem(classes_tabs," Tree ");
  vframe=new FXVerticalFrame(classes_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);
  SetSpaceHV(vframe,0,0);
  classtree=new MyTreeList(vframe,this,ID_SELECT_CLASS,LIST_OPTS|LAYOUT_FILL);
  classtree->setSortFunc(ClassTreeSortFunc);

  new MyTabItem(classes_tabs," Files ");
  vframe=new FXVerticalFrame(classes_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);
  SetSpaceHV(vframe,0,0);

  hframe=new FXHorizontalFrame(vframe,LAYOUT_FILL_X|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  SetPadLRTB(hframe,0,0,0,4);
  SetSpaceHV(hframe,0,0);
  dirbox=new FXListBox(hframe,this,ID_PICK_DIRECTORY,LISTBOX_NORMAL|LAYOUT_FILL_X);
  SetPadLRTB(dirbox,0,0,0,0);
  filelist=new FXList(vframe,this,ID_PICK_FILE,LIST_NORMAL|LIST_BROWSESELECT|LAYOUT_FILL);
  filelist->setSortFunc(FileListSortFunc);


  new MyTabItem(classes_tabs," Search ");
  vframe=new FXVerticalFrame(classes_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);

  search_opts=0;

  hframe=new FXHorizontalFrame(vframe,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  SetPadLRTB(hframe,0,0,0,0);
  search_field=new FXTextField(hframe,24,this,ID_SEARCH_ALL,TEXTFIELD_NORMAL|LAYOUT_FILL_X);
  new FXButton(hframe, "!", NULL, this,ID_CLEAR_FIELD);
  new FXButton(hframe, "Search", NULL, this, ID_SEARCH_ALL);
  FXMatrix *mtx=new FXMatrix(vframe,2,2);

  anchor_chk = new FXCheckButton(mtx, "Anchored",   this, ID_SET_SEARCH_OPT);
  whole_chk  = new FXCheckButton(mtx, "Whole word", this, ID_SET_SEARCH_OPT);
  regex_chk  = new FXCheckButton(mtx, "Reg Expr",   this, ID_SET_SEARCH_OPT);
  case_chk   = new FXCheckButton(mtx, "Match case", this, ID_SET_SEARCH_OPT);

  vframe=new FXVerticalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);
  search_results=new FXList(vframe,this,
    ID_SELECT_RESULT,LIST_NORMAL|LIST_BROWSESELECT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL);

  ((MyTextField*)classfinder)->SetTabFocusDn(classes_tabs);
  vframe=new FXVerticalFrame(hsplit,FRAME_SUNKEN|LAYOUT_FILL);
  vsplit=new FXSplitter(vframe,SPLITTER_VERTICAL|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);
  vframe=new FXVerticalFrame(vsplit,FRAME_SUNKEN|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);

  hframe=new FXHorizontalFrame(vframe,LAYOUT_FILL_X|FRAME_RAISED);
  SetPadLRTB(hframe,0,0,0,0);
  new FXLabel(hframe,"Find:");
  memberfinder=new MyTextField(hframe, 32, this, ID_SEARCH_MEMBERS, TEXTFIELD_NORMAL|LAYOUT_FILL_X);
  new FXButton(hframe, "!", NULL, this,ID_CLEAR_FIELD);
  new FXButton(hframe, " < ", NULL, this, ID_FIND_PREV_MEMBER);
  new FXButton(hframe, " > ", NULL, this, ID_FIND_NEXT_MEMBER);
  anchored= new FXCheckButton(hframe,"Anchored",this,ID_TOGGLE_ANCHORED);
  anchored->setCheck(false);
  anchored->setTipText("If checked, searches match only from start of identifier");

  hframe=new FXHorizontalFrame(hframe,LAYOUT_FILL|FRAME_NONE);
  SetPadLRTB(hframe,0,0,0,0);

  path_btn=new FXButton(hframe, " Setup... ", NULL, this, ID_EDIT_PATHLIST, BUTTON_NORMAL|LAYOUT_RIGHT);
  
  members_tabs=new FXTabBook(vframe,NULL,0,LAYOUT_FILL);

  new MyTabItem(members_tabs," methods ");
  vframe=new FXVerticalFrame(members_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);
  memberlist=new MyIconList(vframe,this,ID_SELECT_MEMBER);
  memberlist->appendHeader("Access");
  memberlist->appendHeader("Kind");
  memberlist->appendHeader("Type");
  memberlist->appendHeader("Description");

  new MyTabItem(members_tabs," other ");
  vframe=new FXVerticalFrame(members_tabs,FRAME_RIDGE|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,4,0);
  otherlist=new MyIconList(vframe,this,ID_SELECT_MEMBER);
  otherlist->appendHeader("Access");
  otherlist->appendHeader("Kind");
  otherlist->appendHeader("Type");
  otherlist->appendHeader("Description");

  ((MyTextField*)memberfinder)->SetTabFocusDn(members_tabs);

  vframe=new FXVerticalFrame(vsplit,FRAME_SUNKEN|LAYOUT_FILL);
  SetPadLRTB(vframe,0,0,0,0);
  view_switch=new FXSwitcher(vframe,LAYOUT_FILL);
  SetPadLRTB(view_switch,0,0,0,0);
  hdr_view=new MyText(view_switch);
  src_view=new MyText(view_switch);
  Styler::InitStyles();
  hframe=new FXHorizontalFrame(vframe,LAYOUT_FILL_X);
  SetPadLRTB(hframe,2,2,0,2);
  new FXButton(hframe, "A",NULL,this,ID_SET_CODE_FONT);
  openbutton=new FXButton(hframe, "open",NULL,this,ID_OPEN_BUTTON);
  sourcebutton=new FXButton(hframe, "view source",NULL,this,ID_SOURCE_BUTTON);
  sourcebutton->disable();
  filenamefield=new FXTextField(hframe,64,NULL,0,TEXTFIELD_READONLY|TEXTFIELD_NORMAL);
  ShowSource(FXString::null);

  ((MyText*)hdr_view)->SetTabFocusUpDn(members_tabs->getFirst(),filenamefield);
  ((MyText*)src_view)->SetTabFocusUpDn(members_tabs->getFirst(),filenamefield);
  ((MyIconList*)memberlist)->SetTabFocusDn(view_switch);
  ((MyIconList*)otherlist)->SetTabFocusDn(view_switch);
  tp=new TagParser(this,classtree,classlist,memberlist,otherlist);
  status_label=new FXLabel(statbar,FXString::null);
  SetPadLRTB(status_label,0,0,0,0);
}

