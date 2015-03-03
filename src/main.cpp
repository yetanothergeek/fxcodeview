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


#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fx.h>

#include "interproc.h"
#include "tagger.h"
#include "fxcv.h"
#include "theme.h"



static void Help(FXApp*app) {
  const char*name=FXPath::name(app->getArgv()[0]).text();
  fxmessage(
    "Usage:\n"
    "\n"
    " Server startup:\n"
    "   %s  [some-directory]  [another-directory ...]  [class-name]\n"
    "\n"
    " Client request:\n"
    "   %s  [class-name]\n"
    "\n"
    "Directories and class-name may be given in any order, but directory\n"
    "paths must contain at least one slash %c\n"
    "\n"
    "The environment variable $FXCV_EDITOR can be used for the \"open\" command, \n"
    "where %%s is a placeholder for the filename, %%d for the line number.\n"
    "\n"
    "For example:\n"
    "  export FXCV_EDITOR=\"myeditor.exe --file=%%s --line=%%d\"\n"
    "  %s /usr/include/fox-1.7 /usr/src/fox-1.7.26/lib FXString\n\n"
    "\n"
    "The environment variables $XDG_RUNTIME_DIR and $FXCV_SOCKET can be\n"
    "used to create and/or control more than one instance of %s.\n\n"
    ,
    name,
    name,
    '/',
    name,
    name
    );
    ::exit(0);
}



static bool IsValidIdentifier(const char *txt)
{
  if ((txt==NULL)||(*txt=='\0')) {
    fxmessage("Empty text!!!\n");
    return false;
  }
  if (!(Ascii::isLetter(txt[0])||(txt[0]=='_'))) {
    fxmessage("Invalid char: #%d at position 0.\n",txt[0]);
    return false;
  }
  for (const FXchar *c=txt+1;  *c; c++) {
    if (!(Ascii::isAlphaNumeric(*c)||(*c=='_'))) {
    fxmessage("Invalid char #%d at position %d.\n",*c, c-txt);
      return false;
    }
  }
  return true;
}



#define FATAL(msg,...) \
  app->create(); \
  FXMessageBox::error(app, MBOX_OK, \
    FXPath::name(app->getArgv()[0]).text(), msg, __VA_ARGS__ ); \
  ::exit(1);

#define DIR_FATAL(msg,e) \
  FATAL("Could not %s directory:\n  %s\n(%s)\n", \
  msg, arg.text(), strerror(e))


static void ParseCommandLine(FXApp*app) {
  for (FXint i=1; i<app->getArgc(); i++) {
    const FXString arg=app->getArgv()[i];
    if ((arg[0]=='-')) {
      Help(app);
    } else if (arg.find('/')>=0) {
      struct stat st;
      errno=0;
      if (stat(arg.text(),&st)!=0) {    DIR_FATAL("find",   errno); }
      if (!S_ISDIR(st.st_mode)) {       DIR_FATAL("open",   ENOTDIR); }
      if (access(arg.text(),R_OK)!=0) { DIR_FATAL("read",   EACCES); }
      if (access(arg.text(),X_OK)!=0) { DIR_FATAL("browse", EACCES); }
      if (TagParser::DirList().no()+1>=MAX_LOCATION_INDEX) {
        FATAL("Too many search paths! (Max:%d)",MAX_LOCATION_INDEX);
      }
      TagParser::DirList().append(FXPath::simplify(FXPath::absolute(arg)));
    } else {
      if (MainWin::InitialClass().empty()) {
        if (IsValidIdentifier(arg.text())) {
          MainWin::InitialClass(arg);
        } else {
          FATAL("Invalid identifier: \"%s\"", arg.text());
        }
      } else {
       FATAL(
         "Only one class name is permitted from command line.\n\n"
         "Got:  \"%s\"  and  \"%s\"",
          MainWin::InitialClass().text(), arg.text()
        );
      }
    }
  }
}


extern "C" int ini_sort(const char *filename);


int main(int argc, char *argv[])
{
  FXApp a(APP_NAME,FXString::null);
  a.init(argc,argv);
  FXApp*app=&a;
  ParseCommandLine(app);
  FXString sockdir=FXSystem::getEnvironment("XDG_RUNTIME_DIR");
  if (sockdir.empty()) { sockdir=FXSystem::getHomeDirectory().text(); }
  sockdir+='/';
  FXString sockname=FXSystem::getEnvironment("FXCV_SOCKET");
  if (sockname.empty()) { sockname=".fxcv.uds"; }
  sockname.prepend(sockdir);
  InterProc ipc(app, sockname);
  if (ipc.ClientSend(NULL, MainWin::InitialClass())) {
    if (TagParser::DirList().no()) {
      FATAL("%s\n%s\n%s",
      "Already running.",
      "Cannot adjust search path from command line,",
      "please use the \"setup\" button instead."
      );
    }
  } else {
    Theme::init();
    MainWin::Splash(&a);
    if (!TagParser::DirList().no()) {
      TagParser::SetDefaultSearchPaths();
    }
    MainWin* win=new MainWin(app);
    app->create();
    ipc.StartServer(win, win, MainWin::ID_IPC_EXEC);
    int rv=app->run();
    ipc.StopServer();
    FXString cfgfile=a.reg().getUserDirectory();
    if (cfgfile=="$XDG_CONFIG_HOME") {
      FXString xdgch=FXSystem::getEnvironment(cfgfile);
      cfgfile=xdgch.empty()?FXSystem::getHomeDirectory()+PATHSEPSTRING+".config":xdgch;
    }
    cfgfile+=PATHSEPSTRING+FXString(APP_NAME)+".rc";
    Theme::done();
    ini_sort(cfgfile.text());
    return rv;
  }
}

