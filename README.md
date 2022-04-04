# Tabalho 2 - FSE

Este trabalho tem por objetivo a criação de um sistema distribuído de automação predial para monitoramento e acionamento de sensores e dispositivos de um prédio de 2 andares, mas com capacidade de aumento para mais andares.

De acordo com as especificações do trabalho, o sistema deve ser desenvolvido para funcionar em um conjunto de placas Raspberry Pi com um servidor central responsável pelo controle e interface com o usuário e servidores distribuídos para leitura e acionamento dos dispositivos. 
- monitoramento de temperatura e umidade
- sensores de presença
- sensores de fumaça
- sensores de contagem de pessoas
- sensores de abertura e fechamento de portas e janelas
- acionamento de lâmpadas, aparelhos de ar-condicionado, alarme e aspersores de água em caso de incêndio.

Descrição do trabalho: https://gitlab.com/fse_fga/trabalhos-2021_2/trabalho-2-2021-2.

## Instruções de compilação

Esse repositório contém os servidores distribuído e central. Para compilar cada um, entre no diretório do servidor que deseja compilar

```
cd servidor_central
```
ou
```
cd servidor_distribuido
```
após, compile o programa com o comando:

```
 make
```

## Execução

### Servidor Distribuído

O servidor distribuído espera um arquivo json como parâmetro para que possa carregar as informações do andar.

Para o problema proposto, estão disponíveis dois arquivos json no direitório "servidor_distribuido". 

- configuracao_andar_terreo.json
- configuracao_andar_1.json


Sendo assim, para executar o program:

```
make run FILE=<nome_do_arquivo>
```

### Servidor Central

Após a compilação do programa, basta: 

```
make run
```

## Uso 

### Servidor Distribuído

O servidor distribuído irá esperar conseguir fazer a conexão com o servidor central. Após realizar a conexão, ele irá encerrar caso a conexão seja perdida. 

### Servidor Central

- O servidor distribuído está configurado para escutar a porta **10061**

O servidor central irá esperar a conexão de ao menos 1 servidor distribuído. Após a conexão, será apresentada a tela de monitoramento do andar atual, como na imagem abaixo:




![]("./assets/experimento1.png")

