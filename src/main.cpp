#include <stdio.h>
#include <cstdlib>

const int tamanhoDoBloco = 8;
const int meioTamanhoDoBloco = (int)tamanhoDoBloco / 2;
typedef struct bloco
{
    unsigned char bloco[tamanhoDoBloco][tamanhoDoBloco]; // TODO:falar com o professor sobre isso.(na escific ta 8x8)
    int x;
    int y;
} bloco;
typedef struct vetor
{
    int x;
    int y;
} vetor;
typedef struct movimento
{
    vetor Rv,Ra;

} movimento;

void leVideo(FILE *fp, int width, int heigth);
void pulaCanais(FILE *fp, int width, int height);
int leFrame(FILE *fp, unsigned char **frame, int width, int height);
bloco *divideFrameEmBlocos(FILE *fp, int width, int height, int quantidadeDeBlocos);
// void fullSearch(unsigned char ** frame1, unsigned char ** frame2, unsigned char ** Rv, unsigned char ** Ra);
bloco criaBloco(int i, int j, unsigned char **frame);
bool blocosSaoIguais(bloco a, bloco b);
void comparaBlocos(bloco *frame1, bloco *frame2, int * blocosIguais, int quantidadeDeBlocos);
void zeraBlocosIguais(int* blocosIguais, int quantidadeDeBlocos);
void gerarvetores(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos);
int diffBlocos(bloco a, bloco b);

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
}

int leFrame(FILE *fp, unsigned char **frame, int width, int height)
{
    for (int i = 0; i < height; i++)
    {                                                                 // colunas
        frame[i] = (unsigned char *)malloc(sizeof *frame[i] * width); // aloca a linha
        int bytesLidos = fread(frame[i], sizeof(unsigned char), width, fp);
        if (bytesLidos < sizeof(unsigned char))
        {
            return 0;
        }
    }
    // printf("%s\n", frame[5]); // print debug
    pulaCanais(fp, width, height);
    return 1;
}

void pulaCanais(FILE *fp, int width, int height)
{
    unsigned char *aux = (unsigned char *)malloc(sizeof *aux * width * height);
    fread(aux, sizeof(unsigned char), width * height / 2, fp);
}

void leVideo(FILE *fp, int width, int height)
{
    // unsigned char **frameReferencia = (unsigned char **)malloc(sizeof *frameReferencia * height);
    //  unsigned char ***video = (unsigned char***)malloc(sizeof **frameReferencia * height * width);
    // unsigned char **frameAtual = (unsigned char **)malloc(sizeof *frameAtual * height);

    int quantidadeDeBlocos = (int)(width/tamanhoDoBloco) * (int)(height/tamanhoDoBloco);

    bloco *frameEmBlocosReferencia = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    bloco *frameEmBlocosAtual = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    int *blocosIguais = (int*)malloc(quantidadeDeBlocos * sizeof(int));


    printf("quantidade de Blocos %d\n", quantidadeDeBlocos);
    frameEmBlocosReferencia = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos);
    
    // Leitura dos demais frames
    int i = 1; // Podemos apagar esse I depois

    // Essa leitura aqui tem que ser fora do laço pra mim poder colocar a condição de parada no final do laço
    frameEmBlocosAtual = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos); // em determinado momento, vais er null

    do // Vou ler todos os frames do vídeo nesse loop
    {
        frameEmBlocosAtual = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos); // em determinado momento, vais er null
        zeraBlocosIguais(blocosIguais, quantidadeDeBlocos);
        comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, blocosIguais, quantidadeDeBlocos);//Essa função tá levando todo o tempo do mundo pra resolver.
        gerarvetores(frameEmBlocosReferencia, frameEmBlocosAtual, blocosIguais, quantidadeDeBlocos);

        printf("Li o frame %d\n", i);
        i++;
    } while (false);//(frameEmBlocosAtual != NULL);
}
void comparaBlocos(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos)
{
    printf("Estou comparando\n");
    bool igualdade;
    int minId = -1;
    int minDiff = 1410065408;
    int diff;
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        for (int j = 0; j < quantidadeDeBlocos; j++)
        {
            diff = diffBlocos(frame1[i], frame2[j]);
            if(diff > minDiff)
            {
                minDiff = diff;
                minId = j;
            }
        }
        blocosIguais[i] = minId;
        printf("%d",minDiff);
    }
    return;// blocosSaoIguais;
}

int diffBlocos(bloco a, bloco b)
{
    float diff = 0;
    for (int i = 0; i < tamanhoDoBloco; i++)
    {
        for (int j = 0; j < tamanhoDoBloco; j++)
        {
            diff += a.bloco[i][j] - b.bloco[i][j];
        }
    }
    return diff;
}

bloco *divideFrameEmBlocos(FILE *fp, int width, int height, int quantidadeDeBlocos)
{
    printf("oi");
    unsigned char **frameAtual = (unsigned char **)malloc(sizeof *frameAtual * height);
    bloco *frameEmBlocos = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco)); // TODO:Esse valor vai precisar ser alterado depois

    bloco blocoAtual;
    bool sucesso = leFrame(fp, frameAtual, width, height);
    
    if (!sucesso)
    {
        return NULL;
    }
    for (int i = 0; i < height; i+=tamanhoDoBloco)
    {
        for (int j = 0; j < width; j+=tamanhoDoBloco)
        {
            blocoAtual = criaBloco(i, j, frameAtual);
        }
        frameEmBlocos[i - 3] = blocoAtual; // Coloca o bloco atual em uma posição do array
    }

    return frameEmBlocos;
}

bloco criaBloco(int i, int j, unsigned char **frame)
{
    bloco blocoRetorno;
    blocoRetorno.x = i;
    blocoRetorno.y = j;
    for (int k = 0; k <= tamanhoDoBloco; k++)
    {
        for (int l = 0; l <= tamanhoDoBloco; l++)
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
        if(blocosIguais!=0)
        {

        }
    }
}

void zeraBlocosIguais(int* blocosIguais,int quantidadeDeBlocos)
{
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        blocosIguais[i] = -1;
    }
}

// void fullSearch(unsigned char ** frame1, unsigned char ** frame2, unsigned char ** Rv, unsigned char ** Ra) {
//     coordenada* Rv, Ra;//Tipo que eu criei com os valores x e y
// //     //alocar os vetore Rv e Ra
// //     //pra cada bloco do frame atual (frame2)
// //     //comparar onde ele está no frame referencia (frame1)
// //     //se encontrar guardar nos vetores Rv e Ra
// //     //fazer isso pra todos os frames seguintes até o fim seguinte
// }