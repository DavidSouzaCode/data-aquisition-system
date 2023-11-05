# Sistema Assíncrono de Aquisição de Dados

## Descrição do Projeto

O objetivo deste projeto é criar um servidor assíncrono que simula um sistema de aquisição de dados de sensores industriais. 

Cada conexão representa um sensor. O servidor deve ser capaz de lidar com múltiplas conexões (sensores) simultaneamente, receber dados de forma assíncrona e registrar esses dados em um arquivo único para cada sensor.

## Requisitos

1. **Servidor assíncrono**: Implemente um servidor TCP utilizando a biblioteca Boost.Asio para aceitar múltiplas conexões simultaneamente. Cada conexão representa um sensor industrial.

2. **Recepção de dados**: O servidor deve receber dados dos clientes (sensores) de forma assíncrona. Cada mensagem enviada pelos sensores deve conter informações como identificador do sensor, data/hora da leitura e a leitura do sensor.

3. **Registro de dados**: Todas as leituras dos sensores recebidas pelo servidor devem ser registradas em um arquivo único para cada sensor. Assegure-se de que o servidor possa lidar com o acesso concorrente aos arquivos de log.

4. **Consulta de registros**: Implemente uma funcionalidade que permita a um cliente solicitar as últimas `n` leituras de um sensor específico, identificado pelo seu ID. O servidor deve responder com as `n` últimas leituras registradas para esse sensor.

## Formato das Mensagens

### Sensor para Servidor (Envio de Dados)

A mensagem deve ter o seguinte formato: `LOG|SENSOR_ID|DATA_HORA|LEITURA\r\n`. 

Por exemplo: `LOG|SENSOR_001|2023-05-11T15:30:00|78.5\r\n`.

### Cliente para Servidor (Solicitação de Registros)

A mensagem deve ter o seguinte formato: `GET|SENSOR_ID|NUMERO_DE_REGISTROS\r\n`. 
 
Por exemplo: `GET|SENSOR_001|10\r\n`.

### Servidor para Cliente (Resposta à Solicitação de Registros)

A mensagem deve ter o seguinte formato: `NUM_REGISTROS;DATA_HORA|LEITURA;...;DATA_HORA|LEITURA\r\n. 

O servidor deve retornar as `n` últimas leituras do sensor, precedidas pelo número total de registro, separadas por `;`. 

Por exemplo: `2;2023-05-11T15:30:00|78.5;2023-05-11T15:31:00|77.5\r\n`. 

Se um cliente solicitar mais registros do que os disponíveis, o servidor deve retornar apenas os registros disponíveis.

### Servidor para Cliente (Resposta de Erro) 

Se o ID do sensor fornecido pelo cliente for inválido (ou seja, não corresponder a nenhum sensor conhecido pelo servidor), o servidor deve enviar uma resposta de erro no seguinte formato: `ERROR|INVALID_SENSOR_ID\r\n`. 

Por exemplo: `ERROR|INVALID_SENSOR_ID\r\n`.

## Formato do Arquivo de Log

O arquivo de log é um arquivo binário composto por registros. Cada registro contém o ID do sensor (string), a data/hora da leitura (timestamp) e o valor da leitura (double). 

A definição do registro pode ser representada como uma struct em C++:

```c++
#pragma pack(push, 1)
struct LogRecord {
    char sensor_id[32]; // supondo um ID de sensor de até 32 caracteres
    std::time_t timestamp; // timestamp UNIX
    double value; // valor da leitura
};
#pragma pack(pop)
```

### Trabalhando com std::time_t

`std::time_t` é um tipo definido na biblioteca padrão de C++ que representa o tempo como o número de segundos passados desde a época Unix, que é 00:00:00 UTC em 1º de janeiro de 1970 (sem incluir os segundos bissextos). Portanto, é comumente usado para armazenar timestamps.

Para converter uma string de data/hora no formato "AAAA-MM-DDTHH:MM:SS" para `std::time_t`, você pode usar a seguinte função:

```c++
#include <ctime>
#include <iomanip>
#include <sstream>

std::time_t string_to_time_t(const std::string& time_string) {
    std::tm tm = {};
    std::istringstream ss(time_string);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::mktime(&tm);
}
```

Para converter um std::time_t para uma string de data/hora no formato "AAAA-MM-DDTHH:MM:SS", você pode usar a seguinte função:

```c++
#include <ctime>
#include <iomanip>
#include <sstream>

