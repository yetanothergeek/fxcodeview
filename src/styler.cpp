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
#include <fxkeys.h>
#include "styler.h"


enum {
  synNULL=0,   // Reserved, must be first
  synCODE,     // Default text
  synKWORD,    // Keywords
  synMLCOMM,   // Multi-line comments
  synSLCOMM,   // Single-line comments
  synCHAR,     // Character constants
  synSTRING,   // Quoted strings
  synOPERAT,   // Operators
  synNUMBER,   // Numbers
  synSPACE,    // White space
  synPREPROC,  // Preprocessor directives
  synEOL,      // New-line character
  synLAST      // Reserved, must be last
};


FXColor fgCODE    = FXRGB(0x00,0x00,0x00);
FXColor fgOPERAT  = FXRGB(0x00,0x00,0x00);
FXColor fgKWORD   = FXRGB(0x00,0x00,0xB0);
FXColor fgMLCOMM  = FXRGB(0xC0,0x00,0x00);
FXColor fgSLCOMM  = FXRGB(0xC0,0x00,0x00);
FXColor fgCHAR    = FXRGB(0x00,0xC0,0x00);
FXColor fgSTRING  = FXRGB(0x00,0xC0,0x00);
FXColor fgNUMBER  = FXRGB(0x00,0x00,0xE0);
FXColor fgPREPROC = FXRGB(0xB0,0x00,0xC0);
FXColor bgActive  = FXRGB(255,255,192);
FXColor fgHilite  = FXRGB(0x00,0x00,0x00);
FXColor bgHilite  = FXRGB(0xFF,0xFF,0xFF);



static FXColor InvertColor(FXColor rgb)
{
  long r,g,b;
  r=FXREDVAL(rgb);
  g=FXGREENVAL(rgb);
  b=FXBLUEVAL(rgb);
  r=255-r;
  g=255-g;
  b=255-b;
  if ((r>0)&&(r<0x80)) { r+=0x40; }
  if ((g>0)&&(g<0x80)) { g+=0x40; }
  if ((b>0)&&(b<0x80)) { b+=0x40; }
  return FXRGB(r,g,b);
}

static FXHiliteStyle Styles[synLAST];

void Styler::InitStyles()
{
  memset(&Styles, 0, sizeof(Styles));
  FXApp*app=FXApp::instance();
  if (app->getBackColor()<FXRGB(0x80,0x80,0x80)) {
    fgKWORD   = InvertColor(fgKWORD);
    fgMLCOMM  = InvertColor(fgMLCOMM);
    fgSLCOMM  = InvertColor(fgSLCOMM);
    fgCHAR    = InvertColor(fgCHAR);
    fgSTRING  = InvertColor(fgSTRING);
    fgNUMBER  = InvertColor(fgNUMBER);
    fgPREPROC = InvertColor(fgPREPROC);
    bgActive  = InvertColor(bgActive);
  }
  fgCODE=app->getForeColor();
  for (FXint i=0; i<synLAST; i++) {
    Styles[i].normalBackColor=app->getBackColor();
    Styles[i].selectForeColor=app->getSelforeColor();
    Styles[i].selectBackColor=app->getSelbackColor();
    Styles[i].activeBackColor=bgActive;
  }
  Styles[synCODE].normalForeColor    = fgCODE;
  Styles[synOPERAT].normalForeColor  = fgCODE;
  Styles[synKWORD].normalForeColor   = fgKWORD;
  Styles[synMLCOMM].normalForeColor  = fgMLCOMM;
  Styles[synSLCOMM].normalForeColor  = fgSLCOMM;
  Styles[synCHAR].normalForeColor    = fgCHAR;
  Styles[synSTRING].normalForeColor  = fgSTRING;
  Styles[synNUMBER].normalForeColor  = fgNUMBER;
  Styles[synPREPROC].normalForeColor = fgPREPROC;
  Styles[synOPERAT].style = FXText::STYLE_BOLD;
  Styles[synKWORD].style = FXText::STYLE_BOLD;
}


/*
Copied here for quick reference...
struct FXHiliteStyle {
  FXColor normalForeColor;            /// Normal text foreground color
  FXColor normalBackColor;            /// Normal text background color
  FXColor selectForeColor;            /// Selected text foreground color
  FXColor selectBackColor;            /// Selected text background color
  FXColor hiliteForeColor;            /// Highlight text foreground color
  FXColor hiliteBackColor;            /// Highlight text background color
  FXColor activeBackColor;            /// Active text background color
  FXuint  style;                      /// Highlight text style
  };
*/


