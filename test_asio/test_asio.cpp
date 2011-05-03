// test_asio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>

#include <asio.hpp>

#include "file_opener.hpp"
#include "seek_t.hpp"
#include "str_to_float.hpp"


#include <boost/iostreams/stream_buffer.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>


//////////////////////////////////////////////////////////////////////////
struct random_handle_wrapper
{
  random_handle_wrapper (asio::io_service &io_service, HANDLE file)
  : handle_ (io_service, file)
  , file_ (file)
  , transfered_ (0)
  {

  }

  ~random_handle_wrapper ()
  {
    handle_.cancel ();
  }

  template <typename MutableBufferSequence>
  std::size_t read_some (const MutableBufferSequence& buffers, asio::error_code &ec)
  {
    return handle_.read_some_at (transfered_, buffers, ec);
  }

  template <typename MutableBufferSequence, typename ReadHandler>
  void async_read_some (const MutableBufferSequence& buffers, ReadHandler handler)
  {
    handle_.async_read_some_at (transfered_, buffers, handler);
  }

  void move_fwd (size_t transfered)
  {
    transfered_ += transfered;
  }

  asio::windows::basic_random_access_handle<> handle_;
  HANDLE  file_;
  boost::uint64_t  transfered_;
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template <typename strategy_t, size_t array_size>
struct processor 
{
  processor (asio::io_service &io_service_, const char *filename)
  : file_ (strategy_t::create_file (filename))
  , stream_ (io_service_, file_)
  , complete_condition_ (boost::bind (&processor::complete_condition, this, asio::placeholders::error, asio::placeholders::bytes_transferred))
  , handler_ (boost::bind (&processor::read_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred))
  , is_end_ (false)
  , transferred_ (0)
  {
    data_a_ = strategy_t::alloc_memory (array_size);
    data_b_ = strategy_t::alloc_memory (array_size);
    data_ = &data_a_;
    DWORD filesize = GetFileSize(file_, 0);
    seek_.set_info (array_size, filesize / array_size);
  }

  ~processor ()
  {
    strategy_t::free_memory (data_a_, array_size);
    strategy_t::free_memory (data_a_, array_size);
  }

  void start (const char *section)
  {
    char tmp[128];
    int n = 0;

    while (true)
      {
        size_t read = asio::read (stream_, asio::buffer (data_a_, array_size));
        assert (read == array_size);
        
        _snscanf (data_a_, sizeof (tmp) - 1, "%s%n", tmp, &n);
        assert (n < sizeof (tmp));

        transferred_ += n;
        seek_.seek (stream_, transferred_);

        if (_stricmp (tmp, section) == 0)
          {
            break;
          }
      }

    read ();
  }

  void read ()
  {
    asio::async_read (stream_, asio::buffer (*data_, array_size), 
                      complete_condition_, 
                      handler_);
  }

  void read_handler (asio::error_code ec, size_t transferred)
  {
    array_t *data = data_;
    data_ = data_ == &data_a_ ? &data_b_ : &data_a_;
    seek_.seek (stream_, transferred);

    if (!is_end_)
      read ();

    transferred_ += transferred;
    bool b = (-1 == process_buffer (vec, (*data), (*data) + transferred));
    if (!is_end_)
      is_end_ = b;
  }

  size_t process_buffer (std::vector <float> &vec, char *start, char *end)
  {
    bool is_end = false;
    char *begin = start;
    for (; start < end; ++start)
      {
        if (*start == '/')
          {
            is_end = true;
            break;
          }

        if (isspace (*start))
          {
            *start = 0;
            if (begin != start)
              {
                strategy_t::string_to_float (vec, begin);
              }

            begin = start + 1;
          }
      }

    return is_end ? -1 : (end - begin);
  }

  bool complete_condition (asio::error_code ec, size_t transferred)
  {
    is_end_ = vec.size () > 5000000 || (ec.value () != 0 && ec.value () != ERROR_IO_PENDING);
    return is_end_;
  }

  typedef char *                  array_t;

  HANDLE                          file_;
  typename strategy_t::stream_t   stream_;
  typename strategy_t::seek_t     seek_;

  bool                            is_end_;

  std::vector <float>             vec;
  boost::uint64_t                 transferred_;

  array_t                         *data_;
  array_t                         data_a_;
  array_t                         data_b_;

  boost::function <bool (asio::error_code, size_t)> complete_condition_;
  boost::function <void (asio::error_code, size_t)> handler_;

  enum { static_size = array_size, };
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template <typename processor_t>
struct test 
{
  test (asio::io_service &io_service, const char *filename, const char *section)
  {
    processor_t p (io_service, filename);
    clock_t start = clock ();
    p.start (section);
    io_service.reset ();
    io_service.run ();
    clock_t end = clock ();

    size_           = processor_t::static_size;
    time_ms_        = (double (end) - double (start)) / double (CLOCKS_PER_SEC);
    bytes_          = p.transferred_;
    elems_          = p.vec.size ();
    data_transfer_  = bytes_ / time_ms_ / 1024.0 / 1024.0;
  }

