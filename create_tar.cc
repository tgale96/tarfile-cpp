#include <cassert>
#include <fcntl.h>
#include <libtar.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using std::cout;
using std::endl;
using std::string;
using std::vector;

#define TAR_CALL(code)                                  \
  do {                                                  \
    if (code != 0) {                                    \
    string file = __FILE__;                             \
    string line = std::to_string(__LINE__);             \
    cout << file << ":" << line << " tar error: " <<    \
      strerror(errno) << endl;                          \
      exit(-1);                                         \
    }                                                   \
  } while (0)


/**
 * @brief Reads the file refered to by the current tar header.
 */
int TarReadRegFileToVector(TAR *t, vector<char> *file) {
	mode_t mode;
	size_t size;
	uid_t uid;
	gid_t gid;
	int fdout;
	ssize_t i, k;
	char buf[T_BLOCKSIZE];
	char *filename;

	if (!TH_ISREG(t))
	{
		errno = EINVAL;
		return -1;
	}

	mode = th_get_mode(t);
	size = th_get_size(t);
	uid = th_get_uid(t);
	gid = th_get_gid(t);

  // Extract the file and copy it into the vector
  cout << "file is " << size << " bytes" << endl;
  file->resize(size);
  char* vec_ptr = file->data();
	for (i = size; i > 0; i -= T_BLOCKSIZE)
	{
		k = tar_block_read(t, buf);
		if (k != T_BLOCKSIZE)
		{
			if (k != -1)
				errno = EINVAL;
			close(fdout);
			return -1;
		}

    // Write the block to the output vector
    size_t bytes = (i > T_BLOCKSIZE) ? T_BLOCKSIZE : i;
    std::memcpy(vec_ptr, buf, bytes);
    vec_ptr += bytes;
	}

	return 0;
}

int main() {
  TAR *tar_ptr = nullptr;

  string src_dir = "/Users/trevorgale/Desktop/cifar/test";
  string extract_dir = "./test";
  string tar_name = "/Users/trevorgale/Desktop/test.tar";

  TAR_CALL(tar_open(&tar_ptr, tar_name.c_str(), NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU));
  TAR *created_tar = tar_ptr;
  TAR_CALL(tar_append_tree(tar_ptr, const_cast<char*>(src_dir.c_str()),
          const_cast<char*>(extract_dir.c_str())));
  
  TAR_CALL(tar_append_eof(tar_ptr));

  // TODO(tgale): Should we remove the tar file on destruction?
  // TODO(tgale): Is there a way to create the file and read from it directly?
  TAR_CALL(tar_close(tar_ptr));
  
  TAR_CALL(tar_open(&tar_ptr, tar_name.c_str(), NULL, O_RDONLY, 0644, TAR_GNU));
  int i = 0;
  while ((i = th_read(tar_ptr)) == 0) {
    cout << tar_ptr->th_buf.name << endl;

    if (TH_ISDIR(tar_ptr)) {
      continue;
    } else if (TH_ISREG(tar_ptr)) {
      vector<char> file;
      TAR_CALL(TarReadRegFileToVector(tar_ptr, &file));
      std::ofstream out_file("test.png");
      for (auto &val : file) out_file << val;
      // TAR_CALL(tar_skip_regfile(tar_ptr));      
    } else {
      cout << "error: tar member not dir or regular file" << endl;
    }
  }
  
  if (i == -1) {
    string file = __FILE__;                             
    string line = std::to_string(__LINE__);             
    cout << file << ":" << line << " tar error: " <<    
      strerror(errno) << endl;                          
    exit(-1);                                           
  }
  
  tar_close(tar_ptr);
  return 0;
}
