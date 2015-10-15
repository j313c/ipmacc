#include <malloc.h>
#include <time.h>
#include <openacc.h>
//#include <accelmath.h>
#include <math.h>

#define LEN 1024
#define SIZE LEN*LEN

int main(int argc, char *argv[])
{
    int i;
    #ifdef __NVCUDA__
    acc_init( acc_device_nvcuda );
    #endif 
    #ifdef __NVOPENCL__
    acc_init( acc_device_nvocl );
    #endif 

    float *a, *b, *c;
    //float *kK[10], *jJ[10];
    float *seq;
    //acc_init( acc_device_nvidia );
    a=(float*)malloc(SIZE*sizeof(float));
    b=(float*)malloc(SIZE*sizeof(float));
    c=(float*)malloc(SIZE*sizeof(float));
    seq=(float*)malloc(SIZE*sizeof(float));

    // Initialize matrices.
    for (i = 0; i < SIZE; ++i) {
        //B
        a[i] = (float)i ;
        b[i] = (float)2*i;
        c[i] = 0.0f;
    }// B

    unsigned long long int tic, toc;
    // Compute vector Add
    int k,j,l;
    for(k=0; k<3; k++){
        printf("Calculation on GPU ... ");
        tic = clock();

        #pragma acc data pcopyin(a[0:SIZE],b[0:SIZE]) pcopy(c[0:SIZE])
        {
            #pragma acc kernels
            {
                #pragma acc loop independent   
                {
                    for (i = 0; i < LEN; ++i) {
                        #pragma acc loop independent  
                        {
                            for(j=0; j<LEN; j++){
                                float sum=0;
                                for(l=0; l<LEN; l++){
                                    sum += a[i*LEN+l]*b[l*LEN+j];
                                }
                                c[i*LEN+j]=sum;
                            }
                        }
                    }
                }
            }
        }
        toc = clock();
        printf(" %6.4f ms\n",(toc-tic)/(float)1000);
    }

    // ****************
    // double-check the OpenACC result sequentially on the host
    // ****************
    // Perform the add

    /*
    printf("A:\n");
    for (i = 0; i < LEN; ++i) {
       for(j=0; j<LEN; j++)
           printf("%6.4f ",a[i*LEN+j]);
        printf("\n");
    }
    printf("B:\n");
    for (i = 0; i < LEN; ++i) {
       for(j=0; j<LEN; j++)
           printf("%6.4f ",b[i*LEN+j]);
        printf("\n");
    }
    printf("C:\n");
    for (i = 0; i < LEN; ++i) {
       for(j=0; j<LEN; j++)
           printf("%6.4f ",c[i*LEN+j]);
        printf("\n");
    }
    */

    printf("Calculation on CPU ... ");

    tic = clock();
    for (i = 0; i < LEN; ++i) {
        for(j=0; j<LEN; j++){
            float s=0;
            for(l=0; l<LEN; l++){
                s += a[i*LEN+l]*b[l*LEN+j];
            }
            seq[i*LEN+j]=s;
            if(seq[i*LEN+j]!=c[i*LEN+j]){
                fprintf(stderr,"mismatch on %dx%d\n",i,j);
                exit(-1);
            }
        }
    }
    toc = clock();
    printf(" %6.4f ms\n",(toc-tic)/(float)1000);

    fprintf(stderr,"OpenACC matrix multiply test with dynamic arrays was successful!\n");

    return 0;
}
