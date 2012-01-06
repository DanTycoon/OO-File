// MEMORY LEAK CHECKING IN VISUAL STUDIO: ////
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//////////////////////////////////////////////


#include "File_Wrapper.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

#define flags(f) static_cast<File::Mode>(f)
#define constlen(s) (sizeof(s) / sizeof(*s))

// Test opening a file
void test1(void)
{
  File::File f("testfile.txt", static_cast<File::Mode>(File::MODE_READ | File::MODE_TEXT));
}

// Test opening a second file with a file already open.
void test2(void)
{
  File::File file1("testfile.txt", flags(File::MODE_READ | File::MODE_TEXT));
  file1.Open("testfile2.txt");
}

// Test opening an invalid file
void test3(void)
{
  try
  {
    File::File invalid("NotActuallyAFile");
  }
  catch( File::File_Exception e )
  {
    printf("Caught expected exception.\n%s", e.what());
  }
}

// Test saving a file to another name
void test4(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ));
  f.WriteFile("testout.txt");
}

// Test GetChar and EndOfFile
void test5(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ));

  // This is what's in the first text file.
  while(!f.EndOfFile())
  {
    char nextChar = f.GetChar();
    printf("%c", nextChar);
  }
  printf("\nDone!");
}

// Test GetChar with ignoring white space
void test6(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ));

  // Thisiswhat'sinthefirsttextfile.
  while(!f.EndOfFile())
  {
    char nextChar = f.GetChar(true);
    printf("%c", nextChar);
  }
  printf("\nDone!");
}

// Test GetChar after EOF is reached
void test7(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ));

  while(!f.EndOfFile())
    f.GetChar();

  int nextChar = static_cast<int>(f.GetChar());

  std::printf("One past the last character is value %d.\n", nextChar);
}

// Test GetString
void test8(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ | File::MODE_TEXT));

  char string[100];

  unsigned length = f.GetString(string, sizeof(string) / sizeof(*string));

  std::printf("String was: \"%s\" (Length of %d)\n", string, length);
}

// Test GetString with a different terminator
void test9(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ | File::MODE_TEXT));

  char string[100];

  unsigned length = f.GetString(string, sizeof(string) / sizeof(*string), ' ');

  std::printf("String was: \"%s\" (Length of %d)\n", string, length);
}

// Test GetString with not enough space in buffer
void test10(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ | File::MODE_TEXT));

  char string[100];

  unsigned length = f.GetString(string, 10);

  std::printf("String was: \"%s\" (Length of %d)\n", string, length);
}

