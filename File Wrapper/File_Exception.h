/* File_Exception.h
 * Purpose: Give a single exception class for the File system to
 * throw on errors.
 *
 * A list of error codes is described in File_ErrorCodes.h
 */

#ifndef FILE_EXCEPTION_H
#define FILE_EXCEPTION_H

namespace File
{
  class File_Exception
  {
  public:
    File_Exception();
    File_Exception(const char* str);
    File_Exception(unsigned errCode);
    File_Exception(const File_Exception& rhs);

    ~File_Exception(void);

    File_Exception& operator=(const File_Exception& rhs);

    virtual const char* what(void);
    unsigned whatcode(void);

  private:
    unsigned errorCode;
    char* errorString;  // Used for custom error strings.
  };
}

#endif