/***************************************************************
 
 GLUI 2 - OpenGL User Interface Library 2
 Copyright 2011 Core S2 - See License.txt for details
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 2013 Modified by Patryk Cieslak
 
 File: NativeDialog.cpp/h
 Desc: A wrapper for the host operating-system's generic dialog
 boxes. For Windows we use "MessageBox(...)", for OSX we use
 "CreateStandardAlert(...)", but for Linux we just use the
 command line (because who knows which GUI they are on?). Note
 that we are opening a Modal that may or may not halt the
 calling thread (can't get unified behavior for now) and may
 or may not act as a modal vs. standard window.
 
***************************************************************/

// Inclusion guard
#ifndef __NATIVEDIALOG_H__
#define __NATIVEDIALOG_H__

#include "common.h"

// Maximum input/output/message length
static const int Dialog_MaxBufferLength = 1024;

// The extension buffer length
static const int Dialog_MaxExtensionLength = 32;

// The enumeration of dialog types
enum DialogType
{
    DialogType_Open = 0,
    DialogType_Save,
    DialogType_Notification,
};

// Define the cross-platform responses
enum DialogResult
{
    DialogResult_OK,
    DialogResult_Cancel,
};

class NativeDialog
{
public:
    
    // Dialog constructor creates either an open or a save dialog
	// Extension argument is only a three-letter argument that should be the end of the file
	// Note that this extension should not include and dot characters or asterisk
     NativeDialog(DialogType Type, const char* Message, const char* Extension = NULL);
    
    // Destructor explicitly takes down any open modals
     ~NativeDialog();
    
    // Show the dialog
     void Show();
    
    // The user's choice is given; if the result is a save
    // or open event, and the given result reference pointer is not null
    // then the given pointer is allocated a buffer of the user's content
    // and must be deleted explicitly by the user
     DialogResult GetInput(char** result = NULL);
    
private:
    
    // Dialog type
    DialogType Type;
    
    // User's choice
    DialogResult Selection;
    
    // Message buffer
    char MessageBuffer[Dialog_MaxBufferLength];
    
    // Result (user input) buffer
    char ResultBuffer[Dialog_MaxBufferLength];

	// File extension buffer
	char FileExtension[Dialog_MaxExtensionLength];
};

// End of inclusion guard
#endif
