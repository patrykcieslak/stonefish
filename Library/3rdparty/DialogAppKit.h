/***************************************************************
 
 GLUI 2 - OpenGL User Interface Library 2
 Copyright 2011 Core S2 - See License.txt for details
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 2013 Modified by Patryk Cieslak
 
 File: DialogAppKit.mm/h
 Desc: Objective-C specific App Kit based API callbacks. It
 is necessary to split this code into a separate h/m pair
 to access the OS-level (in ObjC) open/save panels.
 
 Note that the *.mm extension to the implementation file is
 critical as it tells the compiler it is a mix of C, C++,
 and Objective-C (so we can have C-style global functions yet
 includes Objective-C AppKit function calls).
 
 OutBuffer will be written to as much as possible.
 
 Results are based on true: success, false: failure
 
***************************************************************/

// All of this should only compile if we are in OSX
#ifdef __APPLE__

// Inclusion guard
#ifndef __G2APPKIT_H__
#define __G2APPKIT_H__

// Open a dialog
bool __ShowDialog(const char* Message);

// Open a save-file dialog
bool __ShowSaveDialog(const char* Message, const char* FileExtension, char* OutBuffer, int OutLength);

// Open an open-file dialog
bool __ShowOpenDialog(const char* Message, const char* FileExtension, char* OutBuffer, int OutLength);

// End of inclusion guard
#endif

// End of apple guard
#endif
