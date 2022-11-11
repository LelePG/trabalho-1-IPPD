#include <stdio.h>
#include <mpi.h>
int main(int argc, char** argv) {
 int quantidade_de_maquinas, meu_codigo, aux;
 char computador[MPI_MAX_PROCESSOR_NAME];
 MPI_Init(NULL, NULL); // Inicialização
 MPI_Comm_size(MPI_COMM_WORLD, &quantidade_de_maquinas); // Quantos processos envolvidos?
 MPI_Comm_rank(MPI_COMM_WORLD, &meu_codigo); // Meu identificador
 MPI_Get_processor_name(computador, &aux);
 int numeros_para_imprimir = 13;
 int numeros_do_core = numeros_para_imprimir/quantidade_de_maquinas;
 int resto = numeros_para_imprimir%quantidade_de_maquinas;
 for(int i = numeros_do_core * meu_codigo; i < numeros_do_core*meu_codigo+numeros_do_core; i++){
    printf("Estou executando no computador %s, meu rank %d de um total de %d processos e i = %d\n",
        computador, meu_codigo, quantidade_de_maquinas, i);
 }
 if(meu_codigo < resto && resto != 0){
    // for(int i = numeros_do_core*quantidade_de_maquinas; i != resto + (numeros_do_core*quantidade_de_maquinas); i++){
        printf("Estou executando no computador %s, meu rank %d de um total de %d processos e i = %d\n",
            computador, meu_codigo, quantidade_de_maquinas, ((numeros_do_core*quantidade_de_maquinas) + meu_codigo));
    // }
 }
 MPI_Finalize(); // Finalização
}

// 0 - 13
// 5 cores
// 0// 0 5 10
// 1// 1 6 11
// 2// 2 7 12
// 3// 3 8 13
// 4// 4 9

// // numeros_para_imprimir = 14
// // numeros_do_core = 2
// // resto = 4
// // quantidade_de_maquinas * (numeros_do_core)
// // quantidade_de_maquinas = 5

// numeros_do_core * quantidade_de_maquinas = 10
// (numeros_do_core * quantidade_de_maquinas)+ meu_codigo = 10

