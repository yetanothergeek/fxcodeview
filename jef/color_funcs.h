/*
  FXiTe - The Free eXtensIble Text Editor
  Copyright (c) 2009-2014 Jeffrey Pohlmeyer <yetanothergeek@gmail.com>

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

#ifndef FXITE_COLOR_FUNCS_H
#define FXITE_COLOR_FUNCS_H


typedef char ColorName[8];

class ColorFuncs {
public:
  static void RgbToHex(FXColor rgb, ColorName &clr);
  static long HexToRGB(const char* rgb);
  static void InvertColors(bool inverted);
  static bool ColorsInverted();
};

#endif

