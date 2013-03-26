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

#ifndef TAGPARSE_H
#define TAGPARSE_H
#include "taggerbase.h"


// Manages a list of Location structs (and their required memory.)
class LocationList: public FXObjectListOf<Location> {
public:
  FXbool append(LocationIndex idir, LocationIndex ifile, FXint line=0, FXchar kind='f');
  void clear();
  ~LocationList();
};


// Extends the TagParserBase class by providing more specific ctags
// information for individual classes, etc.
class TagParser: public TagParserBase {
  FXDECLARE(TagParser)
  TagParser() {}
  FXString contents;
  FXIconList*memberlist;
  FXIconList*otherlist;
  bool wholefile;
  bool ParseClassTag(FXStringDict *dict, const FXString &thisclass);
  bool ParseOtherTag(FXStringDict *dict);
public:
  TagParser(FXMainWindow*win, FXTreeList*clstree, FXList*clslist, FXIconList*members, FXIconList*others):
  TagParserBase(win, clstree, clslist) {
    memberlist=members;
    otherlist=others;
  }
  long onParseSingleHeader(FXObject*o, FXSelector sel, void*p);
  void ParseSingle(const FXString &filename, bool entire_file=false);
  FXString& Contents() { return contents; }
  enum {
    ID_READ_SINGLE_FILE_LINES=TagParserBase::ID_LAST,
    ID_LAST
  };
};

#endif
