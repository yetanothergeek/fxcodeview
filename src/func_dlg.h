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


#ifndef FUNC_DLG_H
#define FUNC_DLG_H
#include "histbox.h"

class ParamsDlg: public FXDialogBox {
private:
  FXMatrix *mtx;
  FXStringDict*dict;
  MainWinWithClipBrd*mw;
protected:
  FXDECLARE(ParamsDlg)
  ParamsDlg() {}
public:
  long onEdit(FXObject*o, FXSelector sel, void*p);
  ParamsDlg(MainWinWithClipBrd*owner, FXStringDict*d, const FXString &opt_names, const FXString &id_names);
  virtual FXuint execute(FXuint placement=PLACEMENT_OWNER);
  enum {
    ID_EDIT=FXDialogBox::ID_LAST,
    ID_LAST
  };
};

#endif

