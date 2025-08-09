# Phoenix
Simple drag and drop application for X/Wayland written in C++.

# Dependencies
- Gtkmm 4.0
- Spdlog
- CLI11

# Compile
```bash
mkdir build && cd build
cmake ..
make # or make -j$(nproc) for faster compilation time

# Install
sudo make install
```

# Example
```bash
$ phoenix source file1.txt file2.cpp file3.html # Display 3 files to drag to anywhere
$ phoenix source -a file1.txt file2.cpp file3.html # Drag 3 files at once
$ phoenix source -A file1.txt file2.cpp file3.html # Drag 3 files at once but compact UI

# If there are special files named like "-a", "-A", you can drag them
# by using the following command
$ phoenix source [OPTIONS..] -- -a -A
```

# Usage
```
$ phoenix -h
phoenix - lightweight DnD source/target 


build/phoenix [OPTIONS] SUBCOMMAND


OPTIONS:
  -h,     --help              Print this help message and exit 
  -v,     --version           Show version 
  -c,     --config [~/.config/phoenix/phoenix.ini]  
                              Config file 
          --log-level LEVEL:{trace,debug,info,warn,err,critical,off} [info]  
                              Change log level 

SUBCOMMANDS:
  source                      Act as a drag source 
  target                      Act as a drop target
```
```
$ phoenix source -h
Act as a drag source 


build/phoenix source [OPTIONS] FILES...


POSITIONALS:
  FILES TEXT ... REQUIRED     Files to drag 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -x,     --and-exit          Exit after a single completed drop 
  -a,     --all               Drag all files at once 
  -A,     --all-compact       Drag all files at once, only displaying the number of files
```

```
$ phoenix target -h
Act as a drop target 


build/phoenix target [OPTIONS]


OPTIONS:
  -h,     --help              Print this help message and exit 
  -p,     --print-path        Print file paths instead of URIs
```
