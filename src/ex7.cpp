#include <stdio.h>
#include <cstdlib>
#include <iostream> //pode dar problema
#include <string>
#include <cstring>
#include <omp.h>
#include <mpi.h>

using namespace std;

const int tamanhoDoBloco = 5;
const int quantidadeDeBlocosPorFrame = 6;
const int quantidadeDeFrames = 6;
typedef struct bloco
{
    unsigned char blocoDeVerdade[tamanhoDoBloco][tamanhoDoBloco];
    int x;
    int y;
} bloco;

typedef struct correspondencia
{
    int xReferencia;
    int yReferencia;
    int xAtual;
    int yAtual;

} correspondencia;

correspondencia manipula(bloco a); // Função que vai fazer algum tipo de manipulação do dado

int main(int argc, char **argv)
{
    int quantidade_de_maquinas, meu_codigo;

    bloco framesDoVideo[quantidadeDeFrames][quantidadeDeBlocosPorFrame] = {}; // Vetor de entrada
    correspondencia correspondenciaFinal[quantidadeDeFrames][quantidadeDeBlocosPorFrame];
    for (int i = 0; i < quantidadeDeFrames; i++)
    {
        for (int j = 0; j < quantidadeDeBlocosPorFrame; j++)
        {
            framesDoVideo[i][j].x = 10; // rand() % 100;
            framesDoVideo[i][j].y = 20; // rand() % 100;
            // framesDoVideo[i][j].blocoDeVerdade[0][0] = 'a';
            // framesDoVideo[i][j].blocoDeVerdade[0][1] = 'b';
            // framesDoVideo[i][j].blocoDeVerdade[1][0] = 'c';
            // framesDoVideo[i][j].blocoDeVerdade[1][1] = 'd';
        }
    }

    // Precisa usar a sintaxe de ponteiro. Se usar array NÃO FUNCIONA e não me perguntem porquê.
    int *elementosPorProcesso = (int *)malloc(sizeof(int) * quantidade_de_maquinas); // Quantos processos vão pro processador           // array describing how many elements to send to each process
    int *aPartirDoIndice = (int *)malloc(sizeof(int) * quantidade_de_maquinas);
    aPartirDoIndice[0] = 0;

    // Início do MPI
    MPI_Init(&argc, &argv);                                 // Inicialização do MPI
    MPI_Comm_size(MPI_COMM_WORLD, &quantidade_de_maquinas); // Quantos processos envolvidos?
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_codigo);             // Meu identificador

    // Definição do tipo MPI_Bloco, correspondente à struct bloco
    MPI_Datatype MPI_BLOCO;
    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_INT, MPI_INT};
    int quantidadesDeVariaveis[3] = {tamanhoDoBloco * tamanhoDoBloco, 1, 1};
    MPI_Aint ondeAsVariaveisIniciam[3];
    bloco blocoDisp; // bloco pra calcular o displacement

    MPI_Aint base_address;
    MPI_Get_address(&blocoDisp, &base_address);
    MPI_Get_address(&blocoDisp.blocoDeVerdade[0][0], &ondeAsVariaveisIniciam[0]);
    MPI_Get_address(&blocoDisp.x, &ondeAsVariaveisIniciam[1]);
    MPI_Get_address(&blocoDisp.y, &ondeAsVariaveisIniciam[2]);

    ondeAsVariaveisIniciam[0] = MPI_Aint_diff(ondeAsVariaveisIniciam[0], base_address);
    ondeAsVariaveisIniciam[1] = MPI_Aint_diff(ondeAsVariaveisIniciam[1], base_address);
    ondeAsVariaveisIniciam[2] = MPI_Aint_diff(ondeAsVariaveisIniciam[2], base_address);

    MPI_Type_create_struct(3, quantidadesDeVariaveis, ondeAsVariaveisIniciam, types, &MPI_BLOCO);
    MPI_Type_commit(&MPI_BLOCO);

    // Definição da Struct Correspondencia
    MPI_Datatype MPI_CORRESPONDENCIA;
    MPI_Datatype types2[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    int quantidadesDeVariaveis2[4] = {1, 1, 1, 1};
    // unsiged char é 1 byte e int é 4 bytes
    MPI_Aint ondeAsVariaveisIniciam2[4] = {0, 4, 8, 12};
    MPI_Type_create_struct(4, quantidadesDeVariaveis2, ondeAsVariaveisIniciam2, types2, &MPI_CORRESPONDENCIA);
    MPI_Type_commit(&MPI_CORRESPONDENCIA);

    int processosResto = quantidadeDeFrames % quantidade_de_maquinas; // Quantidade de resto que é gerada pela quantidade de máquina.
    // Essa variável precisa ficar aqui porque senão quantidade_de_maquinas não tá definido

    // Cria vetor de quantidade de elementosPorProcesso
    for (int i = 0; i < quantidade_de_maquinas; i++)
    {
        elementosPorProcesso[i] = quantidadeDeFrames / quantidade_de_maquinas;
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

    // printf("Chuncks (quantidade de items do array por proceso) no processo %d-> %d\n", meu_codigo, tamanhoChunck);

    bloco blocosDoFrame[tamanhoChunck];                     // Vetor auxiliar
    correspondencia correspondenciasDoFrame[tamanhoChunck]; // Vetor auxiliar
    MPI_Scatterv(
        framesDoVideo[meu_codigo], // Vetor que tem as informações que eu quero mandar pra todo mundo
        elementosPorProcesso,      // Quantas posições desse array eu quero mandar pra cada processo
        aPartirDoIndice,           // A partir de qual índice do arquivo original eu quero mandar essas posições
        MPI_BLOCO,                 // Tipo do array que quero mandar pra todo mundo
        blocosDoFrame,             // Variável que representa a "fatia" do meu array original que cada processo vai receber
        tamanhoChunck,             // Tamanho da "fatia"
        MPI_BLOCO,                 // Tipo dos elementos da "fatia"
        0,                         // Processo de origem
        MPI_COMM_WORLD);

    // // processamento
    // // em cada processo, eu tenho uma fatia de tamanhoChunck de elementos do meu array.
    // // Essa fatia é um array que vai começar em zero e vai até tamanhoChunck-1

    for (int i = 0; i < tamanhoChunck; i++)
    {
        correspondenciasDoFrame[i] = manipula(blocosDoFrame[i]);

        // printf("resultado[%d] = (%d,%d)=>(%d,%d)\n", i, correspondenciasDoFrame[i].xAtual, correspondenciasDoFrame[i].yAtual, correspondenciasDoFrame[i].xReferencia, correspondenciasDoFrame[i].yReferencia);
    }

    MPI_Gatherv(
        correspondenciasDoFrame,    // Variável que armazena as "fatias" que eu quero armazenar no meu array de retorno
        tamanhoChunck,                // Quantidades de variáveis que eu tenho na minha fatia e que vou colocar no resultado
        MPI_CORRESPONDENCIA,          // Tipo dos elementos da fatia
        correspondenciaFinal[meu_codigo], // Variavel onde eu vou juntar todas as fatias
        elementosPorProcesso,         // quantidades de elementos dessa fatia que eu quero colocar na variável que armazena todas as fatias
        aPartirDoIndice,              // A partir de qual índice eu coloco as fatias do array BLOCOerno no array resposta
        MPI_CORRESPONDENCIA,          // Tipo dos elementos que da variável que armazena as fatias
        0,                            // Processo pra onde todas as fatias são enviadas
        MPI_COMM_WORLD);

    if (meu_codigo == 0)
    {
        // Print do array numerosPorProcesso
        for (int i = 0; i < quantidade_de_maquinas; i++)
        {
            // printf("numerosPorProcesso[%d] = %d\n", i, elementosPorProcesso[i]);
        }
        // Print do array de deslocamento
        for (int i = 0; i < quantidade_de_maquinas; i++)
        {
            // printf("deslocamento[%d] = %d\n", i, aPartirDoIndice[i]);
        }
        // Print do resultado final
        for (int i = 0; i < 6; i++)
        {
            printf("resultado[%d] = (%d,%d)=>(%d,%d)\n", i, correspondenciaFinal[i][0].xAtual, correspondenciaFinal[i][0].yAtual, correspondenciaFinal[i][0].xReferencia, correspondenciaFinal[i][0].yReferencia);
        }
        printf("Fim\n");
    }
    MPI_Finalize(); // Finalização
    free(elementosPorProcesso);
    free(aPartirDoIndice);
}

correspondencia manipula(bloco a)
{
    correspondencia retorno;
    retorno.xAtual = a.x;
    retorno.yAtual = a.y;
    retorno.xReferencia = a.x + a.y;
    retorno.yReferencia = a.x - a.y;
    return retorno;
}
