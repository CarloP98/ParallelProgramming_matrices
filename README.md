# ParallelProgramming_matrices
Parallelized matrix multiplications

MPI
```console
mpic++ MPI.cpp -o MPI
./MPI [l_dimension] <m_dimension n_dimmension>
```

OpenMP
```console
g++ openMP.cpp -fopenmp -lpthread
./a.out NumOfThreads [l_dimension] <m_dimension n_dimmension>
```

Threads
```console
g++ threaded.cpp -pthread
./a.out NumOfThreads [l_dimension] <m_dimension n_dimmension>
```
<br />


MPI results

![scrrenshot](img/time.PNG)
![scrrenshot](img/speedup.PNG)
![scrrenshot](img/efficiency.PNG)





https://en.wikipedia.org/wiki/Message_Passing_Interface

https://en.wikipedia.org/wiki/OpenMP
