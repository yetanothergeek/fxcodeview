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

#ifndef FXCV_H
#define FXCV_H
#include "histbox.h"
#include "tagger.h"


/*
  Arguments to the --fields option for ctags:
    a: Access (or export) of class members
    f: File-restricted scoping [enabled]
    i: Inheritance information
    k: Kind of tag as a single letter [enabled]
    K: Kind of tag as full name
    l: Language of source file containing tag
    m: Implementation information
    n: Line number of tag definition
    s: Scope of tag definition [enabled]
    S: Signature of routine (e.g. prototype or parameter list)
    z: Include the "kind:" key in kind field
    t: Type and name of a variable or typedef as "typeref:" field [enabled]
*/

/*
C
    c  classes
    d  macro definitions
    e  enumerators (values inside an enumeration)
    f  function definitions
    g  enumeration names
    l  local variables [off]
    m  class, struct, and union members
    n  namespaces
    p  function prototypes [off]
    s  structure names
    t  typedefs
    u  union names
    v  variable definitions
    x  external and forward variable declarations [off]
C++
    c  classes
    d  macro definitions
    e  enumerators (values inside an enumeration)
    f  function definitions
    g  enumeration names
    l  local variables [off]
    m  class, struct, and union members
    n  namespaces
    p  function prototypes [off]
    s  structure names
    t  typedefs
    u  union names
    v  variable definitions
    x  external and forward variable declarations [off]
*/


#define APP_NAME "FXCodeView"

class TagParser;


enum {
  SRCH_ANCHORED  = 1,
  SRCH_CASEMATCH = 2,
  SRCH_REGEX     = 4,
  SRCH_WHOLEWORD = 8,
};



class MainWin : public MainWinWithClipBrd {
  FXDECLARE(MainWin)
protected:
  MainWin(){}

  void BuildClassTree();
  void ClearClassTree();
  void ClearList(FXIconList*list);
  void ClearLocationList(FXList*list);
  void FindSourceFile(FXStringDictionary*dict);
  void ShowSource(const FXString &srcfile);
  void MakeBoxRed(FXTextField*box, bool iserror);
  bool SearchMatch(const FXString&needle, const FXString&haystack, FXRex*rx);
  static FXint ClassListSortFunc(const FXListItem*a,const FXListItem*b) {
    return comparecase(a->getText(),b->getText());
  }
  static FXint FileListSortFunc(const FXListItem*a,const FXListItem*b) {
    return compare(a->getText(),b->getText());
  }
  static FXint ClassTreeSortFunc(const FXTreeItem*a,const FXTreeItem*b) {
    return comparecase(a->getText(),b->getText());
  }
  void LoadLocation(Location*locn, bool wholefile=false);
  void UpdateDirBox();
  void SetCodeFont(const FXString &name, FXint size);


  TagParser *tp;
  FXString hdrfile;
  LocationList src_locns;
  FXTabBook *classes_tabs;
  FXTreeList *classtree;
  FXList *classlist;
  FXListBox *dirbox;
  FXList *filelist;
  FXTextField *classfinder;
  FXTabBook *members_tabs; 
  FXIconList *memberlist;
  FXIconList *otherlist;
  FXTextField *memberfinder;
  FXCheckButton *anchored;
  FXText *hdr_view;
  FXText *src_view;
  FXSwitcher *view_switch;
  FXButton *sourcebutton;
  FXButton *openbutton;
  FXTextField *filenamefield;
  FXSplitter *hsplit;
  FXSplitter *vsplit;
  FXString stylebuf;
  FXHorizontalFrame *statbar;
  FXHorizontalFrame *ancestry;
  FXLabel *status_label;
  FXFont *mono;
  FXTextField *search_field;
  FXList *search_results;
  FXCheckButton *anchor_chk;
  FXCheckButton *case_chk;
  FXCheckButton *regex_chk;
  FXCheckButton *whole_chk;
  FXuint search_opts;
  FXButton *path_btn;
public:
  static const FXString&InitialClass();
  static void InitialClass(const FXString&classname);
  long onSelectResult(      FXObject*o, FXSelector sel, void*p);
  long onSelectClass(       FXObject*o, FXSelector sel, void*p);
  long onSelectMember(      FXObject*o, FXSelector sel, void*p);
  long onSearchClasses(     FXObject*o, FXSelector sel, void*p);
  long onSearchMembers(     FXObject*o, FXSelector sel, void*p);
  long onSearchAll(         FXObject*o, FXSelector sel, void*p);
  long onEditTimer(         FXObject*o, FXSelector sel, void*p);
  long onIpcExec(           FXObject*o, FXSelector sel, void*p);
  long onSetCodeFont(       FXObject*o, FXSelector sel, void*p);
  long onSourceButton(      FXObject*o, FXSelector sel, void*p);
  long onOpenButton(        FXObject*o, FXSelector sel, void*p);
  long onPickDirectory(     FXObject*o, FXSelector sel, void*p);
  long onPickFile(          FXObject*o, FXSelector sel, void*p);
  long onLeftPaneTabs(      FXObject*o, FXSelector sel, void*p);
  long onSetSearchOpt(      FXObject*o, FXSelector sel, void*p);
  long onClearField(        FXObject*o, FXSelector sel, void*p);
  long onEditPathList(      FXObject*o, FXSelector sel, void*p);
  long onAncestryBtns(      FXObject*o, FXSelector sel, void*p);
  long onActivateMember(    FXObject*o, FXSelector sel, void*p);
  enum {
    ID_SELECT_CLASS=FXMainWindow::ID_LAST,
    ID_SELECT_MEMBER,
    ID_SELECT_RESULT,
    ID_SEARCH_CLASSES,
    ID_SEARCH_MEMBERS,
    ID_SEARCH_ALL,
    ID_SET_SEARCH_OPT,
    ID_TOGGLE_ANCHORED,
    ID_EDIT_TIMER,
    ID_FIND_NEXT_MEMBER,
    ID_FIND_PREV_MEMBER,
    ID_FIND_NEXT_CLASS,
    ID_FIND_PREV_CLASS,
    ID_IPC_EXEC,
    ID_SET_CODE_FONT,
    ID_SOURCE_BUTTON,
    ID_OPEN_BUTTON,
    ID_PICK_DIRECTORY,
    ID_PICK_FILE,
    ID_LEFT_PANE_TABS,
    ID_CLEAR_FIELD,
    ID_EDIT_PATHLIST,
    ID_ANCESTRY_BTNS,
    ID_LAST
  };
  MainWin(FXApp* a);
  virtual void create();
  virtual ~MainWin();
  static void Splash(FXApp*a);
};

#endif

