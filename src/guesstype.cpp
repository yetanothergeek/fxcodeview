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



//  The ctags program doesn't provide type information for variables, function returns, etc.
//  But it does tell us which line the declaration starts on, so we can *sometimes* grab the
//  type info from there. This approach seems to work fairly well on well-formatted headers
//  like FOX uses. But it is almost sure to fail for other headers, there are just too many
//  possible ways to write a declaration:
//    multiline qualifier lists, comments, macros, who knows...

#include <fx.h>
#include "tagger.h"
#include "guesstype.h"


static void RemoveStringsAndComments(FXString&line)
{ 
  line.simplify();
  line.substitute("\t", " ",true); // Convert tabs to spaces
  for (char*s=line.text();*s; s++) { // Convert strings and comments into whitespace
    switch (*s) {
      case '"': {
        *s=' ';
        for (char*p=s+1; *p; p++) {
          if (*p=='"') {
            *p=' ';
            s=p;
            break;
          } else if (*p=='\\') {
            *p=' ';
            p++;
          } else { *p=' ';  }
          s=p;
        }
        break;
      }
      case '/': {
        switch (s[1]) {
          case '/': { 
            for(char*p=s; *p; p++) { *p=' '; }
            break;
          }
          case '*': {
            char*p2=strstr(s,"*/");
            if (p2) {
              p2+=2;
            } else {
              p2=strchr(s, '\0');
            }
            for(char*p=s; p<=p2 && *p; p++) { *p=' '; }
            break;
          }
        }
        break;
      }
    }
  }
  line.simplify(); // now squeeze out the extra spaces
}



static void RemoveExtraSpace(FXString&line)
{
  FXchar oldbuf[3]={0,0,0};
  FXchar newbuf[2]={0,0};
  bool found=false;
  const FXchar *puncs="!#$%&()*+,-./:;<=>?@[]^{|}~";
  do {
    found=false;
    for (const FXchar *c=puncs; *c; c++) {
      newbuf[0]=*c;
      oldbuf[0]=*c;
      oldbuf[1]=' ';
      while (line.find(oldbuf)>=0) {
        found=true;
        line.substitute(oldbuf,newbuf);
      }
      oldbuf[0]=' ';
      oldbuf[1]=*c;
      while (line.find(oldbuf)>=0) {
        found=true;
        line.substitute(oldbuf,newbuf);
      }
    }
  } while (found);
}


#define IsWordChar(c) ( (c=='_') || Ascii::isAlphaNumeric(c) )

static void StripQualifiers(FXString&line, const FXString &name, const FXString &kind)
{
  RemoveStringsAndComments(line);
  RemoveExtraSpace(line);
  if (line.find('#')==0) {
    line=line.section(' ', 0);
    return;
  }
  FXint sigstart=-1;
  if (name.find("operator ")==0) {
    FXString op=name;
    op.substitute(" ","");
    sigstart=line.find(op);
    if (sigstart==0) {
      line=FXString::null;
      return;
    } else if ((sigstart>0)&&!IsWordChar(line[sigstart-1])) {
      line.trunc(sigstart);
    }
  } else {
    sigstart=line.find("::"+name+"(");
    if (sigstart>=0) {
      if (sigstart>0) { sigstart--; }
      while ( (sigstart>0) && IsWordChar(line[sigstart]) ) { sigstart--; }
      line.trunc(sigstart+1);
    } else {
      sigstart=line.find(name+"(");
      if (sigstart>=0) {
        line.trunc(sigstart);
      } else {
        sigstart=line.find(name+";");
        if ((sigstart>=1)&&(!IsWordChar(line[sigstart-1]))) {
          line.trunc(sigstart);
        }
      }
    }
    if (line.find("template<")==0) {
      FXint gt=line.find('>');
      if (gt>=0) {
        line.erase(0,gt+1);
      }
    }
  }
  bool found;
  do { // strip off qualifiers from beginning of line
    found=false;
    const char* qualifs[] = {
      "struct ",
      "typedef ",
      "virtual ",
      "static ",
      "const ",
      "friend ",
      "inline ", 
      "extern \"C++\" ",
      "extern \"C\" ",
      "extern ",
      "__const ",
      "__extern_always_inline ",
      "__extension__ ",
      "FXAPI ",
      "XMLPUBFUN ",
      NULL };
    for (const char**qualif=(kind=="struct")?qualifs:qualifs+1; *qualif; qualif++) {
      int len=strlen(*qualif);
      if (compare(line,*qualif,len)==0) {
        line.erase(0,len);
        found=(line.find(' ')>=0);
      }
    }
  } while (found);
  line=line.trim();
  for (FXint i=0; i<=line.contains(' '); i++) {
    const FXString sect=line.section(' ', i);
    if (sect.find(';')>=0) {
      line=line.section(' ',0,i);
      break;
    }
  }
  FXint functype=line.find(")(");

  if ((functype>=0)&&(line.find('(')<functype)) {
    line.trunc(functype+1);
  }
  if (line.find(' ')>=0) { // Maybe something at start that looks like a macro?
    const FXString sect=line.section(' ',0);
    const char* heads[] = {
      "API",
      NULL
    };
    for (const char**t=heads; *t; t++) {
      if (sect.rfind(*t)==sect.length()-strlen(*t)) {
        line.erase(0,line.find(' '));
        break;
      }
    }
  }
  line.substitute("*", "* ");
  line.trimEnd();
  if (line.find(' ')>=0) { // Maybe something at end that looks like a macro?
    const FXString sect=line.section(' ',line.contains(' '));
    const char* tails[] = {
     "XMLCALL",
      NULL
    };
    for (const char**t=tails; *t; t++) {
      if (sect.rfind(*t)==sect.length()-strlen(*t)) {
        line.trunc(line.rfind(' '));
        break;
      }
    }
  }


}



void GetTypeInfo(FXStringDictionary *dict, const FXString &name, const FXString &kind, const FXString &contents)
{
  if (kind=="enumerator") {
    FXString type=Key("enum");
    TagParser::StripNamespace(type);
    dict->insert("type",type.text());
    return;
  }
  if ((kind=="typedef")) { 
    const FXString typeref=Key("typeref");
    if (!typeref.empty()) {
      dict->insert("type",typeref.section(':', 0).text());
      return;
    }
  }
  const FXString line=Key("line");
  if (!(line.empty())) {
    FXbool ok=false;
    FXint linenum=line.toInt(10, &ok);
    if (ok) {
      FXString type=contents.section('\n',linenum-1);
      StripQualifiers(type,name,kind);
      if ((linenum>1)&&type.empty()) { // We didn't get anything yet, try the line above
        type=contents.section('\n',linenum-2);
        StripQualifiers(type,name,kind);
      }
      if ((type.length()>0)&&(type[type.length()-1]=='{')) {
        type.trunc(type.length()-1);
      }
      if (type.find(name+"(")!=0) {
        dict->insert("type",type.text());
      }
    }
  }
}


