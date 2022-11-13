#include <stdio.h>
#include <mpi.h>

int manipula(int a); // Função que vai fazer algum tipo de manipulação do dado

int main(int argc, char **argv)
{
    int quantidade_de_maquinas, meu_codigo, aux;

    // Por enquanto, só funciona com um tamanho que dê pra dividir sem resto entre os processos
    int numerosArray[6] = {1, 2, 3, 82, 91, 16}; // Vetor de entrada
    int numerosArrayResposta[6] = {};            // Vetor onde eu quero armazenar a saída

    MPI_Init(&argc, &argv);                                 // Inicialização do MPI
    MPI_Comm_size(MPI_COMM_WORLD, &quantidade_de_maquinas); // Quantos processos envolvidos?
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_codigo);             // Meu identificador

    int tamanhoChunck = 6 / quantidade_de_maquinas; // Quantos números vão pra cada processo

    printf("Chuncks (quantidade de items do array por proceso)-> %d\n", tamanhoChunck);

    int numerosAux[tamanhoChunck]; // Vetor auxiliar
    MPI_Scatter(
        numerosArray,  // Vetor que tem as informações que eu quero mandar pra todo mundo
        tamanhoChunck, // Quantas posições desse array eu quero mantar pra todo mundo
        MPI_INT,       // Tipo do array que quero mandar pra todo mundo
        numerosAux,    // Variável que representa a "fatia" do meu array original que cada processo vai receber
        tamanhoChunck, // Tamanho da "fatia"
        MPI_INT,       // Tipo dos elementos da "fatia"
        0,             // Processo de origem
        MPI_COMM_WORLD);

    // processamento
    // em cada processo, eu tenho uma fatia de tamanhoChunck de elementos do meu array.
    // Essa fatia é um array que vai começar em zero e vai até tamanhoChunck-1
    for (int i = 0; i < tamanhoChunck; i++)
    {
        printf("editando numerosAux[%d] do processo %d que tem o valor %d\n", i, meu_codigo, numerosAux[i]);
        numerosAux[i] = manipula(numerosAux[i]);
    }

    MPI_Gather(
        numerosAux,           // Variável que armazena as "fatias" que eu quero armazenar no meu array de retorno
        tamanhoChunck,        // Quantidades de variáveis que eu tenho na minha fatia e que vou colocar no resultado
        MPI_INT,              // Tipo dos elementos da fatia
        numerosArrayResposta, // Variavel onde eu vou juntar todas as fatias
        tamanhoChunck,        // quantidades de elementos dessa fatia que eu quero colocar na variável que armazena todas as fatias
        MPI_INT,              // Tipo dos elementos que da variável que armazena as fatias
        0,                    // Processo pra onde todas as fatias são enviadas
        MPI_COMM_WORLD);

    // Assinaturas do método Gather e Scatter

    //  MPI_Scatter(
    //     void* send_data,
    //     int send_count,
    //     MPI_Datatype send_datatype,
    //     void* recv_data,
    //     int recv_count,
    //     MPI_Datatype recv_datatype,
    //     int root,
    //     MPI_Comm communicator)
    // MPI_Gather(
    //     void* send_data,
    //     int send_count,
    //     MPI_Datatype send_datatype,
    //     void* recv_data,
    //     int recv_count,
    //     MPI_Datatype recv_datatype,
    //     int root,
    //     MPI_Comm communicator)

    if (meu_codigo == 0)
    {
        for (int i = 0; i < 6; i++)
        {
            printf("numeros[%d] = %d\n", i, numerosArrayResposta[i]);
        }
    }
    MPI_Finalize(); // Finalização
}

int manipula(int a)
{
    return a * 100;
}
