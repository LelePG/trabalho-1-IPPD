# trabalho-1-IPPD

## Integrantes

- Darlei Matheus Schmegel
- Dauan Ghisleni Zolinger
- Letícia Pegoraro Garcez
- Victor Kunde Bergmann

## Install MPI C++

```
$ sudo apt-get update -y
$ sudo apt-get install -y mpi
$ apt-get install mpich
```

## Compilar e executar com MPI

```
$ mpicxx -fopenmp main.cpp -lpthread && ./a.out >saida.txt
```

## Compilar e executar MPI com mais de um nó

```
$ mpicxx -fopenmp main.cpp -lpthread && && mpirun --host localhost:2 ./a.out >saida.txt
```

## Rodar com mais de um nó virtual

```
$ mpirun --host localhost:2 ./ex
```

## Compilar e executar com mais de um nó virtual

```
$ mpicxx ex.cpp -o ex && mpirun --host localhost:2 ./ex
```

## Comando para rodar o programa e colocar as saída no arquivo saida.txt

```
 $ g++ -fopenmp main.cpp -lpthread && ./a.out > saida.txt
```

## PC 8 núcleos 16 threads

- Rodou o programa em 39.428647 segundos.

## PC 2 núcleos 2 threads

- Rodou em 276.65s (isso dá 4:37 minutos)

## Comparativo de paralelismo no pc de 2 núcleos

| Função divideFrameEmBlocos paralelizada | Função criaBloco paralelizada | Tempo  |
| :-------------------------------------: | :---------------------------: | :----: |
|                   Não                   |              Não              | 32.64s |
|                   Sim                   |              Não              | 26.71s |
|                   Não                   |              Sim              | 27.51s |
|                   Sim                   |              Sim              | 26.15s |
