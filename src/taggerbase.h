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


typedef FXushort LocationIndex;
#define MAX_LOCATION_INDEX 65535


#define ctags_languages "--languages=c,c++"

#define ctags_indentifiers "-I \
__NTH+,\
__REDIRECT+,\
__REDIRECT_NTH+,\
__THROW,\
__asm,\
__attribute_pure__,\
__const=const,\
__nonnull+,\
__restrict,\
__wur,\
XMLPUBFUN,\
XMLCALL\
"


#define ctags_kinds  "+c+d+e+f+g+l+m+n+p+s+t+u+v+x"

#define ctags_fields "--fields=+a+i+k+f+s+t-l-n+t-K-z"


typedef struct {
  FXchar kind;
  LocationIndex idir;
  LocationIndex ifile;
  FXint line;
} Location;


// The list of tags for a given file
class TagList: public FXObjectListOf<FXString> { 
  FXString _filename;
  FXTime modified;
public:
  TagList(const FXString &file):FXObjectListOf<FXString>(),
    _filename(file), modified(FXStat::modified(file)) { }
  virtual ~TagList() { clear(); }
  const FXString &filename() const { return _filename; }
  FXTime stamp() { return modified; }
  void stamp(FXTime t)  { modified=t; }
  FXbool append(FXString*s) { return FXObjectList::append((FXObject*)s); }
  void clear() {
    for (FXint i=0; i<no(); i++) {
      FXString*s=at(i);
      if (s) { delete s; }
    }
    FXObjectList::clear();
  }
};


// The list of source files in a given directory
class FileList: public FXObjectListOf<TagList> {
  FXString _dirname;
  FXTime modified;
public:
  FileList(const FXString &dir):FXObjectListOf<TagList>(),
    _dirname(dir), modified(FXStat::modified(dir)){ }
  const FXString &dirname() const { return _dirname; }
  FXTime stamp() { return modified; }
  void stamp(FXTime t)  { modified=t; }
  virtual ~FileList() { clear(); }
  FXbool append(const FXString &file) {
    if (no()+1>=MAX_LOCATION_INDEX) {
      fxerror("***FATAL: Maximum %d files per directory.\n", MAX_LOCATION_INDEX);
    }
    return FXObjectList::append((FXObject*)(new TagList(file)));
  }
  void clear() {
    for (FXint i=0; i<no(); i++) { 
      delete (TagList*)at(i);
    }
    FXObjectList::clear();
  }
  TagList*tail() { return no()?at(no()-1):NULL; }
};



// The list of all indexed directories
class DirectoryList: public FXObjectListOf<FileList> {
public:
  FXbool append(const FXString &dirname) {
    for (FXint i=0; i<no(); i++) {
      if (FXFile::identical(at(i)->dirname(),dirname)) { return true; }
    }
    if (no()+1>=MAX_LOCATION_INDEX) {
      fxerror("***FATAL: Maximum %d directories.\n", MAX_LOCATION_INDEX);
    }
    return FXObjectList::append((FXObject*)(new FileList(dirname)));
  }
  void clear() {
    for (FXint i=0; i<no(); i++) {
      delete (FileList*)at(i);
    }
    FXObjectList::clear();
  }
  virtual ~DirectoryList() { clear(); }
};



// Automatically appends a space between subsequent append (+=) operations
// (Because I keep forgetting to do it myself!)
class CmdStr: public FXString {
public:
  FXString& operator+=(const FXchar* s) {
    append(s);
    return append(' ');
  }
  CmdStr(const FXchar*s):FXString(s) { 
    if (length()) { append(' '); }
  }
};



class TagInfoList;



class TagParserBase: public FXObject {
private:
  FXDECLARE(TagParserBase)
  FXList*classlist;
  TagInfoList*tag_info_list;
  FXString current_filename;
  FXint ExtractLineNumber(const FXString* &s);
  void ReadClasses();
  void PopulateClassTree();
  void OrganizeClassTree();
  void OptimizeClassTree();
  void TransformItemData(FXTreeItem *item);
  void PurgeNamespaces();
protected:
  TagParserBase() {}
  FXMainWindow *mainwin;
  FXTreeList*classtree;
public:
  long onParseAllSources(FXObject*o, FXSelector sel, void*p);
  static void SetDefaultSearchPaths();
  static FXint StripNamespace(FXString &s);
  static const FXString SafeKey(FXStringDict*dict, const char*key);
  void CreateTreeItems();
  void LocationToFullPath(FXString &file, Location*locn);
  const FXString &LocationToFilename(Location*locn);
  TagParserBase(FXMainWindow*win, FXTreeList*clstree, FXList*clslist);
  enum {
    ID_READ_ALL_FILES_LINES=1,
    ID_LAST
  };
  static DirectoryList &DirList();
  virtual ~TagParserBase();
};

// Note that the "dict" here must be named "dict".
#define Key(k) TagParserBase::SafeKey(dict,k)


#define debug_ctags true