// Test GetString with JUST enough memory
void test11(void)
{
  File::File f("short.txt", flags(File::MODE_READ | File::MODE_TEXT));

  // Contents of short.txt:
  // Hello
  // 5 characters. + 1 null terminator = 6 characters.

  // Set up borders around the memory to check for overruns
  unsigned char string[12];

  std::memset(&string[0], 0xFF, 3);
  std::memset(&string[9], 0xEE, 3);
  std::memset(&string[3], 0x00, 6);

  std::printf("Memory before GetString:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    // 0x required since printf doesn't put '0x' before 0 values
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n\n");

  f.GetString(reinterpret_cast<char*>(&string[3]), 6);

  std::printf("Memory after GetString:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n");
  std::printf("String: %s", &string[3]);
}

// Test GetString with just UNDER enough memory
void test12(void)
{
  File::File f("short.txt", flags(File::MODE_READ | File::MODE_TEXT));

  // Contents of short.txt:
  // Hello
  // 5 characters. + 1 null terminator = 6 characters.

  // Set up borders around the memory to check for overruns
  unsigned char string[11];

  std::memset(&string[0], 0xFF, 3);
  std::memset(&string[8], 0xEE, 3);
  std::memset(&string[3], 0x00, 5);

  std::printf("Memory before GetString:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    // 0x required since printf doesn't put '0x' before 0 values with #
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n\n");

  f.GetString(reinterpret_cast<char*>(&string[3]), 5);

  std::printf("Memory after GetString:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n");
  std::printf("String: %s", &string[3]);
}

// Test writing to a file.
void test13(void)
{
  File::File f("testfile.txt", flags(File::MODE_WRITE | File::MODE_OVERWRITE));

  // "This is what's in the first text file."

  char string[] = "That";

  for(unsigned i = 0; i < strlen(string); ++i)
  {
    f.PutChar(string[i]);
  }

  f.Close();

  // "That is what's in the first text file."
}

// Test writing past the end of a file and buffer, with Append
void test14(void)
{
  File::File f("test14.txt", flags(File::MODE_WRITE | File::MODE_APPEND));

  // "Short"

  char string [] = " file for testing increasing buffer size.";
  const int len = strlen(string);

  for(int i = 0 ; i < len; ++i)
  {
    f.PutChar(string[i]);
  }

  f.Close();

  // "Short file for testing increasing buffer size."
}

// Test writing into a read-only file
void test15(void)
{
  File::File f("test15.txt", flags(File::MODE_READ | File::MODE_OVERWRITE));

  // "Short"

  char string [] = "Test Failed\nYou shouldn't be able to see this.";
  const int len = strlen(string);

  try
  {
    for(int i = 0 ; i < len; ++i)
    {
      f.PutChar(string[i]);
    }
  }
  catch ( File::File_Exception e )
  {
    printf("Caught expected exception.\n%s", e.what());
  }
}

// Test writing into a read-only file without exception throwing.
void test16(void)
{
  File::File f("test15.txt", flags(File::MODE_READ | File::MODE_OVERWRITE));

  // "Short"

  char string [] = "Test Failed\nYou shouldn't be able to see this.";
  const int len = strlen(string);


  for(int i = 0 ; i < len; ++i)
  {
    f.PutChar(string[i], true);
  }

  f.Close();

  // "Short"
}

// Test writing into an empty file.
void test17(void)
{
  File::File f("test17.txt", flags(File::MODE_WRITE | File::MODE_APPEND));

  char string[] = "Test Passed.";
  const int len = strlen(string);

  for(int i = 0; i < len; ++i)
  {
    f.PutChar(string[i], true);
  }

  f.Close();
}

// Test Read
void test18(void)
{
  File::File f("testfile.txt", flags(File::MODE_READ | File::MODE_PROTECT));

  char string[100] = {0};
  unsigned bytesRead = f.Read(string, 5);

  printf("String was: \"%s\" (%d bytes)\n", string, bytesRead);
}

// Test Read with too many bytes
void test19(void)
{
  File::File f("testFile.txt", flags(File::MODE_READ | File::MODE_PROTECT));

  char string[100] = {0};
  unsigned bytesRead = f.Read(string, 100);

  printf("String was: \"%s\" (%d bytes)\n", string, bytesRead);
}

// Test Read with JUST enough bytes.
void test20(void)
{
  File::File f("short.txt", flags(File::MODE_READ | File::MODE_PROTECT));

  // Contents of short.txt:
  // Hello(no newline or null)

  // Set up borders around the memory to check for overruns
  unsigned char string[11];

  std::memset(&string[0], 0xFF, 3);
  std::memset(&string[8], 0xEE, 3);
  std::memset(&string[3], 0x00, 5);

  std::printf("Memory before Read:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    // 0x required since printf doesn't put '0x' before 0 values with #
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n\n");

  unsigned bytesRead = f.Read(reinterpret_cast<char*>(&string[3]), 5);

  std::printf("Memory after Read:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n");
}

// Test Read with just UNDER enough bytes.
void test21(void)
{
  File::File f("short.txt", flags(File::MODE_READ | File::MODE_PROTECT));

  // Contents of short.txt:
  // Hello(no newline or null)

  // Set up borders around the memory to check for overruns
  unsigned char string[10];

  std::memset(&string[0], 0xFF, 3);
  std::memset(&string[7], 0xEE, 3);
  std::memset(&string[3], 0x00, 4);

  std::printf("Memory before Read:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    // 0x required since printf doesn't put '0x' before 0 values with #
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n\n");

  unsigned bytesRead = f.Read(reinterpret_cast<char*>(&string[3]), 4);

  std::printf("Memory after Read:\n");
  for(unsigned i = 0; i < sizeof(string) / sizeof(*string); ++i)
  {
    std::printf("0x%.2x ", string[i]);
  }
  std::printf("\n");
}

// Test Reopen
void test22(void)
{
  File::File a("test22a.txt", flags(File::MODE_READ | File::MODE_PROTECT));
  File::File b("test22b.txt", flags(File::MODE_READ | File::MODE_PROTECT));
  File::File ModA("test22a.txt", flags(File::MODE_WRITE | File::MODE_CLEAR));
  // ModA clears file A

  char stringA[50] = {0};
  char stringB[50] = {0};

  a.Read(stringA, 50);
  b.Read(stringB, 50);

  printf("File A contained: \"%s\"\n", stringA);
  printf("File B contained: \"%s\"\n", stringB);

  // Put string B before A in the A file.
  ModA.PutString(stringB);
  ModA.PutChar('\n');
  ModA.PutString(stringA);

  ModA.Close();
  b.Close();

  a.Reopen();
  char finalString[100] = {0};
  a.Read(finalString, 100);

  printf("File A now contains:\n%s\n", finalString);
  a.Close();
}

// Test GetPos/SetPos
void test23(void)
{
  File::File f("test23.txt", flags(File::MODE_READ | File::MODE_PROTECT | File::MODE_TEXT));

  char string[100];

  f.GetString(string, 100);
  printf("First string was: \"%s\"\n", string);

  unsigned secondBegin = f.GetPos();

  f.GetString(string, 100);
  printf("Second string was: \"%s\"\n", string);

  f.SetPos(secondBegin);

  f.GetString(string, 100);
  printf("Second string again was: \"%s\"\n", string);

  f.GetString(string, 100);
  printf("Third string is: \"%s\"\n", string);

  if(!f.EndOfFile())
    printf("NOT AT END OF FILE!\n");

}

void (*tests[])(void) = {
  test1,
  test2,
  test3,
  test4,
  test5,
  test6,
  test7,
  test8,
  test9,
  test10,
  test11,
  test12,
  test13,
  test14,
  test15,
  test16,
  test17,
  test18,
  test19,
  test20,
  test21,
  test22,
  test23,
};

int main(int argc, char** argv)
{
  try{
    if(argc > 1)
    {
      int which = std::atoi(argv[1]);

      // If the number given is greater than the number of tests we have, run all of them.
      if(which < sizeof(tests) / sizeof(*tests))
        tests[which]();
      else
      {
        for(unsigned i = 0; i < sizeof(tests) / sizeof(*tests); ++i)
        {
          std::printf("-- BEGIN TEST %02d --------------------------\n", i + 1);
          tests[i]();
          std::printf("-- END TEST %02d ----------------------------\n\n", i + 1);
        }
      }
    }
    else
    {
      // Run the last test
      tests[(sizeof(tests) / sizeof(*tests)) - 1]();
    }
  }
  catch (File::File_Exception e)
  {
    std::printf("Exception occurred!\n%s", e.what());
  }

  _CrtDumpMemoryLeaks();
}