# matrix-read-demo
There's a matrix of integers in a text file, how do you go about reaidng it into a 2d array?

A neat way is STL `fstream >> A[i][j]`. For files of the order of tens of MB on current machines, the difference with `fscanf` can become noticeable. 

But what if we read the entire file into memory and parse it there? And, just for the fun of it, what if you do the parsing in assembly?

In this lab, all of these methods are implemented and clocked. Currently for Visual Studio. This is a demo only. Real-world code would have way more error-checking, not mentioning that it's a bad idea to keep structured data in a free-form text file anyway.

  `fstream` vs `fscanf` vs (`fread` + C++) vs (`fread` + ASM).
