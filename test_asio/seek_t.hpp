#ifndef TEST_ASIO_SEEK_H_
#define TEST_ASIO_SEEK_H_

struct seq_seek
{
  template <typename handle_wrapper_t>
  void seek (handle_wrapper_t &handle_, size_t transferred)
  {
    handle_.move_fwd (transferred);
  }

  void set_info (size_t, size_t)
  {

  }
};

struct rnd_seek
{
  template <typename handle_wrapper_t>
  void seek (handle_wrapper_t &handle_, size_t)
  {
    handle_.transfered_ = (rand () % count_) * block_size_;
  }

  void set_info (size_t block_size, size_t count)
  {
    srand (clock ());
    block_size_ = block_size;
    count_ = count;
  }

  size_t block_size_;
  size_t count_;
};

struct rnd_seek_no_buffering
{
  template <typename handle_wrapper_t> 
  void seek (handle_wrapper_t &handle_, size_t)
  {
    handle_.transfered_ = (rand () % count_) * block_size_;
  }

  void set_info (size_t block_size, size_t count)
  {
    srand (clock ());

    DWORD sec_per_cluster = 0;
    DWORD bytes_per_sector = 0;
    DWORD num_of_free_clusters = 0;
    DWORD total_num_of_clusters = 0;

    bool b_disk_fs = GetDiskFreeSpace (0, &sec_per_cluster, &bytes_per_sector, &num_of_free_clusters, &total_num_of_clusters);
    assert (b_disk_fs);

    block_size_ = block_size;//bytes_per_sector;
    count_ = count;// / bytes_per_sector;
  }

  size_t block_size_;
  size_t count_;
};

#endif  // #ifndef TEST_ASIO_SEEK_H_
