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
#include "intl.h"
#include "path_dlg.h"
#include "tagger.h"


static const char*sysinc_intro=NULL;


PathDlg::PathDlg(FXWindow* w, TagParser* parser):
  DescListDlg(w, _("Edit search directories"), _("Search Directories"),
  _("Directory to search for source files"), NULL,-1,0,64,true), tp(parser)
{

  before.clear();
  FXString dirs=FXString::null;
  for (FXint i=0; i<tp->DirList().no(); i++) {
    FileList *files=tp->DirList()[i];
    dirs+=files->dirname();
    dirs+='\n';
  }
  before=dirs;

}



void PathDlg::setText(const FXString str)
{
  items->clearItems();
  for (FXint i=0; i<str.contains('\n'); i++) {
    FXString sect=str.section('\n',i);
    items->appendItem('\t' + sect.simplify());
  }
}



const FXString& PathDlg::getText()
{
  after.clear();
  for (FXint i=0; i<items->getNumItems(); i++) {
    FXString item=items->getItemText(i)+'\n';
    item.erase(0,1);
    after+=item;
  }
  return after;
}



bool PathDlg::Verify(FXString&item)
{
  item.simplify();
  if (item.empty()) {
    FXMessageBox::error(this, MBOX_OK, _("Error"), _("Path must not be empty."));
    return false;
  } else {
    return true;
  }
}



void PathDlg::RestoreAppDefaults()
{
  FXString dirs=FXString::null;
  TagParser::SetDefaultSearchPaths();
  for (FXint i=0; i<TagParser::DirList().no(); i++) {
    FileList *files=TagParser::DirList()[i];
    dirs+=files->dirname();
    dirs+='\n';
  }
  setText(dirs);
}



bool PathDlg::Browse(FXString &text)
{
  FXDirDialog dlg(this, _("Select include path"));
  dlg.setDirectory(text);
  if (dlg.execute()) {
    text=dlg.getDirectory();
    return true;
  } else {
    return false;
  }
}



FXuint PathDlg::execute(FXuint placement)
{
  if (DescListDlg::execute(placement)) {
    tp->DirList().clear();
    FXString txt=getText();
    for (FXint i=0; i<txt.contains('\n'); i++) {
      tp->DirList().append(txt.section('\n',i).text());
    }
    getApp()->beginWaitCursor();
    hide();
    tp->CreateTreeItems();
    getApp()->endWaitCursor();
    return true;
  } else {
    return false;
  }
}
