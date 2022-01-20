using namespace std; 
#include <iostream> 
#include <iomanip> 
#include <sstream> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/time.h> 
#include <mpi.h> 
 
#define MX_SZ 320 
#define SEED 2397           /* random number seed */ 
#define MAX_VALUE  100.0    /* maximum size of array elements A, and B */ 
 
/* copied from mpbench */ 
#define TIMER_CLEAR     (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0) 
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0) 
#define TIMER_ELAPSED   ((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000)) 
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0) 
struct timeval tv1,tv2; 
 
float *a,*b,*c; 
 
void get_index_size(int argc,char *argv[],int *dim_l,int *dim_m,int *dim_n) { 
   if(argc!=2 && argc!=4) { 
      cout<<"usage:  mm_mult_serial [l_dimension] <m_dimension n_dimmension>" 
           << endl; 
      exit(1); 
   } 
   else { 
      if (argc == 2) { 
         *dim_l = *dim_n = *dim_m = atoi(argv[1]); 
      } 
      else { 
         *dim_l = atoi(argv[1]); 
         *dim_m = atoi(argv[2]); 
         *dim_n = atoi(argv[3]); 
      } 
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
 
/* 
   Routine that outputs the matrices to the screen  
*/ 
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
 
int main( int argc, char *argv[]) 
{ 
   MPI::Init(argc,argv); 
   int rank=MPI::COMM_WORLD.Get_rank(); 
   int numtasks=MPI::COMM_WORLD.Get_size(); 
   MPI::Status status;  
    
   float dot_prod; 
   int dim_l,dim_n,dim_m; 
   int i,j,k; 
   get_index_size(argc,argv,&dim_l,&dim_m,&dim_n); 
    
    a = new (nothrow) float[dim_l*dim_m]; 
    b = new (nothrow) float[dim_m*dim_n]; 
    c = new (nothrow) float[dim_l*dim_n]; 
     
    if(a==0 || b==0 || c==0)  
    { 
     cout <<"ERROR:  Insufficient Memory" << endl; 
     exit(1); 
    } 
    srand48(SEED); 
    fill_matrix(a,dim_l,dim_m); 
    fill_matrix(b,dim_m,dim_n); 
     
 
   if(rank==0) 
   {     
     cout << "A matrix =" << endl; 
     print_matrix(a,dim_l,dim_m); 
     cout << endl; 
     cout << "B matrix =" << endl; 
     print_matrix(b,dim_m,dim_n); 
     cout << endl; 
   } 
 
   TIMER_CLEAR; 
   TIMER_START; 
 
   float *sa = new (nothrow) float[(dim_l*dim_m)]; 
   float *sc = new (nothrow) float[(dim_l*dim_m)]; 
   float sum =0.0;  
    
   int* scouts = new (nothrow) int[numtasks]; 
   int* displs = new (nothrow) int[numtasks]; 
    
   int* scouts2 = new (nothrow) int[numtasks]; 
   int* displs2 = new (nothrow) int[numtasks]; 
    
   int temp=0; 
   int extra = dim_l%numtasks; 
   int space, space2; 
   while(temp < numtasks) 
   { 
    if(rank == temp) 
    { 
       if(extra>0)  
       { 
         space = (dim_l/numtasks)*dim_m + dim_m; 
         space2 = (dim_l/numtasks)*dim_n + dim_n; 
       } 
       else  
       { 
         space = (dim_l/numtasks)*dim_m; 
         space2 = (dim_l/numtasks)*dim_n; 
       } 
    } 
    if(extra>0)extra--; 
    temp++; 
    MPI_Barrier(MPI_COMM_WORLD); 
   } 
 
 
 
   if(rank==0) 
   { 
     int offset=0, offset2=0; 
     extra = dim_l%numtasks; 
     for(int x=0;x<numtasks;x++) 
     { 
       displs[x] = offset; 
       displs2[x] = offset2; 
       if(extra>0)  
       { 
         scouts[x] = (dim_l/numtasks)*dim_m + dim_m; 
         scouts2[x] = (dim_l/numtasks)*dim_n + dim_n; 
         extra--; 
       } 
       else  
       { 
         scouts[x] = (dim_l/numtasks)*dim_m; 
         scouts2[x] = (dim_l/numtasks)*dim_n; 
       } 
       offset += scouts[x]; 
       offset2 += scouts2[x]; 
     } 
   } 
    
    
    
    //SCATTER NUMBERS OF MATRIX A 
    MPI_Scatterv(a,scouts,displs,MPI_FLOAT,sa,space,MPI_FLOAT,0,MPI_COMM_WORLD); 
     
    int q=0; 
     
   //DOT PRODUCT 
   for(int u=0; u<space/dim_m;u++) 
   { 
     for (i = 0; i < dim_n; i++) 
     { 
       for (j = 0; j <dim_m; j++)  
       { 
         sum = (*(sa+ j+(u*dim_m) ) * *(b+(j*dim_n+i)))+sum;   
         if((j+1)%dim_m==0) 
         { 
           *( sc+ q ) = sum; 
           q++; 
           sum = 0.0;  
         }  
       }   
     }    
   } 
    
   MPI_Gatherv(sc, space2,MPI_FLOAT, c, scouts2,displs2, MPI_FLOAT, 0, MPI_COMM_WORLD); 
     
   TIMER_STOP; 
 
   if(rank==0)  
   { 
     cout<<"C matrix ="<<endl;  
     print_matrix(c,dim_l,dim_n); 
     cout << endl; 
     cout << "time=" << setprecision(8) <<  TIMER_ELAPSED/1000000.0 << " seconds" << endl; 
   } 
    
   MPI::Finalize(); 
}