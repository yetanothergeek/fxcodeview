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

#include "ctokens.h"
#include "styler.h"


static FXColor fgCODE    = FXRGB(0x00,0x00,0x00);
static FXColor fgOPERAT  = FXRGB(0x00,0x00,0x00);
static FXColor fgKWORD   = FXRGB(0x00,0x00,0xB0);
static FXColor fgMLCOMM  = FXRGB(0xC0,0x00,0x00);
static FXColor fgSLCOMM  = FXRGB(0xC0,0x00,0x00);
static FXColor fgCHAR    = FXRGB(0x00,0xC0,0x00);
static FXColor fgSTRING  = FXRGB(0x00,0xC0,0x00);
static FXColor fgNUMBER  = FXRGB(0x00,0x00,0xE0);
static FXColor fgPREPROC = FXRGB(0xB0,0x00,0xC0);
static FXColor bgActive  = FXRGB(255,255,192);
static FXColor fgHilite  = FXRGB(0x00,0x00,0x00);
static FXColor bgHilite  = FXRGB(0xFF,0xFF,0xFF);



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
  Styles[synNAME].normalForeColor    = fgCODE;
  Styles[synSYMBOL].normalForeColor  = fgCODE;
  Styles[synKWORD].normalForeColor   = fgKWORD;
  Styles[synMLCOMM].normalForeColor  = fgMLCOMM;
  Styles[synSLCOMM].normalForeColor  = fgSLCOMM;
  Styles[synCHAR].normalForeColor    = fgCHAR;
  Styles[synSTRING].normalForeColor  = fgSTRING;
  Styles[synNUMBER].normalForeColor  = fgNUMBER;
  Styles[synPREPROC].normalForeColor = fgPREPROC;
  Styles[synSYMBOL].style = FXText::STYLE_BOLD;
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
static const char* CppKeywords[] = {
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




static int token_callback(const TokenInfo*info)
{
  FXuint offset=info->token-info->src;
  char*dst=(char*)info->user_data;
  memset(dst+offset,info->type,info->len);
  return 1;
}



void Styler::ParseContents(FXText *viewer, FXString &hilite, const FXString &source)
{
  hilite.length(source.length());
  memset(hilite.text(),0,source.length());
  tokenize(source.text(),token_callback,(void*)hilite.text(),CppKeywords,1);
  viewer->setText(source);
  viewer->setStyled(true);
  viewer->setHiliteStyles(Styles+1);
  viewer->changeStyle(0,hilite);
  viewer->setActiveBackColor(bgActive);
}

