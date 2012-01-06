/* File_ErrorCodes.h
 * Purpose: Provide a central file to define all error codes.
 */

#ifndef FILE_ERRORCODES_H
#define FILE_ERRORCODES_H

namespace File
{
  enum ErrorCode
  {
    E_UNSPECIFIED,
    E_CUSTOMSTRING,
    E_FOPENERROR,
    E_OUTOFMEMORY,
    E_FILETOOLARGE,
    E_BADFLAGS,
    E_PROTECTED,
    E_INVALIDPOSITION,
  };

  static const char* ErrorStrings[] = {
    "Unspecified Error",                     // E_UNSPECIFIED
    0,                                       // E_CUSTOMSTRING
    "fopen returned an error.",              // E_FOPENERROR
    "Out of system memory.",                 // E_OUTOFMEMORY
    "File is larger than int max.",          // E_FILETOOLARGE
    "Inappropriate flags specified.",        // E_BADFLAGS
    "Attempt to write into protected file.", // E_PROTECTED
    "Invalid position specified.",           // E_INVALIDPOSITION
  };
}

#endif