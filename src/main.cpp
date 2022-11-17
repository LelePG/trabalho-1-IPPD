#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <omp.h>
#include <mpi.h>

using namespace std;

const int tamanhoDoBloco = 8;
const int quantidadeDeFrames = 5;
const int quantidadeDeFramesSemReferencia = quantidadeDeFrames - 1;
const int quantidadeDeBlocosPorFrame = (int)(640 / tamanhoDoBloco) * (int)(360 / tamanhoDoBloco);
const int quantidadeTotal = quantidadeDeBlocosPorFrame * quantidadeDeFramesSemReferencia;

// Tamanho do bloco - 8
// quantidade de frames = 5
//  quantidade de framesSemReferencia = 4
//  qiamtodade De blocos por frame = 3600
//  quantidade total (de blocos ) = 14400
typedef struct TypeFrame
{
    int width = 640;
    int height = 360;
    unsigned char **conteudo = (unsigned char **)malloc(sizeof *conteudo * height);
} TypeFrame;

typedef struct TypeBloco
{
    unsigned char bloco[tamanhoDoBloco][tamanhoDoBloco];
    int x;
    int y;
    int frameId;
} TypeBloco;

typedef struct TypeCorrespondencia
{
    int xReferencia;
    int yReferencia;
    int xAtual;
    int yAtual;
    int frameId;

} TypeCorrespondencia;

typedef struct TypeCoordenada
{
    int x;
    int y;
} TypeCoordenada;

int leFrame(FILE *fp, TypeFrame frame, int width, int height);
void pulaCanais(FILE *fp, int width, int height);
void manipulaVideo(TypeBloco *frameReferencia, TypeFrame *framesDoVideo, int width, int height, TypeCorrespondencia **correspondencias);
TypeBloco criaBloco(int i, int j, unsigned char **frame);
TypeBloco *divideFrameEmBlocos(TypeFrame frame, int idFrame, int quantidadeDeBlocosPorFrame);
int calculaNivelDeProximidade(TypeBloco a, TypeBloco b);
void comparaBlocos(TypeBloco *frame1, TypeBloco *frame2, TypeCoordenada *Rv, TypeCoordenada *Ra, int quantidadeDeBlocosPorFrame, int framePosicao);
void deletaArrayDeTypeFrame(TypeFrame *frame, int tamanho);
char *imprimeCorrespondencia(TypeCorrespondencia *correspondencias, int tamanhoVetor);
void copiaCorrespondencia(TypeCorrespondencia *correspondencia, TypeCoordenada *Rv, TypeCoordenada *Ra, int tamanho);
void imprimeCorrespondenciaTestes(TypeCorrespondencia **correspondencias, int tamanho);
void manipulaVideoDEV(TypeBloco *frameReferencia, TypeFrame *framesDoVideo, int width, int height, TypeCorrespondencia *correspondencias);

int main(int argc, char *argv[])
{
    int width = 640;
    int height = 360;
    // int quantidadeDeBlocosPorFrame = (int)((width) / tamanhoDoBloco) * (int)((height) / tamanhoDoBloco);
    // int quantidadeDeFramesSemReferencia = quantidadeDeFrames - 1;
    // int quantidadeTotal = quantidadeDeFBlocos* quantidadeDeFrames;

    printf("Total de Threads Disponíveis: %d \n", omp_get_max_threads());

    // Criação das variáveis relevantes
    TypeCorrespondencia *correspondencias = (TypeCorrespondencia *)malloc(quantidadeTotal * sizeof(TypeCorrespondencia));
    TypeBloco *frameEmBlocosReferencia;
    TypeFrame *framesDoVideo = (TypeFrame *)malloc(quantidadeDeFrames * sizeof(TypeFrame));
    TypeBloco *framesDoVideoEmBlocos = (TypeBloco *)malloc(quantidadeTotal * sizeof(TypeBloco));

    FILE *fp = fopen("../video.yuv", "rb");

    if (fp == NULL)
    {
        printf("Cannot open file");
        return 0;
    }

    ;

    // Fazer a leitura do vídeo pra usar o MPI posteriormente
    // Leitura do frameDeReferencia e transformação do frame de Referencia pra blocos
    TypeFrame frameReferencia;
    leFrame(fp, frameReferencia, width, height);
    frameEmBlocosReferencia = divideFrameEmBlocos(frameReferencia, -1, quantidadeDeBlocosPorFrame);

    // Leitura do resto do vídeo (considerando que o primeiro frame foi lido)
    for (int w = 0; w < quantidadeDeFramesSemReferencia; w++)
    {
        leFrame(fp, framesDoVideo[w], width, height);
        // Converter o framesDoVideo[w] pra um array de frames e armazenar isso em framesDoVideoEmBlocos
        TypeBloco *frameEmBlocosAtual = divideFrameEmBlocos(framesDoVideo[w], w, quantidadeDeBlocosPorFrame); // em determinado momento vai ser null
        // adicionar isso no framesDoVideoEmBlocos
        int deslocamento = w * quantidadeDeBlocosPorFrame;
        printf("%d - ", deslocamento);
        //frameEmBlocosAtual.
        // Copia o frameEmBlocosAtual pra dentro do framesDoVideoEmBlocos

        for (int z = 0; z < quantidadeDeBlocosPorFrame; z++)
        {
            framesDoVideoEmBlocos[z + deslocamento] = frameEmBlocosAtual[z];
            printf("%d - ", (z + deslocamento));
        }
    }

    fclose(fp);
    // Nesse ponto do código, framesDoVideo é uma variável que contém o código puro de todos os frames do vídeo, e todo o vídeo foi lido.

    // contagem do tempo
    // double begin, end;
    // begin = omp_get_wtime();
    manipulaVideoDEV(frameEmBlocosReferencia, framesDoVideo, width, height, correspondencias);
    // end = omp_get_wtime();
    // printf("====================================================\n");
    // printf("Tempo em segundos execução %f\n", end - begin);
    // printf("====================================================\n");

    free(frameEmBlocosReferencia);
    deletaArrayDeTypeFrame(framesDoVideo, quantidadeDeFramesSemReferencia);

    return 0;
}

