#ifndef TEST_ASIO_FILE_OPENER_H_
#define TEST_ASIO_FILE_OPENER_H_

struct file_opener_seq
{
  static HANDLE create_file (const char *filename)
  {
    return CreateFile (filename, 
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);
  }

  static char *alloc_memory (size_t array_size)
  {
    return new char [array_size];
  }
  static void free_memory (char *array, size_t array_size)
  {
    delete [] array;
  }
};
struct file_opener_rnd
{
  static HANDLE create_file (const char *filename)
  {
    return CreateFile (filename, 
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_RANDOM_ACCESS ,
                       NULL);
  }

  static char *alloc_memory (size_t array_size)
  {
    return new char [array_size];
  }
  static void free_memory (char *array, size_t array_size)
  {
    delete [] array;
  }
};
struct file_opener_seq_nb
{
  static HANDLE create_file (const char *filename)
  {
    return CreateFile (filename, 
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING,
                       NULL);
  }

  static char *alloc_memory (size_t array_size)
  {
    return (char*)VirtualAlloc (0, array_size, MEM_COMMIT, PAGE_READWRITE);
  }
  static void free_memory (char *array, size_t array_size)
  {
    VirtualFree (array, array_size, MEM_DECOMMIT);
  }
};
struct file_opener_rnd_nb
{
  static HANDLE create_file (const char *filename)
  {
    return CreateFile (filename, 
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_NO_BUFFERING,
                       NULL);
  }

  static char *alloc_memory (size_t array_size)
  {
    return (char*)VirtualAlloc (0, array_size, MEM_COMMIT, PAGE_READWRITE);
  }
  static void free_memory (char *array, size_t array_size)
  {
    VirtualFree (array, array_size, MEM_DECOMMIT);
  }
};

#endif  // #ifndef TEST_ASIO_FILE_OPENER_H_
