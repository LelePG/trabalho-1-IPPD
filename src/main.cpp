#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string> //necessário para usar strings
#include <cstring>
#include <omp.h>

using namespace std;

const int tamanhoDoBloco = 8;
const int quantidadeDeFrames = 12;

typedef struct TypeFrame
{
    int width = 640;
    int height = 360;
    unsigned char** conteudo = (unsigned char **)malloc(sizeof *conteudo*height);
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
bloco* divideFrameEmBlocos(TypeFrame frame, int quantidadeDeBlocos);
int calculaNivelDeProximidade(bloco a, bloco b);
void comparaBlocos(bloco *frame1, bloco *frame2, coordenada *Rv ,coordenada *Ra, int quantidadeDeBlocos, int framePosicao);
string imprimeCorrespondencia(coordenada *Rv, coordenada *Ra, int tamanhoVetor);
void imprimeBloco(bloco b);
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

    //contagem do tempo
    double begin, end;
	begin = omp_get_wtime();
    leVideo(fp, width, height, str);
    end = omp_get_wtime();
    printf("====================================================\n");
	printf("Tempo em segundos execução da compressão %f\n", end-begin); 
    printf("====================================================\n");
    
    //impressão do resulta
    printf("Frames compactados:\n");
    for(int i =0; i <quantidadeDeFrames-1;i++){
    printf("_______________________________\nFrame[%d]\n%s_______________________________\n",i,str[i].c_str());

    }
    fclose(fp); 

    return 0;
}

int leFrame(FILE *fp, TypeFrame frame, int width, int height)
{
    for (int i = 0; i < frame.height; i++)
    {                                                                                         
        frame.conteudo[i] = (unsigned char *)malloc(sizeof *frame.conteudo[i] * frame.width); //aloca a linha
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
void deletaTypeFrame(TypeFrame *frame){
    for(int i = 0; i< quantidadeDeFrames; i++){
        for(int j = 0; j< frame[i].height; j++){
            free(frame[i].conteudo[j]);
        }
        free(frame[i].conteudo);
    }
}

void leVideo(FILE *fp, int width, int height, string *str)
{
    int quantidadeDeBlocos = (int)((width)/tamanhoDoBloco) * (int)((height)/tamanhoDoBloco);

    bloco *frameEmBlocosReferencia = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    bloco *frameEmBlocosAtual= (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));

    //printf("Quantidade de Blocos: %d\n", quantidadeDeBlocos);

    TypeFrame framesPraComparar[quantidadeDeFrames];
    for (int w = 0;  w < quantidadeDeFrames  ; w++)
    {
        leFrame(fp, framesPraComparar[w], width, height);
    }
    
    //divideFrameEmBlocos(framesPraComparar[0],frameEmBlocosReferencia, quantidadeDeBlocos);
    frameEmBlocosReferencia = divideFrameEmBlocos(framesPraComparar[0], quantidadeDeBlocos); //em determinado momento vai ser null
            
     coordenada Rv[quantidadeDeFrames][quantidadeDeBlocos];
     coordenada Ra[quantidadeDeFrames][quantidadeDeBlocos];

	#pragma omp parallel for shared(frameEmBlocosReferencia,quantidadeDeBlocos, Rv, Ra)
		for (int w = 1;  w < quantidadeDeFrames ; w++) 
        {
            //divideFrameEmBlocos(framesPraComparar[w], frameEmBlocosAtual, quantidadeDeBlocos); 
             frameEmBlocosAtual = divideFrameEmBlocos(framesPraComparar[w], quantidadeDeBlocos); //em determinado momento vai ser null
            
            //printf("Inicio frame %d. Thread %d\n", w, omp_get_thread_num());
			comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, Rv[w-1],Ra[w-1], quantidadeDeBlocos, w); 
            str[w-1] = imprimeCorrespondencia(Rv[w-1],Ra[w-1], quantidadeDeBlocos);
        }
    deletaTypeFrame(framesPraComparar);
    free(frameEmBlocosAtual);
    free(frameEmBlocosReferencia);
    return ;
}

void comparaBlocos(bloco *frame1, bloco *frame2, coordenada *Rv ,coordenada *Ra, int quantidadeDeBlocos, int framePosicao)
{
    bool igualdade;
    int nivelDeProximidadeAtual[quantidadeDeBlocos];
    int indiceBlocoMaisParecido[quantidadeDeBlocos];
    int menorNivelDeProximidade[quantidadeDeBlocos];

    for (int  i = 0; i < quantidadeDeBlocos;i++)
    {
        indiceBlocoMaisParecido[i] = -1;
        menorNivelDeProximidade[i] = 1000000;
    }
    int i,j;
	
        #pragma for collapse(2) nowait schedule(static)
        for ( i = 0; i < quantidadeDeBlocos; i++) //frame1
        {         
                for (j = 0; j <= quantidadeDeBlocos; j++) //frame2
                { 
                        if(j < quantidadeDeBlocos)
                        {
                            nivelDeProximidadeAtual[i] = calculaNivelDeProximidade(frame1[i], frame2[j]);
                            if (nivelDeProximidadeAtual[i] < menorNivelDeProximidade[i] && nivelDeProximidadeAtual[i]>0)
                            {
                                menorNivelDeProximidade[i] = nivelDeProximidadeAtual[i];
                                indiceBlocoMaisParecido[i] = j;
                            }
                        }else{
                            Rv[i].x = frame1[i].x;
                            Rv[i].y = frame1[i].y; 
                            Ra[i].x = frame2[indiceBlocoMaisParecido[i]].x;
                            Ra[i].y = frame2[indiceBlocoMaisParecido[i]].y;   
                        }
                }      
        }
    
    return ; 
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

void imprimeBloco(bloco b)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf(" %d ", b.bloco[i][j]);
        }
        printf("\n");
    }
    printf("----------------------------------------------------------------\n");
}

int calculaNivelDeProximidade(bloco a, bloco b)
{
    float diff = 0;
    for (int i = 0; i < tamanhoDoBloco; i++)
    {
        for (int j = 0; j < tamanhoDoBloco; j++)
        {   
            diff += abs( (int)a.bloco[i][j] - (int)b.bloco[i][j]);
        }
    }
    return diff;
}

bloco *divideFrameEmBlocos(TypeFrame frame, int quantidadeDeBlocos)
{
    bloco *frameEmBlocos = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco)); // TODO:Esse valor vai precisar ser alterado depois
    bloco blocoAtual;

	// #pragma omp for collapse(2)
    for (int i = 0; i < frame.height; i+= tamanhoDoBloco)
    {
        for (int j = 0; j < frame.width; j+= tamanhoDoBloco)
        {
			bloco blocoAtual = criaBloco(i, j, frame.conteudo);
			int novoJ = (int)j / tamanhoDoBloco;
			int novoI = (int)i / tamanhoDoBloco;
			int indice = novoI * (int)(frame.width / tamanhoDoBloco) + novoJ;
			// printf("%d  ", indice);
			frameEmBlocos[indice] = blocoAtual; // Coloca o bloco atual em uma posição do array
        }

    }
    return frameEmBlocos;
}

bloco criaBloco(int i, int j, unsigned char **frame)
{
    bloco blocoRetorno;
    blocoRetorno.x = i;
    blocoRetorno.y = j;
    
  //#pragma parallel omp for collapse(2) shared(blocoRetorno)//não compensa
    for (int k = 0; k < tamanhoDoBloco; k++)
    {
        for (int l = 0; l < tamanhoDoBloco; l++)
        {
            blocoRetorno.bloco[k][l] = frame[i + k][j + l];
        }
    }
    return blocoRetorno;
}
