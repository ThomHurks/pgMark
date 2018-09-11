# pgMark

pgMark is a domain- and query language-independent framework targeting highly tunable generation of both graph instances and graph query workloads based on user-defined schemas.

pgMark is based on gMark, for more details about gMark, please refer to our technical report: http://arxiv.org/abs/1511.08386**

pgMark is a version of gMark that supports property graphs, whereas gMark only supports edge-labelled graphs.

**gMark was demonstrated in VLDB 2016. The gMark research paper was published in the TKDE journal.** 

If you use pgMark, please cite:

    @article{BBCFLA17,
      author = {Bagan, G. and Bonifati, A. and Ciucanu, R. and Fletcher, G. H. L. and Lemay, A. and Advokaat, N.},
      title = {{gMark}: Schema-Driven Generation of Graphs and Queries},
      journal = {IEEE Transactions on Knowledge and Data Engineering},
      volume = {29},
      number = {4},
      pages = {856--869},
      year = {2017}
    }

and/or

    @article{BBCFLA16,
      author = {Bagan, G. and Bonifati, A. and Ciucanu, R. and Fletcher, G. H. L. and Lemay, A. and Advokaat, N.},
      title = {Generating Flexible Workloads for Graph Databases},
      journal = {PVLDB},
      volume = {9},
      number = {13},
      pages = {1457--1460},
      year = {2016}
    }
    
# How to compile
pgMark uses [CMake](https://cmake.org/runningcmake/). Using the commandline, first go to the pgMark directory, then:
```commandline
ccmake .
```
Then press 'c' to configure and press 'g' to generate and exit. Then:
```commandline
make
```
Note that by default pgMark assumes you have clang-tidy installed. If you do not have clang-tidy or do not want to install it, then you can remove the ```CXX_CLANG_TIDY``` target property from the CMakeLists.txt file.
Based on if you use GCC or CLANG to compile the c++ code you can also pass ```CLANG_COMPILER_FLAGS``` or ```GCC_COMPILER_FLAGS``` to the ```target_compile_options``` command in the CMakeLists.txt file. You can find out what compiler ```make``` uses on your system by checking the ```CC``` environment variable.

If you use an IDE such as CLion it should automatically detect the CMakeLists.txt file and allow you to build. CLion also has the required binaries such as CMake and clang-tidy built-in.
    
# Usage
```commandline
./pgMark examples/social_network.xml 1000
./pgMark examples/social_network.xml 10000 --output=graph.csv
./pgMark examples/social_network.xml 1000 | csplit - /\#\#\#/
./pgMark --help
```
