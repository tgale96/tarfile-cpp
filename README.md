# Build
Depends on [libtar](https://github.com/tklauser/libtar).

`g++ -std=c++11 create_tar.cc -L/usr/local/lib -ltar -o create_tar`

# TODO
Tar doesn't support fast random access. Create table of offsets into file w/ filenames to read quickly.