int leFrame(FILE *fp, TypeFrame frame, int width, int height)
{
    for (int i = 0; i < frame.height; i++)
    {
        frame.conteudo[i] = (unsigned char *)malloc(sizeof *frame.conteudo[i] * frame.width); // aloca a linha
        int bytesLidos = fread(frame.conteudo[i], sizeof(unsigned char), frame.width, fp);
        if (bytesLidos < sizeof(unsigned char))
        {
            return 0;
        }
    }
    pulaCanais(fp, frame.width, frame.height);
    return 1;
}

void pulaCanais(FILE *fp, int width, int height)
{
    unsigned char *aux = (unsigned char *)malloc(sizeof *aux * width * height);
    fread(aux, sizeof(unsigned char), width * height / 2, fp);
    free(aux);
}

void deletaArrayDeTypeFrame(TypeFrame *frame, int tamanho)
{
    for (int i = 0; i < tamanho; i++)
    {
        free(frame[i].conteudo);
    }
}

void manipulaVideoDEV(TypeBloco *frameEmBlocosReferencia, TypeFrame *framesDoVideo, int width, int height, TypeCorrespondencia *correspondencias)
{
    // Tenho o meu frame de referencia em blocos, um array com todos os frames que precisamos codificar pra blocos e o array de correspondencias
    int quantidadeDeBlocosPorFrame = (int)((width) / tamanhoDoBloco) * (int)((height) / tamanhoDoBloco);

    TypeCoordenada Rv[quantidadeTotal];
    TypeCoordenada Ra[quantidadeTotal];

    for (int w = 0; w < quantidadeDeFrames - 1; w++)
    {
    }
}

// void manipulaVideo(TypeBloco *frameEmBlocosReferencia, TypeFrame *framesDoVideo, int width, int height, TypeCorrespondencia **correspondencias)
// {
//     int quantidadeDeBlocosPorFrame = (int)((width) / tamanhoDoBloco) * (int)((height) / tamanhoDoBloco);
//     TypeCoordenada Rv[quantidadeDeFrames][quantidadeDeBlocosPorFrame];
//     TypeCoordenada Ra[quantidadeDeFrames][quantidadeDeBlocosPorFrame];

//     // correspondencias= (TypeCorrespondencia **)malloc(quantidadeDeFrames * sizeof(TypeCorrespondencia *));
//     // for (int i = 0; i < quantidadeDeBlocosPorFrame; i++)
//     // {
//     //     correspondencias[i] = (TypeCorrespondencia *)malloc(quantidadeDeBlocosPorFrame * sizeof(TypeCorrespondencia));
//     // }

//     // MPI_Init(NULL, NULL);

//     // printf("Quantidade de Blocos: %d\n", quantidadeDeBlocosPorFrame);
//     // AQUI VAI O MPI
//     //  #pragma omp parallel for shared(frameEmBlocosReferencia, quantidadeDeBlocosPorFrame, correspondencias, Rv, Ra)
//     for (int w = 0; w < quantidadeDeFrames - 1; w++)
//     {
//         // Esse daqui é um ponteiro pro cara que eu aloquei dentro do divideFrames em blocos
//         //  Então posso dar free nele aqui dentro do for depois
//         TypeBloco *frameEmBlocosAtual = divideFrameEmBlocos(framesDoVideo[w],w, quantidadeDeBlocosPorFrame); // em determinado momento vai ser null
//         // printf("Inicio frame %d. Thread %d\n", w, omp_get_thread_num());
//         comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, Rv[w], Ra[w], quantidadeDeBlocosPorFrame, w);
//         // str[w] = imprimeCorrespondencia(Rv[w], Ra[w], quantidadeDeBlocosPorFrame);

//         copiaCorrespondencia(correspondencias[w], Rv[w], Ra[w], quantidadeDeBlocosPorFrame);
//         // printf("%d\n", correspondencias[0][0].yAtual);
//         // exit(0);
//         free(frameEmBlocosAtual);
//     }