  void print (const char *name)
  {
    std::cout << name << ";" << size_ << ";" << time_ms_ << ";" << bytes_ << ";" << elems_ << ";" << data_transfer_ << ";" << std::endl;
  }

  size_t size_;
  double time_ms_;
  size_t bytes_;
  size_t elems_;
  double data_transfer_;
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template <typename str_to_float_t, typename file_opener_t, typename seek_t>
struct strategy
{
  typedef random_handle_wrapper stream_t;
  typedef seek_t                seek_t;
  typedef str_to_float_t        string_to_float_t;

  static HANDLE create_file (const char *filename)
  {
    HANDLE handle = file_opener_t::create_file (filename);
    assert (handle != INVALID_HANDLE_VALUE);
    return handle;
  }

  static char *alloc_memory (size_t array_size)
  {
    return file_opener_t::alloc_memory (array_size);
  }
  
  static void free_memory (char *array, size_t array_size)
  {
    file_opener_t::free_memory (array, array_size);
  }

  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    return string_to_float_t::string_to_float (vec, str);
  }
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template <typename str_to_float_t, typename file_opener_t, typename seek_t>
void do_test (asio::io_service &io_service, char *argv[], const char *name)
{
  //test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 512> >        (io_service, argv[1], argv[2]).print (name);
  //test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024> >       (io_service, argv[1], argv[2]).print (name);
  //test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 4> >   (io_service, argv[1], argv[2]).print (name);
  //test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 16> >  (io_service, argv[1], argv[2]).print (name);
  test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 32> >  (io_service, argv[1], argv[2]).print (name);
  test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 64> >  (io_service, argv[1], argv[2]).print (name);
  test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 128> > (io_service, argv[1], argv[2]).print (name);
  test <processor <strategy <str_to_float_t, file_opener_t, seek_t>, 1024 * 256> > (io_service, argv[1], argv[2]).print (name);
}
//////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
  using namespace std;

  asio::io_service io_service;

  //////////////////////////////////////////////////////////////////////////
  do_test <fast_strtod_, file_opener_seq, seq_seek>         (io_service, argv, "fast_strtod - seq - seq");
  do_test <fast_strtod_, file_opener_rnd, seq_seek>         (io_service, argv, "fast_strtod - rnd - seq");
  do_test <fast_strtod_, file_opener_seq, rnd_seek>         (io_service, argv, "fast_strtod - seq - rnd");
  do_test <fast_strtod_, file_opener_rnd, rnd_seek>         (io_service, argv, "fast_strtod - rnd - rnd");
  do_test <fast_strtod_, file_opener_seq_nb, seq_seek>      (io_service, argv, "fast_strtod - seq_nb - seq");
  do_test <fast_strtod_, file_opener_rnd_nb, seq_seek>      (io_service, argv, "fast_strtod - rnd_nb - seq");
  do_test <fast_strtod_, file_opener_seq_nb, rnd_seek>      (io_service, argv, "fast_strtod - seq_nb - rnd");
  do_test <fast_strtod_, file_opener_rnd_nb, rnd_seek>      (io_service, argv, "fast_strtod - rnd_nb - rnd");

  //////////////////////////////////////////////////////////////////////////
  do_test <irr_fast_atof_, file_opener_seq, seq_seek>       (io_service, argv, "irr_fast_atof_ - seq - seq");
  do_test <irr_fast_atof_, file_opener_rnd, seq_seek>       (io_service, argv, "irr_fast_atof_ - rnd - seq");
  do_test <irr_fast_atof_, file_opener_seq, rnd_seek>       (io_service, argv, "irr_fast_atof_ - seq - rnd");
  do_test <irr_fast_atof_, file_opener_rnd, rnd_seek>       (io_service, argv, "irr_fast_atof_ - rnd - rnd");
  do_test <irr_fast_atof_, file_opener_seq_nb, seq_seek>    (io_service, argv, "irr_fast_atof_ - seq_nb - seq");
  do_test <irr_fast_atof_, file_opener_rnd_nb, seq_seek>    (io_service, argv, "irr_fast_atof_ - rnd_nb - seq");
  do_test <irr_fast_atof_, file_opener_seq_nb, rnd_seek>    (io_service, argv, "irr_fast_atof_ - seq_nb - rnd");
  do_test <irr_fast_atof_, file_opener_rnd_nb, rnd_seek>    (io_service, argv, "irr_fast_atof_ - rnd_nb - rnd");


	return 0;
}

