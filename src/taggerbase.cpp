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
#include "taggerbase.h"

#include "runcmd.h"


class TagInfo: public FXObject {
public:
  FXString classname;
  FXString ancestor;
  Location *location;
  TagInfo(FXchar knd, const FXString &cls, const FXString &anc, LocationIndex idir, LocationIndex ifile, FXint lin):FXObject() {
    classname=cls;
    ancestor=anc;
    location=new Location;
    location->kind=knd;
    location->ifile=ifile;
    location->line=lin;
    location->idir=idir;
  }
};

#define SameLocation(a,b) ((a->idir==b->idir)&&(a->ifile==b->ifile))

class TagInfoList: public FXObjectListOf<TagInfo> {
public:
  void append(FXchar knd, const FXString &cls, const FXString &anc, LocationIndex idir, LocationIndex ifile, FXint lin) {
    FXObjectList::append(new TagInfo(knd,cls,anc,idir,ifile,lin));
  }
  void clear() {
    for (FXint i=0; i<no(); i++) { delete at(i); }
    FXObjectList::clear();
  }
  virtual ~TagInfoList() {
    clear();
  }
};



FXDEFMAP(TagParserBase) TagParserBaseMap[]={
  FXMAPFUNC(SEL_IO_WRITE,  TagParserBase::ID_READ_ALL_FILES_LINES, TagParserBase::onParseAllSources),
  FXMAPFUNC(SEL_IO_EXCEPT, TagParserBase::ID_READ_ALL_FILES_LINES, TagParserBase::onParseAllSources),
};

FXIMPLEMENT(TagParserBase,FXObject,TagParserBaseMap,ARRAYNUMBER(TagParserBaseMap))


static DirectoryList dirs;


TagParserBase::TagParserBase( FXMainWindow*win, FXTreeList*clstree, FXList*clslist): FXObject()
{
  mainwin=win;
  classtree=clstree;
  classlist=clslist;
}


//  Try to read a key from dict, return the result as FXString (or empty)
const FXString TagParserBase::SafeKey(FXStringDictionary*dict, const char*key)
{
  return dict->at(key);
}



FXint TagParserBase::ExtractLineNumber(const FXString* &s)
{
  FXbool ok=false;
  FXString linenum=s->section('\t',2).substitute(";\"","");
  FXint rv=linenum.toInt(10,&ok);
  return ok?rv:-1;
}



// When BuildClassTree() is called, ctags parses all source files in the directory list.
// This routine is called by the CmdIO object, once for each line of output from ctags.
long TagParserBase::onParseAllSources(FXObject*o, FXSelector sel, void*p)
{
  const FXString* s=(const FXString*)p;
  if (FXSELTYPE(sel)==SEL_IO_EXCEPT) { return 1; }
  LocationIndex dirindex=(LocationIndex)((FXuval)((CmdIO*)o)->getUserData());
  FXString ancestor=FXString::null;
  FXString scopename=FXString::null;
  FXint linenum=ExtractLineNumber(s);
  FXchar kind=s->section('\t', 3)[0];
  FXString tagname=s->section('\t',0);
  FXString filename=s->section('\t',1);
  if (current_filename!=filename) {
    current_filename=filename;
    dirs[dirindex]->append(filename);
  }
  const FXchar ext=Ascii::toLower(FXPath::extension(filename)[0]);

  if ( (kind=='n') ) {
    if (ext=='c') { return 1; } // namespaces are only interesting if they are in a public header
  }
  for (FXint i=4; i<=s->contains('\t'); i++) {
    const FXString sect=s->section('\t', i);
    if (sect.find(':')>0) {
      const FXString key=sect.section(':',0);
      if ((kind=='s')&&(key=="access")) {  // ignore structs defined inside a class
        return 1;
      } else if ((kind=='f')&&((key=="class")||(key=="namespace"))) {
        scopename=sect.section(':',1,1024);
        StripNamespace(scopename);
      } else if (key=="inherits") {
        ancestor=sect.section(':',1);
        if (ancestor.find(',')>0) {
          ancestor=ancestor.section(',',0);  // can't do multiple inheritence
        }
        StripNamespace(ancestor);
      }
    }
  }
  switch (kind) {
    case 'c':
    case 'n':    
    case 's': {
      tag_info_list->append(kind, tagname, ancestor, dirindex, dirs[dirindex]->no()-1, linenum);
      break;
    }
    default: {
      FXString* entry = new FXString();
      entry->format(
        "%c\t%s%s%s\t%d",
        kind,
        scopename.text(),
        scopename.empty()?"":"::",
        tagname.text(),
        linenum
      );
      dirs[dirindex]->tail()->append(entry);
      break;
    }
  }
  return 1;
}



DirectoryList &TagParserBase::DirList() { return dirs; }


static const CmdStr &CtagsCmd() {
  static CmdStr cmd=FXString::null;
  if (cmd.empty()) {
    cmd="ctags --options=NONE -n -f-";
    cmd+=ctags_fields;
    cmd+=ctags_indentifiers;
    cmd+=ctags_languages;
    cmd+="--c++-kinds=" ctags_kinds;
    cmd+="--c-kinds=" ctags_kinds;
    cmd+="--file-scope=no";
    cmd+="--sort=no"; // <= We must process tags on a per-file basis - Do not sort!  
  }
  return cmd;
}

