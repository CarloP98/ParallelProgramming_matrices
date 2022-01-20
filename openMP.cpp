using namespace std; 
#include <iostream> 
#include <iomanip> 
#include <sstream> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/time.h> 
#include <omp.h> 
 
#define MX_SZ 320 
#define SEED 2397           /* random number seed */ 
#define MAX_VALUE  100.0    /* maximum size of array elements A, and B */ 
 
/* copied from mpbench */ 
#define TIMER_CLEAR     (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0) 
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0) 
#define TIMER_ELAPSED   (double) (tv2.tv_usec-tv1.tv_usec)/1000000.0+(tv2.tv_sec-tv1.tv_sec) 
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0) 
struct timeval tv1,tv2; 
 
float *a,*b,*c; 
 
void get_index_size(int argc,char *argv[],int *dim_l,int *dim_m,int *dim_n) { 
   if(argc!=5) { 
      cout<<"usage: NumOfThreads [l_dimension]    <m_dimension n_dimmension>"<< 
endl; 
      exit(1); 
   } 
   else { 
      *dim_l = atoi(argv[2]); 
      *dim_m = atoi(argv[3]); 
      *dim_n = atoi(argv[4]); 
      } 
   if (*dim_l<=0 || *dim_n<=0 || *dim_m<=0) { 
      cout<<"Error: number of rows and/or columns must be greater than 0" 
          << endl; 
      exit(1); 
   } 
} 
 
void fill_matrix(float *array,int dim_m,int dim_n) 
{ 
   int i,j; 
   for(i=0;i<dim_m;i++) { 
      for (j=0;j<dim_n;j++) { 
         array[i*dim_n+j]=drand48()*MAX_VALUE; 
      } 
   } 
} 
 
void print_matrix(float *array,int dim_m,int dim_n) 
{ 
   int i,j; 
   for(i=0;i<dim_m;i++) { 
      for (j=0;j<dim_n;j++) { 
         cout << array[i*dim_n+j] << " "; 
      } 
      cout << endl; 
   } 
} 
 
void multiply(int NumOfThreads,int dim_l,int dim_m,int dim_n, int POSITION, int count) 
{ 
   for (int i=POSITION;i<POSITION+count;i++) { 
      for (int j=0;j<dim_n;j++) { 
         float dotProduct = 0.0; 
         for (int k=0;k<dim_m;k++) { 
            dotProduct += *(a+i*dim_m+k)  *  *(b+k*dim_n+j); 
         } 
         *(c+i*dim_n+j) = dotProduct; 
      } 
   } 
} 
 
int main( int argc, char *argv[]) 
{ 
   float dot_prod; 
   int dim_l,dim_n,dim_m; 
 
   get_index_size(argc,argv,&dim_l,&dim_m,&dim_n); 
 
   cout<<"Number of threads ="<<argv[1]<<endl; 
 
 
   a = new (nothrow) float[dim_l*dim_m]; 
   b = new (nothrow) float[dim_m*dim_n]; 
   c = new (nothrow) float[dim_l*dim_n]; 
   if(a==0 || b==0 || c==0) { 
     cout <<"ERROR:  Insufficient Memory" << endl; 
     exit(1); 
   } 
    
   srand48(SEED); 
   fill_matrix(a,dim_l,dim_m); 
   fill_matrix(b,dim_m,dim_n); 
 
   //cout << "A matrix =" << endl; 
   //print_matrix(a,dim_l,dim_m); 
   //cout << endl; 
 
   //cout << "B matrix =" << endl; 
   //print_matrix(b,dim_m,dim_n); 
   //cout << endl; 
 
   int NumOfThreads = atoi(argv[1]); 
   int extra = dim_l%NumOfThreads; 
   int position=0;  
   int plus=0; 
 
   TIMER_CLEAR; 
   TIMER_START; 
 
   omp_set_num_threads(NumOfThreads);    
   int nthreads = omp_get_num_threads(); 
      
   #pragma omp parallel 
   { 
     int tid = omp_get_thread_num(); 
      
     for (char i = 0; i < NumOfThreads; ++i)  
     { 
       plus = (extra>0) ? 1 : 0; 
       if(tid==i)multiply(NumOfThreads, dim_l, dim_m, dim_n, position, (dim_l/NumOfThreads)+plus); 
       position = position + (dim_l/NumOfThreads) + plus; 
       extra--; 
     } 
   } 
 
   
   TIMER_STOP; 
 
   //cout << "C matrix =" << endl; 
   //print_matrix(c,dim_l,dim_n); 
   //cout << endl; 
    
   cout <<"time=" << setprecision(8) <<  TIMER_ELAPSED  
        << " seconds" << endl; 
} 
