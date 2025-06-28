# Estrutura do Repositório

Este repositório contém os arquivos e diretórios necessários para o desenvolvimento e execução do projeto. Abaixo está a descrição da estrutura:

```
├── main.pdf -> .relatorio/main.pdf: Arquivo PDF do relatório final.
├── main.tex -> .relatorio/main.tex: Arquivo LaTeX do relatório final.
├── projeto_b
│   ├── include
│   │   └── lodepng.h: Cabeçalho para manipulação de imagens PNG.
│   ├── Makefile: Arquivo para automação da compilação do projeto.
│   ├── src
│   │   ├── juliaSerial.cpp: Implementação do conjunto de Julia em modo serial.
│   │   ├── juliaThreads.cpp: Implementação do conjunto de Julia com threads.
│   │   ├── lodepng.cpp: Implementação para manipulação de imagens PNG.
│   │   ├── main.cpp: Arquivo principal do programa.
│   │   ├── mandelbrotSerial.cpp: Implementação do conjunto de Mandelbrot em modo serial.
│   │   └── mandelbrotThread.cpp: Implementação do conjunto de Mandelbrot com threads.
│   └── test.sh: Script para execução de testes automatizados.
└── README.MD: Este arquivo de documentação.
```

## Como Compilar

O projeto utiliza um `Makefile` para facilitar a compilação. Certifique-se de ter o compilador `g++` instalado. Abaixo estão as principais variáveis e comandos de compilação:

- **Compilador**: `g++`
- **Flags**: `-O3 -Wall -std=c++11 -lpng -fopenmp`
- **Estrutura de diretórios**:
    - Código-fonte: `src`
    - Objetos compilados: `build`
    - Binários: `bin`
    - Cabeçalhos: `include`

Para compilar o projeto, execute:

```bash
make
```

O binário gerado estará disponível no diretório `bin`.

Para limpar os arquivos gerados durante a compilação, utilize:

```bash
make clean
```

## Como Executar os Testes

O script `test.sh` realiza testes combinando diferentes valores de parâmetros para os conjuntos de Mandelbrot e Julia. Ele suporta a execução com ou sem o modo Julia. Para executar:

1. Certifique-se de que o binário foi gerado (`make`).
2. Execute o script:

```bash
bash projeto_b/test.sh
```

Para ativar o modo Julia, utilize o parâmetro `--julia`:

```bash
bash projeto_b/test.sh --julia
```

Os resultados serão armazenados no diretório `resultados_combinados`, organizados por parâmetros utilizados.


## Como Executar Manualmente

Caso prefira executar os binários manualmente, siga os passos abaixo:

1. Certifique-se de que o binário foi gerado (`make`).
2. Navegue até o diretório `bin`:

```bash
cd bin
```

3. Execute o binário correspondente ao conjunto desejado, fornecendo os parâmetros necessários:

- Para o conjunto de Mandelbrot em modo serial:

```bash
./mandelbrotSerial <largura> <altura> <x_min> <x_max> <y_min> <y_max> <max_iter> <arquivo_saida>
```

- Para o conjunto de Mandelbrot com threads:

```bash
./mandelbrotThread <largura> <altura> <x_min> <x_max> <y_min> <y_max> <max_iter> <arquivo_saida> <num_threads>
```

- Para o conjunto de Julia em modo serial:

```bash
./juliaSerial <largura> <altura> <x_min> <x_max> <y_min> <y_max> <max_iter> <arquivo_saida> <c_real> <c_imag>
```

- Para o conjunto de Julia com threads:

```bash
./juliaThreads <largura> <altura> <x_min> <x_max> <y_min> <y_max> <max_iter> <arquivo_saida> <c_real> <c_imag> <num_threads>
```

### Parâmetros

- `<largura>` e `<altura>`: Dimensões da imagem de saída.
- `<x_min>`, `<x_max>`, `<y_min>`, `<y_max>`: Limites do plano complexo.
- `<max_iter>`: Número máximo de iterações.
- `<arquivo_saida>`: Nome do arquivo de saída (ex.: `imagem.png`).
- `<num_threads>`: Número de threads (apenas para versões com threads).
- `<c_real>` e `<c_imag>`: Parte real e imaginária da constante `c` (apenas para o conjunto de Julia).

4. Os arquivos de saída serão gerados no diretório atual com o nome especificado.