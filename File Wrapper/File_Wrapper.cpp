#include "File_Wrapper.h"
#include "File_ErrorCodes.h"

#include <cstring>
#include <cstdio>
#include <cctype>
#include <exception>
#include <limits>

#define mindef(a,b) (a<b?a:b)
#define maxdef(a,b) (a>b?a:b)

namespace File
{
  // Utility functions
  namespace Utils
  {
    // How much to grow the buffer by when going over the buffer size.
    const float GrowthSize = 1.5f;

    char* CopyString(const char* str)
    {
      unsigned len = strlen(str);
      char* ret = new char[len + 1];
      strcpy(ret, str);
      return ret;
    }
  }

  File::File()
  {
    // Shouldn't ever be called.
    throw File_Exception();
  }

  File::File(const char* filename, Mode mode) : open_(false)
  {
    // Can't call the constructor with the same mode as 'before'.
    if(mode == MODE_SAME)
      throw File_Exception(E_BADFLAGS);

    Open(filename, mode);
  }

  File::File(const File& rhs) : open_(false)
  {
    // Copy over the information.
    CopyStatus(rhs);
  }

  File& File::operator=(const File& rhs)
  {
    // Close our current file
    Close();

    // Copy over the information.
    CopyStatus(rhs);

    return *this;
  }

  File::~File()
  {
    // Close the file
    Close();
  }

  void File::Open(const char* filename, Mode mode)
  {
    // If we have a file open, close it
    Close();

    if(mode != MODE_SAME)
    {
      // Apply the defaults to the mode
      ApplyDefaults(mode);
    }
    else
    {
      mode = mode_;
    }

    // Reset the status
    try
    {
      open_       = false;
      filename_   = Utils::CopyString(filename);
      file_       = NULL;
      fileSize_   = 0;
      bufferSize_ = 0;
      currentPos_ = 0;
      protectEnd_ = 0;
      mode_       = mode;
    }
    catch ( std::bad_alloc ) // CopyString can throw.
    {
      throw File_Exception(E_OUTOFMEMORY);
    }

    // Determine the mode for fopen.
    char fopenMode[4] = {(mode & MODE_CREATE ? 'a' : 'r'), (mode & MODE_TEXT ? 't' : 'b') , '+', '\0'};

    // Open the file for reading into our buffer.
    std::FILE* file = std::fopen(filename, fopenMode);

    if(file == NULL)
    {
      delete [] filename_;
      throw File_Exception(E_FOPENERROR);
    }

    // If we're clearing the file, pretend the size is 0.
    long size = 0;

    if((mode & MODE_CLEAR) == 0)
    {
      // Otherwise, get how large the file is (Maximum size of INT_MAX)
      std::fseek(file, 0, SEEK_END);
      size = std::ftell(file);
      if(static_cast<unsigned long>(size) > std::numeric_limits<unsigned>::max())
      {
        delete [] filename_;
        std::fclose(file);
        throw File_Exception(E_FILETOOLARGE);
      }
    }

    // Allocate memory for the buffer.
    fileSize_ = static_cast<unsigned>(size);

    if(mode & MODE_READ)
      bufferSize_ = static_cast<unsigned>(size); // Read-only. Don't add extra bytes to the buffer.
    else
      bufferSize_ = static_cast<unsigned>((size + 1) * Utils::GrowthSize); // Read-write. Multiply by 1.5 to get the buffer size.
    
    try
    {
      file_ = new char[bufferSize_];
    }
    catch ( std::bad_alloc )
    {
      delete [] filename_;
      std::fclose(file);
      throw File_Exception(E_OUTOFMEMORY);
    }

    // Move the pointer back to start and read the file into the buffer.
    std::rewind(file);
    std::fread(file_, sizeof(char), fileSize_, file);

    // Close the file
    std::fclose(file);

    // File is now opened.
    open_ = true;

    // Move the current position if we're on mode_append
    if(mode & MODE_APPEND)
      currentPos_ = fileSize_;

    // Set the protect byte, if applicable
    if(mode & MODE_PROTECT)
      protectEnd_ = currentPos_;
  }

  void File::Close()
  {
    // If we're not open, don't do anything.
    if(open_ == false)
      return;

    // If we're in write mode, write out the file.
    if(mode_ & MODE_WRITE)
      WriteFile(filename_);

    // Free the memory
    delete [] filename_;
    delete [] file_;
    open_ = false;
  }

  void File::CopyStatus(const File& rhs)
  {
    // rhs has no file opened. Don't do anything.
    if(rhs.open_ == false)
    {
      open_ = false;
      return;
    }

    // Copy over the filename and buffer.
    try
    {
      unsigned len = strlen(rhs.filename_);
      filename_ = new char[len + 1];
      strcpy(filename_, rhs.filename_);
    }
    catch ( std::bad_alloc ) // New failed
    {
      throw File_Exception(E_OUTOFMEMORY);
    }

    try
    {
      // Copy the file over as-is.
      file_ = new char[rhs.bufferSize_];
      memcpy(file_, rhs.file_, rhs.bufferSize_);
    }
    catch ( std::bad_alloc ) // New failed
    {
      delete [] filename_;
      throw File_Exception(E_OUTOFMEMORY);
    }

    // Copy over the status
    fileSize_   = rhs.fileSize_;
    bufferSize_ = rhs.bufferSize_;
    currentPos_ = rhs.currentPos_;
    protectEnd_ = rhs.protectEnd_;
    mode_       = rhs.mode_;
    open_       = true;
  }

