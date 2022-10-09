#include <stdio.h>
#include <omp.h>

void processa(int tamanho, int E[5][5], int F[3][3],int resultado[5][5])
{
 int i, j;
#pragma omp parallel
{ 
    #pragma omp for collapse(2)
    for (i = 1; i < tamanho - 1; i++)
        for (j = 1; j < tamanho - 1; j++) 
        {
            // estou na posição i,j
            int acc = 0;
            //1,1
            //#pragma omp parallel for
            for (int k = -1; k <= 1; k++)
            {
              //  #pragma omp parallel for
                for (int l = -1; l <= 1; l++)
                {
                    acc += E[i + k][j + l] * F[k+1][l+1];
                  //  printf("%d,",E[i + k][j + l]);
                    // printf("%d,",F[k+1][l+1]);
                }
                //printf("\n");
            }
            acc= (int)acc/7;
            //printf("%d   \n", acc);
            resultado[i][j] = acc;
            //  E[i][j] = E[i][j] * 3 + E[i+1][j] * 1 + E[i-1][j] * 1 + E[i][j+1] * 1 + E[i][j-1] * 1
        }
}
    
 //   return resultado;

}

int main()
{
	
    int matriz[5][5] = {{2, 3, 4, 5, 6}, {112, 113, 114, 115, 116}, {22, 23, 24, 25, 26}, {32, 33, 34, 35, 36}, {42, 43, 44, 45, 46}};
    int resultado[5][5] = {{2, 3, 4, 5, 6}, {12, 13, 14, 15, 16}, {22, 23, 24, 25, 26}, {32, 33, 34, 35, 36}, {42, 43, 44, 45, 46}};
    int tamanhoReferencia = 3;
    int referencia[3][3] = {{0, 1, 0}, {1, 3, 1}, {0, 1, 0}};

    
    for (int i = 0; i<5;i++){
        for (int j = 0; j < 5; j++)
            {
            printf("%d ",matriz[i][j]);
            }
        printf("\n");
    }
    double begin, end;
	begin = omp_get_wtime();
     processa(5, matriz, referencia,resultado);
     end = omp_get_wtime();
	printf("Tempo em segundos %f\n", end-begin); 
    printf("===============================\n");
    for (int i = 0; i<5;i++){
        for (int j = 0; j < 5; j++)
            {
            printf("%d ",resultado[i][j]);
            }
        printf("\n");
    }
    
    return 0;
}