std::string time_t_to_string(std::time_t time) {
    std::tm* tm = std::localtime(&time);
    std::ostringstream ss;
    ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}
```

## Emulador de Sensor

O emulador de sensor foi projetado para simular um sensor real enviando leituras para o servidor de aquisição de dados. É uma ferramenta útil para testar o sistema em um ambiente controlado.

### Como executar

O emulador de sensor é um script Python e pode ser executado a partir da linha de comando como qualquer outro script Python. Aqui está o comando básico para executar o emulador de sensor:

```bash
python3 sensor_emulator.py
```
Por padrão, o emulador de sensor tentará se conectar a um servidor executando em localhost na porta 9000, e ele usará uma ID de sensor aleatória e enviará uma nova leitura a cada segundo.

### Argumentos da linha de comando

O emulador de sensor aceita vários argumentos de linha de comando que permitem controlar seu comportamento:

- ```--ip```: O endereço IP do servidor ao qual o emulador de sensor deve se conectar. O padrão é 'localhost'.
- ```--port```: A porta do servidor à qual o emulador de sensor deve se conectar. O padrão é 9000.
- ```--sensor_id```: A ID do sensor que o emulador deve usar ao enviar leituras. Por padrão, isso é uma string aleatória de até 31 caracteres.
- ```--frequency```: A frequência, em milissegundos, com que o emulador de sensor deve enviar novas leituras. O padrão é 1000 (1 segundo).

Aqui está um exemplo de como executar o emulador de sensor com argumentos personalizados:

```bash
python3 sensor_emulator.py --ip 127.0.0.1 --port 9000 --sensor_id my_sensor --frequency 500
```

Este comando fará com que o emulador de sensor se conecte a um servidor em 192.168.1.100 na porta 8000, use 'my_sensor' como a ID do sensor, e envie uma nova leitura a cada 500 milissegundos.

## Emulador de Cliente

O emulador de cliente é um script em Python que simula um cliente se conectando ao servidor e solicitando os últimos `n` registros de um sensor específico. Ele envia uma solicitação ao servidor e, em seguida, imprime a solicitação e a resposta.

### Como usar

Para usar o emulador de cliente, você precisa ter Python instalado em seu computador. Uma vez que você tenha Python instalado, você pode executar o emulador de cliente a partir da linha de comando.

Aqui está um exemplo de como executar o emulador de cliente:

```bash
python3 client_emulator.py SENSOR_ID NUM_RECORDS
```

Substitua SENSOR_ID pelo ID do sensor para o qual você deseja dados, e NUM_RECORDS pelo número de registros que você deseja solicitar.

Você também pode especificar o IP e a porta do servidor com as opções --ip e --port, respectivamente. Se você não especificar essas opções, o emulador de cliente usará 'localhost' para o IP e 9000 para a porta.

### Argumentos da Linha de Comando

O emulador de cliente aceita os seguintes argumentos da linha de comando:

- ```--ip```: O endereço IP do servidor. O padrão é 'localhost'.
- ```--port```: O número da porta do servidor. O padrão é 9000.
- ```sensor_id```: O ID do sensor para o qual solicitar dados. Este argumento é obrigatório.
- ```num_records```: O número de registros para solicitar. Este argumento é obrigatório.



## Pasta de Exemplos

A pasta de exemplos contém alguns arquivos de código-fonte que servem como exemplos de como implementar algumas das funcionalidades requeridas para este projeto.

### async_echo_server.cpp

Este arquivo contém uma implementação de um servidor de eco TCP assíncrono usando a biblioteca Boost Asio. Este código serve como um exemplo de como configurar e iniciar um servidor TCP, aceitar conexões de entrada e realizar leitura e gravação assíncrona em soquetes usando manipuladores de retorno de chamada.

### binary_file_manipulation.cpp

Este arquivo ilustra como manipular arquivos binários em C++. Ele mostra como abrir um arquivo em modo binário, escrever dados binários em um arquivo e ler dados binários de um arquivo. Este exemplo pode ser útil para entender como armazenar as leituras de sensores em um arquivo binário.
