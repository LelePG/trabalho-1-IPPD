#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>	// Necessário para usar strings

#include <omp.h>	
using namespace std;

const int tamanhoDoBloco = 8;

typedef struct TypeFrame {
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

// typedef struct movimento
// {
//     vetor Rv,Ra;
// } movimento;

void leVideo(FILE *fp, int width, int heigth);
void pulaCanais(FILE *fp, int width, int height);
int leFrame(FILE *fp, TypeFrame frame, int width, int height);
bloco *divideFrameEmBlocos(TypeFrame frame, int quantidadeDeBlocos);
// bloco *divideFrameEmBlocos(FILE *fp, int width, int height, int quantidadeDeBlocos);
// void fullSearch(unsigned char ** frame1, unsigned char ** frame2, unsigned char ** Rv, unsigned char ** Ra);
bloco criaBloco(int i, int j, unsigned char **frame);
bool blocosSaoIguais(bloco a, bloco b);
string comparaBlocos(bloco *frame1, bloco *frame2, int quantidadeDeBlocos, int framePosicao);
void zeraBlocosIguais(int *blocosIguais, int quantidadeDeBlocos);
void gerarvetores(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos);
int encontraBlocoMaisParecido(bloco a, bloco b);
string imprimeCorrespondencia(coordenada *Rv, coordenada *Ra, int tamanhoVetor);
void imprimeBloco(bloco b);

int main(int argc, char *argv[])
{
    int width = 640;
    int height = 360;

    FILE *fp = fopen("../video.yuv", "rb");

    if (fp == NULL)
    {
        printf("Cannot open file");
        return 0;
    }
    

    leVideo(fp, width, height);
    // fullSearch(frame1, frame2, Rv, Ra);
    //  Close file
    fclose(fp);
    return 0;
}

int leFrame(FILE *fp, TypeFrame frame, int width, int height)
{
    for (int i = 0; i < frame.height; i++)
    {                                                                 // colunas
        frame.conteudo[i] = (unsigned char *)malloc(sizeof *frame.conteudo[i] * frame.width); // aloca a linha
        int bytesLidos = fread(frame.conteudo[i], sizeof(unsigned char), frame.width, fp);
        if (bytesLidos < sizeof(unsigned char))
        {
            return 0;
        }
    }
    // printf("%s\n", frame.conteudo[5]); // print debug
    pulaCanais(fp, frame.width, frame.height);
    return 1;
}

void pulaCanais(FILE *fp, int width, int height)
{
    unsigned char *aux = (unsigned char *)malloc(sizeof *aux * width * height);
    fread(aux, sizeof(unsigned char), width * height / 2, fp);
}

void leVideo(FILE *fp, int width, int height)
{
    int quantidadeDeBlocos = (int)((width) / tamanhoDoBloco) * (int)((height)/ tamanhoDoBloco);

    bloco *frameEmBlocosReferencia = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    bloco *frameEmBlocosAtual = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    int *blocosIguais = (int *)malloc(quantidadeDeBlocos * sizeof(int));

    printf("quantidade de Blocos %d\n", quantidadeDeBlocos);

    TypeFrame framesPraComparar[119];
    for (int w = 0;  w < 18 -2 ; w++){
        leFrame(fp, framesPraComparar[w], width, height);
    }
    
    frameEmBlocosReferencia = divideFrameEmBlocos(framesPraComparar[0], quantidadeDeBlocos);
    

    // Essa leitura aqui tem que ser fora do laço pra mim poder colocar a condição de parada no final do laço
    //frameEmBlocosAtual = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos); // em determinado momento, vais er null   

    	
	#pragma omp parallel for shared(fp, width, height,quantidadeDeBlocos) 
		for (int w = 1;  w < 18 -2 ; w++){//quantidadeDeFrames
            frameEmBlocosAtual = divideFrameEmBlocos(framesPraComparar[w], quantidadeDeBlocos); // em determinado momento, vais er null
            // printf("Inicio frame %d. Thread %d\n", w, omp_get_thread_num());
			comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, quantidadeDeBlocos, w); // Essa função tá levando todo o tempo do mundo pra resolver.
			
		}
	
	
	
}

string comparaBlocos(bloco *frame1, bloco *frame2, int quantidadeDeBlocos, int framePosicao)
{
    printf("Estou comparando\n");
    bool igualdade;
    // int indiceBlocoMaisParecido = -1;
    // int menorNivelDeProximidade = 1000000;
    int nivelDeProximidadeAtual;
    
    int indiceBlocoMaisParecido[quantidadeDeBlocos];
    int menorNivelDeProximidade[quantidadeDeBlocos];
    for (int  i = 0; i < quantidadeDeBlocos;i++){
        indiceBlocoMaisParecido[i] = -1;
        menorNivelDeProximidade[i] = 1000000;
    }

    // Criar os vetores Rv e Ra que são vetores de coordenadas
    //Mover isso para fora e usar global
    coordenada *Rv = (coordenada *)malloc(quantidadeDeBlocos * sizeof(coordenada)); // referencia
    coordenada *Ra = (coordenada *)malloc(quantidadeDeBlocos * sizeof(coordenada)); // atual
	int i,j;
	
    // #pragma omp shared(Rv, Ra, frame1,frame2,i) private(indiceBlocoMaisParecido,nivelDeProximidadeAtual,j) for collapse(2)
    for (  i = 0; i < quantidadeDeBlocos ;i++){                                    // frame1
        for (j = 0; j < quantidadeDeBlocos; j++){ // frame2
        
                nivelDeProximidadeAtual = encontraBlocoMaisParecido(frame1[i], frame2[j]);
                if (nivelDeProximidadeAtual < menorNivelDeProximidade[i] && nivelDeProximidadeAtual>0)
                {
                    menorNivelDeProximidade[i] = nivelDeProximidadeAtual;
                    indiceBlocoMaisParecido[i] = j;
                }
        }
        // indiceBlocoMaisParecido vai ter o índice do bloco mais parecido com frame1[i]

        // menorNivelDeProximidade[i] = 1000000;
        // indiceBlocoMaisParecido[i] = -1;
        
        // Como deixei essas variaveis com priate, não preciso resetar ao fial do for.
		// menorNivelDeProximidade = 1000000;
        // indiceBlocoMaisParecido = -1;
        
    }
    for (  i = 0; i < quantidadeDeBlocos ;i++)  {                                  // frame1
        Rv[i].x = frame1[i].x;
        Rv[i].y = frame1[i].y; // travado
        Ra[i].x = frame2[indiceBlocoMaisParecido[i]].x;
        Ra[i].y = frame2[indiceBlocoMaisParecido[i]].y;
    }
    string concat = imprimeCorrespondencia(Rv,Ra, quantidadeDeBlocos); 
  
    // exit(0)
	printf("_______________________________\nFrame[%d]\n%s_______________________________\n",framePosicao, concat.c_str());
    return concat; // blocosSaoIguais;
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

int encontraBlocoMaisParecido(bloco a, bloco b)
{
    float diff = 0;
	// #pragma omp for collapse(2)
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
			blocoAtual = criaBloco(i, j, frame.conteudo);
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
    for (int k = 0; k < tamanhoDoBloco; k++)
    {
        for (int l = 0; l < tamanhoDoBloco; l++)
        {
            blocoRetorno.bloco[k][l] = frame[i + k][j + l];
        }
    }
    return blocoRetorno;
}
void gerarvetores(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos)
{
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        if (blocosIguais != 0)
        {
        }
    }
}

void zeraBlocosIguais(int *blocosIguais, int quantidadeDeBlocos)
{
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        blocosIguais[i] = -1;
    }
}