// List of C++ keywords
static const char* Keywords[] = {
  "and",        "and_eq",   "asm",           "auto",      "bitand",    "bitor",
  "bool",       "break",    "case",          "catch",     "char",      "class",
  "compl",      "const",    "const_cast",    "continue",  "default",   "delete",
  "do",         "double",   "dynamic_cast",  "else",      "enum",      "explicit",
  "export",     "extern",   "false",         "float",     "for",       "friend",
  "goto",       "if",       "inline",        "int",       "long",      "mutable",
  "namespace",  "new",      "not",           "not_eq",    "operator",  "or",
  "or_eq",      "private",  "protected",     "public",    "register",  "reinterpret_cast",
  "return",     "short",    "signed",        "sizeof",    "static",    "static_cast",
  "struct",     "switch",   "template",      "this",      "throw",     "true",
  "try",        "typedef",  "typeid",        "typename",  "union",     "unsigned",
  "using",      "virtual",  "void",          "volatile",  "wchar_t",   "while",
  "xor",        "xor_eq",   NULL
};


// Create the highlighting "map" from the C++ file contents source
void Styler::ParseContents(FXText *viewer, FXString &hilite, const FXString &source)
{
  FXuint srclen=source.length();
  FXuint i=0;
  hilite.length(srclen);
  const char*src=source.text();
  char*dst=hilite.text();
  memset(dst,0,srclen);
  while (i<srclen) {
    switch (src[i]) {
      case '"': {
        for (const FXchar*p=src+i+1; p[0]; p++) {
          if (p[0]=='"') {
            dst[i]=synSTRING;
            dst[i+1]=synSTRING;
            i+=2;
            break;
          } else if ((p[0]=='\\')) {
            dst[i]=synSTRING;
            dst[i+1]=synSTRING;
            p++;
            i++;
          }
          dst[i]=synSTRING;
          i++;
        }
        break;
      }
      case '\'': {
        dst[i]=synCHAR;
        dst[i+1]=synCHAR;
        dst[i+2]=synCHAR;
        i+=3;
        break;
      }
      case '/':{
        if (src[i+1]=='/') {
          const FXchar*p=strchr(&src[i], '\n');
          if (!p) { p=strchr(&src[i], '\0'); } // Unterminated comment
          if (p) {
            FXint len=(p-&src[i]);
            memset(dst+i,synSLCOMM,len);
            i+=len;
          }
          break;
        } else if (src[i+1]=='*') {
          const FXchar*p=strstr(&src[i]+1, "*/");
          if (!p) { p=strchr(&src[i], '\0'); } // Unterminated comment
          if (p) {
            if (*p) { p+=2; }
            FXint len=(p-&src[i]);
            memset(dst+i,synMLCOMM,len);
            i+=len;
          }
          break;
        } else {
          dst[i]=synOPERAT;
          i++;
          break;
        }
      }
      case '\n': {
        dst[i]=synEOL;
        i++;
        break;
      }
      default: {
        if ((src[i]=='0')&&((src[i+1]=='x')||(src[i+1]=='X'))) {
          dst[i]=synNUMBER;
          dst[i+1]=synNUMBER;
          i+=2;
          while (Ascii::isHexDigit(src[i])) {
            dst[i]=synNUMBER;
            i++;
          }
        } else if (Ascii::isDigit(src[i])) {
          while (Ascii::isDigit(src[i])) {
            dst[i]=synNUMBER;
            i++;
          }
        } else if (Ascii::isSpace(src[i])) {
          dst[i]=synSPACE;
          i++;
        } else if (Ascii::isLetter(src[i])||(src[i]=='#')||(src[i]=='_')) {
          dst[i]=synCODE;
          i++;
          while (Ascii::isAlphaNumeric(src[i])||(src[i]=='_')) {
            dst[i]=synCODE;
            i++;
          }
        } else {
         dst[i]=synOPERAT;
         i++;
        }
      }
    }
  }
  FXchar*p=dst;
  while (p && *p) {
    while ((*p==synEOL)||(*p==synSPACE)) { p++; }
    if (src[p-dst]=='#') {
      *p=synPREPROC;
      p++;
      while (*p==synSPACE) { p++; }
      while (*p==synCODE) {
        *p=synPREPROC;
        p++;
      }
    }
    p=strchr(p,synEOL);
  }
  p=strchr(dst,synCODE);
  while (p) {
    FXint rem=srclen-(p-dst);
    for (const FXchar**k=Keywords; *k; k++) {
      FXint len=strlen(*k);
      if ((rem>=len)&&(p[len]!=synCODE)) {
        if (strncmp(*k,src+(p-dst),len)==0) {
          memset(p,synKWORD,len);
          p+=len;
          break;
        }
      }
    }
    while (*p==synCODE) { p++; }
    p=strchr(p,synCODE);
  }
  viewer->setText(source);
  viewer->setStyled(true);
  viewer->setHiliteStyles(Styles+1);
  viewer->changeStyle(0,hilite);
  viewer->setActiveBackColor(bgActive);
}