//     // // impressão do resultado
//     printf("Correlação dos blocos nos frames:\n");
//     for (int i = 0; i < quantidadeDeFrames - 1; i++)
//     {
//         printf("_______________________________\nFrame[%d]\n%s_______________________________\n", i, imprimeCorrespondencia(correspondencias[i], quantidadeDeBlocosPorFrame));
//     }
//     // MPI_Finalize();
//     return;
// }

void copiaCorrespondencia(TypeCorrespondencia *correspondencia, TypeCoordenada *Rv, TypeCoordenada *Ra, int tamanho)
{
    for (int i = 0; i < tamanho; i++)
    {
        correspondencia[i].xReferencia = Rv[i].x;
        correspondencia[i].yReferencia = Rv[i].y;
        correspondencia[i].yAtual = Ra[i].y;
        correspondencia[i].xAtual = Ra[i].x;
    }
}

void comparaBlocos(TypeBloco *frame1, TypeBloco *frame2, TypeCoordenada *Rv, TypeCoordenada *Ra, int quantidadeDeBlocosPorFrame, int framePosicao)
{
    bool igualdade;
    int nivelDeProximidadeAtual[quantidadeDeBlocosPorFrame];
    int indiceBlocoMaisParecido[quantidadeDeBlocosPorFrame];
    int menorNivelDeProximidade[quantidadeDeBlocosPorFrame];
    int i, j;

    for (int i = 0; i < quantidadeDeBlocosPorFrame; i++)
    {
        indiceBlocoMaisParecido[i] = -1;
        menorNivelDeProximidade[i] = 1000000;
    }

#pragma for collapse(2) nowait schedule(static)
    for (i = 0; i < quantidadeDeBlocosPorFrame; i++) // frame1
    {
        for (j = 0; j <= quantidadeDeBlocosPorFrame; j++) // frame2
        {
            if (j < quantidadeDeBlocosPorFrame)
            {
                nivelDeProximidadeAtual[i] = calculaNivelDeProximidade(frame1[i], frame2[j]);
                if (nivelDeProximidadeAtual[i] < menorNivelDeProximidade[i])
                {
                    menorNivelDeProximidade[i] = nivelDeProximidadeAtual[i];
                    indiceBlocoMaisParecido[i] = j;
                }
            }
            else
            {
                Rv[i].x = frame1[i].x;
                Rv[i].y = frame1[i].y;
                Ra[i].x = frame2[indiceBlocoMaisParecido[i]].x;
                Ra[i].y = frame2[indiceBlocoMaisParecido[i]].y;
            }
        }
    }
    return;
}

char *imprimeCorrespondencia(TypeCorrespondencia *correspondencias, int tamanhoVetor)
{
    string retorno = "";

    for (int i = 0; i < tamanhoVetor; i++)
    {
        // printf("%d\n",i);
        // printf("%d\n", (int) correspondencias[i].xReferencia);
        string Rvx = to_string(correspondencias[i].xReferencia);
        string Rvy = to_string(correspondencias[i].yReferencia);
        string Rax = to_string(correspondencias[i].yAtual);
        string Ray = to_string(correspondencias[i].xAtual);

        retorno += "(" + Rvx + "," + Rvy + ") => (" + Rax + "," + Ray + ")\n";
    }
    return (char *)retorno.c_str();
}

int calculaNivelDeProximidade(TypeBloco a, TypeBloco b)
{
    float diff = 0;
    for (int i = 0; i < tamanhoDoBloco; i++)
    {
        for (int j = 0; j < tamanhoDoBloco; j++)
        {
            diff += abs((int)a.bloco[i][j] - (int)b.bloco[i][j]);
        }
    }
    return diff;
}

TypeBloco *divideFrameEmBlocos(TypeFrame frame, int idFrame, int quantidadeDeBlocosPorFrame)
{

    TypeBloco *frameEmBlocos = (TypeBloco *)malloc(quantidadeDeBlocosPorFrame * sizeof(TypeBloco));

#pragma parallel omp for collapse(2) shared(frameEmBlocos) // pode não compensar dependendo da máquina
    for (int i = 0; i < frame.height; i += tamanhoDoBloco)
    {
        for (int j = 0; j < frame.width; j += tamanhoDoBloco)
        {
            TypeBloco blocoAtual = criaBloco(i, j, frame.conteudo);
            blocoAtual.frameId = idFrame;
            int novoJ = (int)j / tamanhoDoBloco;
            int novoI = (int)i / tamanhoDoBloco;
            int indice = novoI * (int)(frame.width / tamanhoDoBloco) + novoJ;
            frameEmBlocos[indice] = blocoAtual;
        }
    }
    return frameEmBlocos;
}

TypeBloco criaBloco(int i, int j, unsigned char **frame)
{
    TypeBloco blocoRetorno;
    blocoRetorno.x = i;
    blocoRetorno.y = j;

#pragma parallel omp for collapse(2) shared(blocoRetorno) // pode não compensar dependendo da máquina
    for (int k = 0; k < tamanhoDoBloco; k++)
    {
        for (int l = 0; l < tamanhoDoBloco; l++)
        {
            blocoRetorno.bloco[k][l] = frame[i + k][j + l];
        }
    }
    return blocoRetorno;
}