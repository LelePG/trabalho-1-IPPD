# trabalho-1-IPPD

## Integrantes

- Darlei Matheus Schmegel
- Dauan Ghisleni Zolinger
- Letícia Pegoraro Garcez
- Victor Kunde Bergmann

## Comando para rodar o programa e colocar as saída no arquivo saida.txt

     g++ -fopenmp main.cpp -lpthread && ./a.out >saida.txt

## PC 8 núcleos 16 threads

- Rodou o programa em 39.428647 segundos.

- Rodou em 276.65 no pc da letícia 2 threads (isso dá 4:37 minutos)

## Comparativo de paralelismo no pc da
|Função divide frame em Blocos | Função cria Bloco|Tempo|
|:-----------------------------:|:-------------------:|:-:|
|Não|Não|32.64s|
|Sim|Não|26.71s|
|Não|Sim|27.51s|
|Sim|Sim|26.15s|