#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string> 
#include <cstring>
#include <omp.h>
#include <mpi.h>

using namespace std;

const int tamanhoDoBloco = 8;
const int quantidadeDeFrames = 120;

typedef struct TypeFrame
{
    int width = 640;
    int height = 360;
    unsigned char **conteudo = (unsigned char **)malloc(sizeof *conteudo * height);
} TypeFrame;

typedef struct bloco
{
    unsigned char bloco[tamanhoDoBloco][tamanhoDoBloco];
    int x;
    int y;
} bloco;

typedef struct coordenada
{
    int x;
    int y;
} coordenada;

void leVideo(FILE *fp, int width, int heigth, string *str);
int leFrame(FILE *fp, TypeFrame frame, int width, int height);
void pulaCanais(FILE *fp, int width, int height);
bloco criaBloco(int i, int j, unsigned char **frame);
bloco *divideFrameEmBlocos(TypeFrame frame, int quantidadeDeBlocos);
int calculaNivelDeProximidade(bloco a, bloco b);
void comparaBlocos(bloco *frame1, bloco *frame2, coordenada *Rv, coordenada *Ra, int quantidadeDeBlocos, int framePosicao);
string imprimeCorrespondencia(coordenada *Rv, coordenada *Ra, int tamanhoVetor);
void deletaTypeFrame(TypeFrame frame);

int main(int argc, char *argv[])
{
    int width = 640;
    int height = 360;
    printf("Total de Threads Disponíveis: %d \n", omp_get_max_threads());
    string str[quantidadeDeFrames];

    FILE *fp = fopen("../video.yuv", "rb");

    if (fp == NULL)
    {
        printf("Cannot open file");
        return 0;
    }

    // contagem do tempo
    double begin, end;
    begin = omp_get_wtime();
    leVideo(fp, width, height, str);
    end = omp_get_wtime();
    fclose(fp);
    printf("====================================================\n");
    printf("Tempo em segundos execução %f\n", end - begin);
    printf("====================================================\n");

    // impressão do resultado
    printf("Correlação dos blocos nos frames:\n");
    for (int i = 0; i < quantidadeDeFrames - 1; i++)
    {
        printf("_______________________________\nFrame[%d]\n%s_______________________________\n", i, str[i].c_str());
    }

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

void deletaTypeFrame(TypeFrame *frame)
{
    for (int i = 0; i < quantidadeDeFrames; i++)
    {
        for (int j = 0; j < frame[i].height; j++)
        {
            free(frame[i].conteudo[j]);
        }
        free(frame[i].conteudo);
    }
}

void leVideo(FILE *fp, int width, int height, string *str)
{
    int quantidadeDeBlocos = (int)((width) / tamanhoDoBloco) * (int)((height) / tamanhoDoBloco);
    bloco *frameEmBlocosReferencia;
    coordenada Rv[quantidadeDeFrames][quantidadeDeBlocos];
    coordenada Ra[quantidadeDeFrames][quantidadeDeBlocos];
    TypeFrame framesPraComparar[quantidadeDeFrames];

    // printf("Quantidade de Blocos: %d\n", quantidadeDeBlocos);

    for (int w = 0; w < quantidadeDeFrames; w++)
    {
        leFrame(fp, framesPraComparar[w], width, height);
    }

    frameEmBlocosReferencia = divideFrameEmBlocos(framesPraComparar[0], quantidadeDeBlocos); 

//#pragma omp parallel for shared(frameEmBlocosReferencia, quantidadeDeBlocos, Rv, Ra)
    int quantidade_de_maquinas, codigo_core, aux;
    char computador[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(NULL, NULL); // Inicialização
    MPI_Comm_size(MPI_COMM_WORLD, &quantidade_de_maquinas); // Quantos processos envolvidos?
    MPI_Comm_rank(MPI_COMM_WORLD, &codigo_core); // Meu identificador
    MPI_Get_processor_name(computador, &aux);
    quantidadeDeFrames_doCore = quantidadeDeFrames / quantidade_de_maquinas;
    for (int w = quantidadeDeFrames_doCore*codigo_core+1; w < quantidadeDeFrames; w++)
    {
        // Esse daqui é um ponteiro pro cara que eu aloquei dentro do divideFrames em blocos
        //  Então posso dar free nele aqui dentro do for depois
        bloco *frameEmBlocosAtual = divideFrameEmBlocos(framesPraComparar[w], quantidadeDeBlocos); // em determinado momento vai ser null

        // printf("Inicio frame %d. Thread %d\n", w, omp_get_thread_num());
        comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, Rv[w - 1], Ra[w - 1], quantidadeDeBlocos, w);
        str[w - 1] = imprimeCorrespondencia(Rv[w - 1], Ra[w - 1], quantidadeDeBlocos);
        free(frameEmBlocosAtual);
    }
    MPI_Finalize(); // Finalização
    deletaTypeFrame(framesPraComparar);
    free(frameEmBlocosReferencia);
    return;
}

void comparaBlocos(bloco *frame1, bloco *frame2, coordenada *Rv, coordenada *Ra, int quantidadeDeBlocos, int framePosicao)
{
    bool igualdade;
    int nivelDeProximidadeAtual[quantidadeDeBlocos];
    int indiceBlocoMaisParecido[quantidadeDeBlocos];
    int menorNivelDeProximidade[quantidadeDeBlocos];
    int i, j;

    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        indiceBlocoMaisParecido[i] = -1;
        menorNivelDeProximidade[i] = 1000000;
    }

#pragma for collapse(2) nowait schedule(static)
    for (i = 0; i < quantidadeDeBlocos; i++) // frame1
    {
        for (j = 0; j <= quantidadeDeBlocos; j++) // frame2
        {
            if (j < quantidadeDeBlocos)
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

string imprimeCorrespondencia(coordenada *Rv, coordenada *Ra, int tamanhoVetor)
{
    string retorno = "";
    for (int i = 0; i < tamanhoVetor; i++)
    {
        string Rvx = to_string(Rv[i].x);
        string Rvy = to_string(Rv[i].y);
        string Rax = to_string(Ra[i].y);
        string Ray = to_string(Ra[i].x);

        retorno += "(" + Rvx + "," + Rvy + ") => (" + Rax + "," + Ray + ")\n";
    }
    return retorno;
}


int calculaNivelDeProximidade(bloco a, bloco b)
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

bloco *divideFrameEmBlocos(TypeFrame frame, int quantidadeDeBlocos)
{

    bloco *frameEmBlocos = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco)); 

#pragma parallel omp for collapse(2) shared(frameEmBlocos) // pode não compensar dependendo da máquina
    for (int i = 0; i < frame.height; i += tamanhoDoBloco)
    {
        for (int j = 0; j < frame.width; j += tamanhoDoBloco)
        {
            bloco blocoAtual = criaBloco(i, j, frame.conteudo);
            int novoJ = (int)j / tamanhoDoBloco;
            int novoI = (int)i / tamanhoDoBloco;
            int indice = novoI * (int)(frame.width / tamanhoDoBloco) + novoJ;
            frameEmBlocos[indice] = blocoAtual;
        }
    }
    return frameEmBlocos;
}

bloco criaBloco(int i, int j, unsigned char **frame)
{
    bloco blocoRetorno;
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
