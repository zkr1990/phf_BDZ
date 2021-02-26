# phf_BDZ: perfect hash function using BDZ algorithm

## Description

Naive C++ implementation of BDZ algorithm for perfect hash function. Minimum-phf will be added.

All-in-one naive C++, simple comments.
Correctness not guaranteed.

## Run
Generate keys for test input. (input.txt)
Inputs of phf are 64 bits integer numbers.

~~~
cd input_file
python test1_generation.py
~~~

Modify line 222 in chd.cpp according to the number of keys generated in input.txt.

~~~
cd ..
g++ bdz.cpp -o chd
./bdz
~~~

Notice:
The algorithm requires the 3-partite graph constructed successfully.
If you find "unexpected graph, please change h0,h1,h2." This means the 3-graph is not acyclic. 
Please change your h functions and re-run.


## Reference
Original paper:

F. C. Botelho, R. Pagh, N. Ziviani. Simple and space-efficient minimal perfect hash functions. In Proceedings of the 10th International Workshop on Algorithms and Data Structures (WADs'07), Springer-Verlag Lecture Notes in Computer Science, vol. 4619, Halifax, Canada, August 2007, 139-150.
http://cmph.sourceforge.net/bdz.html

Source code fork from:
https://github.com/evilmucedin/minimal-perfect-hash.git
https://github.com/LTzycLT/MPHF.git
Minor changes for conciseness. Use Jenkins hash for h0, h1, h2.