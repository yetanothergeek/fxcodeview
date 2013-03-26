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
#include "runcmd.h"
#include "tagger.h"
#include "guesstype.h"

// Allocates a new Location struct and adds it to the list.
FXbool LocationList::append(LocationIndex idir, LocationIndex ifile, FXint line, FXchar kind)
{ 
    Location*locn=new Location;
    locn->ifile=ifile;
    locn->line=line;
    locn->kind=kind;
    locn->idir=idir;
    return FXObjectList::append((FXObject*)locn);
}


// Removes all Location structs from the list and releases their memory.
void LocationList::clear()
{
  for (FXint i=0; i<no(); i++) {
    Location*locn=at(i);
    if (locn) {
      delete locn;
      replace(i,NULL);
    }
  }
  FXObjectList::clear();
}



LocationList::~LocationList()
{
  clear();
}



FXDEFMAP(TagParser) TagParserMap[]={
  FXMAPFUNC(SEL_IO_WRITE,  TagParser::ID_READ_SINGLE_FILE_LINES, TagParser::onParseSingleHeader),
  FXMAPFUNC(SEL_IO_EXCEPT, TagParser::ID_READ_SINGLE_FILE_LINES, TagParser::onParseSingleHeader),
};

FXIMPLEMENT(TagParser,TagParserBase,TagParserMap,ARRAYNUMBER(TagParserMap))



bool TagParser::ParseClassTag(FXStringDict *dict, const FXString &thisclass)
{
  if (Key("access")=="private") {return false;}
  FXString all=FXString::null;
  FXString name=Key("name").text();
  FXString kind=Key("kind");
  StripNamespace(name);
  if (kind=="prototype") {
    kind=thisclass.empty()?"function":"method";
    dict->insert("realkind","prototype");
  }
  if (!thisclass.empty()) {
    if (kind=="function") { kind="method"; }
    if ( Key("type").empty() && (kind=="method") ) {
      if ((name[0]=='~')&&thisclass==(name.text()+1)) {
        kind="destructor";
      } else if (name==thisclass) {
        kind="constructor";
      }
    }
  }
  if ((kind!="constructor")&&(kind!="destructor")) {
    GetTypeInfo(dict,name,kind,contents);
  }
  if (name.find("operator ")==0) {
    name.erase(0,9);
    name.append(" ");
    kind="operator";
  }
  all.format( "%s\t%s\t%s\t%s%s",
    Key("access").text(),
    kind.text(),
    Key("type").text(),
    name.text(),
    Key("signature").text()
  );
  if ((kind=="method")||(kind=="operator")||(kind=="constructor")||(kind=="destructor")) {
    memberlist->appendItem(all,NULL,NULL,dict);
  } else {
    if (kind=="member") {
      FXint i=0;
      while (
        (i<otherlist->getNumItems())
        && (otherlist->getItemText(i).section('\t',1)=="member")
      ) { i++; }
      otherlist->insertItem(i,all,NULL,NULL,dict);
    } else {
      otherlist->appendItem(all,NULL,NULL,dict);
    }
  }
  dict->replace("kind",kind.text());
  return true;
}



bool TagParser::ParseOtherTag(FXStringDict *dict)
{
  if (Key("class").empty() && Key("struct").empty()) {
    const FXString kind=Key("kind");
    if ((kind!="class")&&(kind!="namespace")&&(kind!="macro")) {
      FXString name=Key("name").text();
      GetTypeInfo(dict,name,kind,contents);
      if ((Key("kind")=="prototype")) {
        dict->replace("kind","function");
      }
      FXString all=FXString::null;
      if (name.find("operator ")==0) {
        name.erase(0,9);
        name.append(" ");
        dict->replace("kind","operator");
      }
      all.format( "%s\t%s\t%s\t%s%s",
        Key("access").text(),
        Key("kind").text(),
        Key("type").text(),
        name.text(),
        Key("signature").text()
      );
      if ((Key("kind")=="function")||(Key("kind")=="operator")) {
        memberlist->appendItem(all,NULL,NULL,dict);
      } else {
        otherlist->appendItem(all,NULL,NULL,dict);
      }
      return true;
    }
  }
  return false;
}


// When a class is selected in the browser, ctags parses the header containing the class.
// This routine is called by the CmdIO object, once for each line of output from ctags.
long TagParser::onParseSingleHeader(FXObject*o, FXSelector sel, void*p)
{
  FXString*s=(FXString*)p;
  if (FXSELTYPE(sel)==SEL_IO_EXCEPT) { return 1; }
  FXStringDict *dict=new FXStringDict();
  if (debug_ctags) { dict->insert("debug",s->text()); }
  dict->insert("name",s->section('\t',0).text());
  for (FXint i=3; i<=s->contains('\t'); i++) {
    const FXString sect=s->section('\t', i);
    if (sect.find(':')>0) {
      dict->insert(sect.section(':',0).text(),sect.section(':',1,1024).text());
    }
  }
  FXString thisclass=Key("class");
  if (thisclass.empty()) { thisclass=Key("struct"); }
  StripNamespace(thisclass);
  if ((thisclass==classtree->getCurrentItem()->getText())||wholefile) {
    if (ParseClassTag(dict,thisclass)) { return 1; }
  } else {
    if (ParseOtherTag(dict)) { return 1; }
  }
  delete dict;
  return 1;
}



void TagParser::ParseSingle(const FXString &filename, bool entire_file)
{
  CmdStr cmd("ctags --options=NONE -n -f-");
  cmd+="--c++-kinds=+p";
  cmd+="--c-kinds=+p";
  cmd+="--fields=+z+t+a+S+K+n+f-k ";
  cmd+=ctags_indentifiers;
  cmd+=filename.text();
  CmdIO cmdio(mainwin);
  wholefile=entire_file;
  memberlist->clearItems();
  otherlist->clearItems();
  cmdio.lines(cmd.text(),this,ID_READ_SINGLE_FILE_LINES);
}


