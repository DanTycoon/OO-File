#include "File_Exception.h"
#include "File_ErrorCodes.h"

#include <cstring>
#include <cstdio>

namespace File
{
  File_Exception::File_Exception(void) : errorCode(E_UNSPECIFIED), errorString(NULL)
  {
  }

  File_Exception::File_Exception(const char* string) : errorCode(E_CUSTOMSTRING)
  {
    // Copy the string into internal memory
    unsigned len = strlen(string);
    errorString = new char[len + 1]; // Include null terminator
    strcpy(&errorString[0], &string[0]);
  }

  File_Exception::File_Exception(unsigned code) : errorCode(code), errorString(NULL)
  {
    // Print a formatted string into our string buffer.
    char prefix[] = "File_Exception (000): ";
    sprintf(&prefix[16], "%03d): ", errorCode);

    int len = strlen(ErrorStrings[errorCode]) + (sizeof(prefix) / sizeof(*prefix));
    errorString = new char[len];
    sprintf(errorString, "%s%s", prefix, ErrorStrings[errorCode]);
  }

  File_Exception::File_Exception(const File_Exception& rhs)
  {
    errorCode = rhs.errorCode;
    unsigned len = strlen(rhs.errorString);
    errorString = new char[len + 1]; // Include null terminator
    strcpy(&errorString[0], &rhs.errorString[0]);
  }

  File_Exception::~File_Exception()
  {
    // If we allocated memory for a custom error string, free it.
    if(errorString != NULL)
    {
      delete [] errorString;
      errorString = NULL;
    }
  }

  File_Exception& File_Exception::operator=(const File_Exception& rhs)
  {
    errorCode = rhs.errorCode;
    unsigned len = strlen(rhs.errorString);
    errorString = new char[len + 1]; // Include null terminator
    strcpy(&errorString[0], &rhs.errorString[0]);

    return *this;
  }

  const char* File_Exception::what()
  {
    return errorString;
  }

  unsigned File_Exception::whatcode()
  {
    return errorCode;
  }
}