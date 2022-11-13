#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int manipula(int a); // Função que vai fazer algum tipo de manipulação do dado

int main(int argc, char **argv)
{
    int quantidade_de_maquinas, meu_codigo, aux;

    int numerosArray[6] = {1, 2, 3, 82, 91, 16}; // Vetor de entrada
    int numerosArrayResposta[6] = {};            // Vetor onde eu quero armazenar a saída

    // Precisa usar a sintaxe de ponteiro. Se usar array NÃO FUNCIONA e não me perguntem porquê.
    int *elementosPorProcesso = (int *)malloc(sizeof(int) * quantidade_de_maquinas); // Quantos processos vão pro processador           // array describing how many elements to send to each process
    int *aPartirDoIndice = (int *)malloc(sizeof(int) * quantidade_de_maquinas);
    aPartirDoIndice[0] = 0;

    // Início do MPI
    MPI_Init(&argc, &argv);                                 // Inicialização do MPI
    MPI_Comm_size(MPI_COMM_WORLD, &quantidade_de_maquinas); // Quantos processos envolvidos?
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_codigo);             // Meu identificador

    int processosResto = 6 % quantidade_de_maquinas; // Quantidade de resto que é gerada pela quantidade de máquina.
    // Essa variável precisa ficar aqui porque senão quantidade_de_maquinas não tá definido

    // Cria vetor de quantidade de elementosPorProcesso
    for (int i = 0; i < quantidade_de_maquinas; i++)
    {
        elementosPorProcesso[i] = 6 / quantidade_de_maquinas;
        if (processosResto > 0)
        { // Se eu tiver processos com resto, toco pra alguma máquina
            elementosPorProcesso[i]++;
            processosResto--;
        }
    }

    // Cria vetor de deslocamento
    for (int i = 1; i < quantidade_de_maquinas; i++)
    {
        aPartirDoIndice[i] = aPartirDoIndice[i - 1] + elementosPorProcesso[i - 1];
    }

    int tamanhoChunck = elementosPorProcesso[meu_codigo];

    printf("Chuncks (quantidade de items do array por proceso) no processo %d-> %d\n", meu_codigo, tamanhoChunck);

    int numerosAux[tamanhoChunck]; // Vetor auxiliar
    MPI_Scatterv(
        numerosArray,         // Vetor que tem as informações que eu quero mandar pra todo mundo
        elementosPorProcesso, // Quantas posições desse array eu quero mandar pra cada processo
        aPartirDoIndice, //A partir de qual índice do arquivo original eu quero mandar essas posições
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

    MPI_Gatherv(
        numerosAux,           // Variável que armazena as "fatias" que eu quero armazenar no meu array de retorno
        tamanhoChunck,        // Quantidades de variáveis que eu tenho na minha fatia e que vou colocar no resultado
        MPI_INT,              // Tipo dos elementos da fatia
        numerosArrayResposta, // Variavel onde eu vou juntar todas as fatias
        elementosPorProcesso, // quantidades de elementos dessa fatia que eu quero colocar na variável que armazena todas as fatias
        aPartirDoIndice,      // A partir de qual índice eu coloco as fatias do array interno no array resposta
        MPI_INT,              // Tipo dos elementos que da variável que armazena as fatias
        0,                    // Processo pra onde todas as fatias são enviadas
        MPI_COMM_WORLD);


    if (meu_codigo == 0)
    {
        //Print do array numerosPorProcesso
        for (int i = 0; i < quantidade_de_maquinas; i++)
        {
            printf("numerosPorProcesso[%d] = %d\n", i, elementosPorProcesso[i]);
        }
        //Print do array de deslocamento
        for (int i = 0; i < quantidade_de_maquinas; i++)
        {
            printf("deslocamento[%d] = %d\n", i, aPartirDoIndice[i]);
        }
        //Print do resultado final
        for (int i = 0; i < 6; i++)
        {
            printf("resultado[%d] = %d\n", i, numerosArrayResposta[i]);
        }
        printf("Processo 0\n");
    }
    MPI_Finalize(); // Finalização
}

int manipula(int a)
{
    return a * 100;
}
