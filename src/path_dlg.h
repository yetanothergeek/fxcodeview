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


#ifndef PATH_DLG_H
#define PATH_DLG_H
#include "desclistdlg.h"

class TagParser; 


class PathDlg: public DescListDlg {
  virtual void setText(const FXString str);
  virtual const FXString& getText();
  virtual bool Verify(FXString &item);
  virtual void RestoreAppDefaults();
  virtual bool Browse(FXString &text);
  TagParser*tp;
public:
  virtual FXuint execute(FXuint placement=PLACEMENT_SCREEN);
  PathDlg(FXWindow*w, TagParser*parser);
};

#endif
