#include <stdio.h>
#include <cstdlib>

void leVideo(FILE *fp, int width, int heigth);
void pulaCanais(FILE *fp, int width, int height);
int leFrame(FILE *fp, unsigned char **frame, int width, int height);

// void fullSearch(unsigned char ** frame1, unsigned char ** frame2, unsigned char ** Rv, unsigned char ** Ra);

typedef struct coordenada{
    int x,y;
} coordenada;

int main(int argc, char *argv[]) {
    int width = 640;
    int height = 360;
    
    FILE *fp = fopen("../video.yuv", "rb");
    
    if (fp == NULL) {
        printf("Cannot open file");
        return 0;
    }

    leVideo(fp, width, height);
    //fullSearch(frame1, frame2, Rv, Ra);
    // Close file
    fclose(fp);
}

int leFrame(FILE *fp, unsigned char **frame, int width, int height){
    for (int i = 0; i < height; i++) {//colunas
        frame[i] = (unsigned char*)malloc(sizeof *frame[i] * width);//aloca a linha
        int bytesLidos  = fread(frame[i], sizeof(unsigned char), width, fp);
        if(bytesLidos  < sizeof(unsigned char)){
            return 0;
        }
    }
    printf("%s\n",frame[5]);//print debug
    pulaCanais(fp,width,height);
    return 1;
}

void pulaCanais(FILE *fp, int width, int height){
    unsigned char * aux = (unsigned char*)malloc(sizeof *aux* width * height);
    fread(aux, sizeof(unsigned char), width * height / 2, fp);
}

void leVideo(FILE *fp, int width, int height) {
    unsigned char **frameReferencia = (unsigned char**)malloc(sizeof *frameReferencia * height);
    //unsigned char ***video = (unsigned char***)malloc(sizeof **frameReferencia * height * width);
    unsigned char **frameAtual = (unsigned char**)malloc(sizeof *frameAtual * height);

    // Leitura do frame de Referencia
    leFrame(fp, frameReferencia, width, height);

    // Leitura dos demais frames
    int sucesso, i;
    i = 1;//Podemos apagar esse I depois
    do{
        sucesso =  leFrame(fp, frameAtual, width, height);
        //Aqui que a gente tem que fazer a manipulação do frame de referencia com o frame atual
        printf("Li o frame %d\n", i);
        i++;
    } while(sucesso);
}

// void fullSearch(unsigned char ** frame1, unsigned char ** frame2, unsigned char ** Rv, unsigned char ** Ra) {
//     coordenada* Rv, Ra;//Tipo que eu criei com os valores x e y
// //     //alocar os vetore Rv e Ra
// //     //pra cada bloco do frame atual (frame2)
// //     //comparar onde ele está no frame referencia (frame1)
// //     //se encontrar guardar nos vetores Rv e Ra
// //     //fazer isso pra todos os frames seguintes até o fim seguinte
// }