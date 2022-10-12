#include <stdio.h>
#include <cstdlib>

const int tamanhoDoBloco = 7;
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
void comparaBlocos(bloco *frame1, bloco *frame2, int quantidadeDeBlocos);
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

    int quantidadeDeBlocos = (640 - meioTamanhoDoBloco) * (360 - meioTamanhoDoBloco);

    bloco *frameEmBlocosReferencia = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    bloco *frameEmBlocosAtual = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco));
    int *blocosIguais = (int*)malloc(quantidadeDeBlocos * sizeof(int));
    movimento **vetoresVideo;
    // for
    //     ler
    //     comparar       CONVERSAR COM O TIME
    //     gerarvetores

    frameEmBlocosReferencia = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos);

    // Leitura dos demais frames
    int i = 1; // Podemos apagar esse I depois

    // Essa leitura aqui tem que ser fora do laço pra mim poder colocar a condição de parada no final do laço
    frameEmBlocosAtual = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos); // em determinado momento, vais er null

    do // Vou ler todos os frames do vídeo nesse loop
    {
        frameEmBlocosAtual = divideFrameEmBlocos(fp, width, height, quantidadeDeBlocos); // em determinado momento, vais er null
        zeraBlocosIguais(blocosIguais);
        comparaBlocos(frameEmBlocosReferencia, frameEmBlocosAtual, blocosIguais, quantidadeDeBlocos);//Essa função tá levando todo o tempo do mundo pra resolver.
        int *vetor = (int*)malloc(quantidadeDeBlocos * sizeof(int));
        gerarvetores(frameEmBlocosReferencia, frameEmBlocosAtual, blocosIguais, quantidadeDeBlocos);
        // comparaBloco()
        // manipulaFrame(frameAtual,frameReferencia);
        // Aqui que a gente tem que fazer a manipulação do frame de referencia com o frame atual
        printf("Li o frame %d\n", i);
        i++;
    } while (frameEmBlocosAtual != NULL);
}
void comparaBlocos(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos)
{
    printf("Estou comparando\n");
    bool igualdade;
    
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        for (int j = 0; j < quantidadeDeBlocos; j++)
        {
            igualdade = blocosSaoIguais(frame1[i], frame2[j]);
            printf("%d",igualdade);
            blocosIguais[i] = j;
        }
    }
    return blocosSaoIguais;
}

bool blocosSaoIguais(bloco a, bloco b)
{
    for (int i = 0; i < tamanhoDoBloco; i++)
    {
        for (int j = 0; j < tamanhoDoBloco; j++)
        {
            if (a.bloco[i][j] != b.bloco[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
}

bloco *divideFrameEmBlocos(FILE *fp, int width, int height, int quantidadeDeBlocos)
{
    unsigned char **frameAtual = (unsigned char **)malloc(sizeof *frameAtual * height);
    bloco *frameEmBlocos = (bloco *)malloc(quantidadeDeBlocos * sizeof(bloco)); // TODO:Esse valor vai precisar ser alterado depois
    // 2345678->2X9 = 18
    // 2345678 -> 14 blocos
    bloco blocoAtual;
    bool sucesso = leFrame(fp, frameAtual, width, height);
    if (!sucesso)
    {
        return NULL;
    }
    for (int i = meioTamanhoDoBloco; i < height - meioTamanhoDoBloco; i++) // Todo: lógica dos números
    {                                                                      // começando em 3 porque 3,3 é o meio de um bloco 7X7
        for (int j = meioTamanhoDoBloco; j < width - meioTamanhoDoBloco; j++)
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
    for (int k = -3; k <= 3; k++)
    {
        for (int l = -3; l <= 3; l++)
        {
            blocoRetorno.bloco[k + 3][l + 3] = frame[i + k][j + l];
        }
    }
    return blocoRetorno;
}
gerarvetores(bloco *frame1, bloco *frame2, int *blocosIguais, int quantidadeDeBlocos)
{
    for (int i = 0; i < quantidadeDeBlocos; i++)
    {
        if(blocosIguais!=0)
        {

        }
    }
}

zeraBlocosIguais(int* blocosIguais)
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