// Run ctags in each source directory
void TagParserBase::ReadClasses()
{
  CmdIO cmdio(mainwin);
  CmdStr cmd=CtagsCmd();
  FXString currdir=FXSystem::getCurrentDirectory();
  FXRex rx("\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$",FXRex::IgnoreCase);
  for (FXint i=0; i<DirList().no(); i++) {
    const FXString dir=DirList().at(i)->dirname();
    if (dir.empty()) { continue; }
    if (FXSystem::setCurrentDirectory(dir)) {
      FXDir de(dir);
      if (de.isOpen()) {
        FXString fn;
        while (de.next(fn)) {
          if (FXStat::isFile(fn) && rx.match(fn)) { cmd+=fn.text(); }
        }
        de.close();
        current_filename=FXString::null;
        cmdio.setUserData((void*)(FXival)i);
        cmdio.lines(cmd.text(),this,ID_READ_ALL_FILES_LINES);
      }
    }
  }
  FXSystem::setCurrentDirectory(currdir);
}



// Remove anything containing a colon : from the beginning of the string.
// Returns number of colons removed.
FXint TagParserBase::StripNamespace(FXString &s)
{
  FXint rv=s.contains(':');
  if (rv) {
    s.erase(0,s.rfind(':')+1);
  }
  return rv;
}



// After we have our tree created, we can get rid of the old classname/ancestor
// list and just store the filename and linenumber for each entry.
void TagParserBase::TransformItemData(FXTreeItem *item)
{
  if (item) {
    for (FXTreeItem*i=item; i; i=i->getNext()) {
      TagInfo*ti=(TagInfo*)i->getData();
      if (ti) {
        i->setData(ti->location);
        classlist->appendItem(i->getText(), NULL, ti->location);
        ti->location=NULL;
      }
      TransformItemData(i->getFirst()); // Recursive!
    }
  }
}



// If we end up with two namespace entries that both refer to the same file,
// how do we know which one to keep? Wild guess: Maintain a count of how 
// often each namespace occurs, then compare the counts, and keep the entry
// that looks more specific, e.g. for FX::Ascii, we have lots more FX than 
// Ascii, so Ascii wins. Something like an anti-popularity contest.
// But actually we should probably maintain a heirarchy of namespaces
// instead of this kludge.
class NumDict:public FXDictionary {
public:
  NumDict():FXDictionary(){}
  void inc(const FXchar*k) {
    FXint v=find(k);
    at(k)=(void*)(v+1);
  }
  FXint find(const FXchar*k) {
    return (FXint)((FXuval)FXDictionary::find(k));
  }
};



// The only namespace entries we want to keep are those that refer to
// header files that would not be included otherwise. So if the header
// file also contains some class information that is referenced in 
// another entry, we should be able to remove the namespace-only entry.
void TagParserBase::PurgeNamespaces()
{
  NumDict nd;
  for (FXint i=0; i<classlist->getNumItems(); i++) {
    Location*iloc=(Location*)(classlist->getItemData(i));
    if (iloc->kind=='n') {                                // <= If it is a namespace,
      for (FXint j=0; j<classlist->getNumItems(); j++) {  // <= Look for entries with same filename.
        if (j!=i) {                                       // <= If it is not the same entry.
          Location*jloc=(Location*)(classlist->getItemData(j));
          if (SameLocation(iloc,jloc)) {                  // <= If it refers to the same file.
            if (jloc->kind!='n') {                        // <= If it is not a namespace.
              FXTreeItem*item=classtree->findItemByData(iloc); // <= Look it up in the class tree.
              if (item && !item->getFirst()) {            // If it has no children.
                iloc->kind='!';                           // Mark it for deletion from the list.
                classtree->removeItem(item);              // Remove it from the tree.
                break;
              }
            } else { // Two namespaces refer to same file, flag both with uppercase 'N'
              nd.inc(classlist->getItemText(i).text());
              iloc->kind='N';
              nd.inc(classlist->getItemText(j).text());
              jloc->kind='N';
            }
          }
        }
      }
    }
  }
  for (FXint i=0; i<classlist->getNumItems(); i++) {  // See comments for "NumDict" class above
    Location*iloc=(Location*)(classlist->getItemData(i));
      if (iloc->kind=='N') { 
      for (FXint j=0; j<classlist->getNumItems(); j++) { 
        if (j!=i) {
          Location*jloc=(Location*)(classlist->getItemData(j));
          if ( (jloc->kind=='N') && SameLocation(iloc,jloc) ) {
            FXint icnt=nd.find(classlist->getItemText(i).text());
            FXint jcnt=nd.find(classlist->getItemText(j).text());
            iloc->kind='n';
            jloc->kind='n';
            Location*xloc=(icnt>=jcnt)?iloc:jloc; // xloc is the more common of the two
            FXTreeItem*item=classtree->findItemByData(xloc);
            if (item && !item->getFirst()) {
              xloc->kind='!'; // Mark it for deletion from the list
              classtree->removeItem(item); // Remove it from the tree.
              break;
            }
          }
        }
      }
    }
  }
  for (FXint i=classlist->getNumItems()-1; i>=0; i--) {
    Location*iloc=(Location*)(classlist->getItemData(i));
    switch (iloc->kind) {
      case '!': { // <= remove items that were marked for deletion
        delete iloc;
        classlist->removeItem(i);
        break;
      }
      case 'n': { // <= The @ notation indicates that this is a namespace, not a class name
        FXString name=classlist->getItemText(i)+"@"+LocationToFilename(iloc);
        classlist->setItemText(i,name);
        FXTreeItem*item=classtree->findItemByData(iloc);
        item->setText(name);
        break;
      }
    }
  }
}