  void File::ApplyDefaults(Mode& mode)
  {
    // Apply read/write defaults
    if( (!(mode & MODE_READ) && !(mode & MODE_WRITE)) || // Neither specified
        ( (mode & MODE_READ) &&  (mode & MODE_WRITE)) )  // Both specified (invalid)
    {
        // Clear both MODE_READ and MODE_WRITE bits from mode
        mode = static_cast<Mode>(mode & (~MODE_READ) & (~MODE_WRITE));

        // Set MODE_WRITE bit
        mode = static_cast<Mode>(mode | MODE_WRITE);
    }


    // Apply binary/text defaults
    if( (!(mode & MODE_BINARY) && !(mode & MODE_TEXT)) || // Neither specified
        ( (mode & MODE_BINARY) &&  (mode & MODE_TEXT)) )  // Both specified (invalid)
    {
        // Clear both MODE_BINARY and MODE_TEXT bits from mode
        mode = static_cast<Mode>(mode & (~MODE_BINARY) & (~MODE_TEXT));

        // Set MODE_BINARY bit
        mode = static_cast<Mode>(mode | MODE_BINARY);
    }

    if( (!(mode & MODE_OVERWRITE) && !(mode & MODE_CLEAR) && !(mode & MODE_APPEND)) || // None specified
        ( (mode & (MODE_CLEAR | MODE_APPEND | MODE_OVERWRITE)) > maxdef(maxdef(MODE_CLEAR, MODE_APPEND), MODE_OVERWRITE)) )  // More than one specified (invalid)
    {
        // Clear MODE_OVERWRITE, MODE_CLEAR, MOVER_APPEND bits from mode
        mode = static_cast<Mode>(mode & (~MODE_OVERWRITE) & (~MODE_CLEAR) & (~MODE_APPEND));

        // Set MODE_OVERWRITE bit
        mode = static_cast<Mode>(mode | MODE_OVERWRITE);
    }
  }

  void File::WriteFile(const char* filename) const
  {
    // Open the file for writing
    std::FILE* file = std::fopen(filename, "w");

    if(file == NULL)
      throw File_Exception(E_FOPENERROR);

    // Write out the buffer
    std::fwrite(file_, sizeof(char), fileSize_, file);

    // Close the file.
    std::fclose(file);
  }

  bool File::EndOfFile() const
  {
    return currentPos_ == fileSize_;
  }

  char File::GetChar(bool ignoreWhitespace)
  {
    // If we're at EOF, do nothing.
    if(EndOfFile())
      return 0;

    char nextChar = 0;

    do
    {
      nextChar = file_[currentPos_++];
    } while ( ignoreWhitespace && std::isspace(static_cast<int>(nextChar)) && !EndOfFile() );

    return nextChar;
  }

  unsigned File::GetPos(void) const
  {
    return currentPos_;
  }

  unsigned File::GetString(char* outputString, unsigned maxLength, char terminator)
  {
    // Skip the white space before the string. Text mode only, since 
    // ' ' could be meaningful in binary mode.
    if(mode_ & MODE_TEXT)
    {
      // Skip the current whitespace
      char checkChar = file_[currentPos_];

      while(std::isspace(static_cast<int>(checkChar)))
        checkChar = file_[++currentPos_];
    }

    // Take off 1 on the max length to account for null terminator
    --maxLength;

    // We're at the start of the string. Continue until we find terminator.
    unsigned stringStart = currentPos_;

    // Note the ; at the end of the statement
    while(file_[++currentPos_] != terminator && !EndOfFile() && currentPos_ - stringStart < maxLength);

    // Copy the string over to their memory
    memcpy(outputString, &file_[stringStart], currentPos_ - stringStart);

    // Set the null terminator
    outputString[currentPos_ - stringStart] = 0;

    // Return how many bytes we read in. (+1 for Null terminator)
    return currentPos_ - stringStart + 1;
  }

  void File::PutChar(char character, bool ignoreErrors)
  {
    // If we're in read-only mode, do nothing.
    if(mode_ & MODE_READ)
    {
      if(ignoreErrors) // Don't throw
        return;

      throw File_Exception(E_PROTECTED);
    };

    // Make sure we're not writing into protected memory.
    if(currentPos_ < protectEnd_)
    {
      if(ignoreErrors) // Don't throw
        return;

      throw File_Exception(E_PROTECTED);
    }

    // Are we writing to the end of the file?
    if(currentPos_ == fileSize_)
    {
      // Make sure there's enough space in the buffer
      if(++fileSize_ > bufferSize_) // Increase file size (write at end)
        Resize(static_cast<unsigned>((fileSize_) * Utils::GrowthSize));
    }

    // Write to the buffer
    file_[currentPos_++] = character;
  }

  void File::Resize(unsigned desiredSize)
  {
    // Make sure it's a valid size
    if(desiredSize <= bufferSize_)
      return;

    // Attempt to allocate new memory
    char* newBuffer;

    try
    {
      newBuffer = new char[desiredSize];
    }
    catch( std::bad_alloc )
    {
      throw File_Exception(E_OUTOFMEMORY);
    }

    // Copy the buffer over
    // Can't use fileSize_ since PutChar makes it larger than bufferSize_,
    // an out-of-bounds memory read.
    memcpy(newBuffer, file_, bufferSize_);

    // Free memory
    delete [] file_;

    // Set data
    file_ = newBuffer;
    bufferSize_ = desiredSize;
  }
}