// Add all class tags to the class tree. At this point the list is 
// still "flat" but adding them to the tree now gives us access to 
// parent/child pointers that we can fill in later.
void TagParserBase::PopulateClassTree()
{
  FXTreeItem*tmp=new FXTreeItem("",NULL,NULL,NULL);
  classtree->appendItem(NULL,tmp);
  for (FXint i=0; i<tag_info_list->no(); i++) {
    TagInfo*ti=tag_info_list->at(i);
    tmp=new FXTreeItem(ti->classname,NULL,NULL,(void*)ti);
    tmp->setExpanded(true);
    classtree->appendItem(NULL,tmp);
  }
}



// This loops through the flat list of classname/ancestor pairs and
// builds a heirarchy of classes
void TagParserBase::OrganizeClassTree()
{
  bool done=false;
  while (!done) {
    done=true;
    for (FXTreeItem*i=classtree->getFirstItem(); i; i=i->getNext()) {
      TagInfo*child_tag=(TagInfo*)i->getData();
      if (child_tag && !child_tag->ancestor.empty()) {
        done=false;
        bool failed=false;
        for (FXint j=0; j<tag_info_list->no(); j++) {
          TagInfo*parent_tag=tag_info_list->at(j);
          failed=true;
          if (child_tag->ancestor==parent_tag->classname) {
            FXTreeItem *parent_item=classtree->findItemByData((void*)parent_tag);
            classtree->moveItem(NULL,parent_item,i);
            failed=false;
            break;
          }
        }
        if (failed) { // Couldn't find ancestor? Play like there isn't one!
          child_tag->ancestor=FXString::null;
        }
        break;
      }
    }
  }
}



// Cleans up some unwanted information from the class tree,
// and sets all tree items to "expanded" state.
void TagParserBase::OptimizeClassTree()
{
  FXTreeItem *item=classtree->getFirstItem();
  if (item) {
    TransformItemData(item);
    classlist->sortItems();
    PurgeNamespaces();
    item=item->getFirst();
    if (item) {
      classtree->expandTree(item);
      classtree->selectItem(item,true);
      classtree->setCurrentItem(item,true);
    }
  }
}



void TagParserBase::CreateTreeItems()
{
  classtree->clearItems();
  classlist->clearItems();
  tag_info_list=new TagInfoList();
  ReadClasses();
  PopulateClassTree();
  OrganizeClassTree();
  OptimizeClassTree();
  delete tag_info_list;
  tag_info_list=NULL;
}



void TagParserBase::SetDefaultSearchPaths()
{
  DirList().clear();
  for (char v='9'; (v>='6') && !DirList().no(); v--) {
    const char*srch[]={
      "/usr/local/include/fox-1.?",
      "/usr/include/fox-1.?",
      NULL
    };
    for (const char**path=srch; *path; path++) {
      char buf[255];
      strncpy(buf,*path,sizeof(buf)-1);
      char*q=strchr(buf,'?');
      *q=v;
      if (FXStat::isDirectory(buf)) {
        DirList().append(buf);
        break;
      }
    }
  }
}



const FXString& TagParserBase::LocationToFilename(Location*locn)
{
  FileList *files=DirList()[locn->idir];
  return files->at(locn->ifile)->filename();
}



void TagParserBase::LocationToFullPath(FXString &file, Location*locn)
{
  FileList *files=DirList()[locn->idir];
  file=files->dirname()+PATHSEPSTRING;
  file+=files->at(locn->ifile)->filename();
}



TagParserBase::~TagParserBase()
{
  for (FXint idir=0; idir<DirList().no(); idir++) {
    FXString key;
    FileList *filelist=DirList()[idir];
    mainwin->getApp()->reg().writeStringEntry("Paths",key.fromInt(idir).text(),filelist->dirname().text());
//    fxmessage("%s\n", filelist->dirname().text());
//    for (FXint ifile=0; ifile<filelist->no(); ifile++) {
//      TagList* taglist=filelist->at(ifile);
//      fxmessage("%s\t%lld\n", taglist->filename().text(), taglist->stamp()/1000000000);
//      for (FXint itag=0; itag<taglist->no(); itag++) {
//        fxmessage("\t%s\n", taglist->at(itag)->text());
//      }
//    }
  }
  DirList().clear();
}  


