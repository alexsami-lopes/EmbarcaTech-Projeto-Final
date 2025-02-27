#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "hardware/adc.h"     
#include "hardware/pwm.h"  
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"


#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define JOYSTICK_X_PIN 27  // GPIO para eixo X
#define JOYSTICK_Y_PIN 26  // GPIO para eixo Y
#define JOYSTICK_PB 22 // GPIO para botão do Joystick
#define Botao_A 5 // GPIO para botão A

#define VRX_PIN 26  
#define LED_PIN_GREEN 11  
#define LED_PIN_BLUE 12 
#define LED_PIN_RED 13  

#define BORDA_NORMAL 0  
#define BORDA_ZIGZAG 1 

//////////////////////////Network///////////////////////////////////////////
//Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6

#define LED_PIN 12          // Define o pino do LED
#define WIFI_SSID_INITIAL "NomeDoWifi"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS_INITIAL "SenhaDoWifi" // Substitua pela senha da sua rede Wi-Fi

// Definindo a senha armazenada (real: "admin") e "criptografia"
// Função de criptografia simples incrementa cada caractere em 1.
// Assim, "admin" torna-se "benjo"
#define INITIAL_STORED_PASSWORD "admin"
#define INITIAL_STORED_PASSWORD_CLIENT "Abrir_"

#define TELA_ABRIR_CONFIG           0
#define TELA_TECLADO                1
#define TELA_TENTATIVAS_FRACASSADAS 2
#define TELA_SISTEMA_TRAVADO        3
#define TELA_TECLADO_ADM            4
#define TELA_WIFI_CONECTADO         5
#define TELA_WIFI_NAO_CONECTADO     6
#define TELA_CONECTAR_WIFI          7
#define TELA_TECLADO_WIFI           8
#define TELA_TECLADO_WIFI_SENHA     9
#define TELA_WIFI_CONECTANDO        10
//////////////////////////Network-end///////////////////////////////////////////
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
uint pwm_slice_red = 0;  
uint pwm_slice_blue = 0; 
uint pwm_wrap = 4096; 
ssd1306_t ssd; // Inicializa a estrutura do display
bool cor = true;
bool leds_pwm_ativados = false;
bool leds_pwm_mudaram_de_estado = true;
bool tipo_borda = false;
//bool alterna_maiuscula = false;
uint borda_atual = 90;
char senha[9] = "";
char senha_oculta[9] = "";
char senha_admin[50] = "";
char senha_admin_oculta[50] = "";
volatile char tecla_atual;
uint tela_atual = 0;
char opcao_atual[16];

uint32_t last_print_time = 0; 

volatile uint tentativas = 5;
volatile bool tentativa_fracassada = false;

char wifi_ssd[50] = WIFI_SSID_INITIAL;
char wifi_pass[50] = WIFI_PASS_INITIAL;

char wifi_ssd_digitado[50] = "";
char wifi_pass_digitado[50] = "";

bool conectado = false;
 
uint16_t adc_value_x = 0;
uint16_t adc_value_y = 0; 
uint8_t screen_x = 6;
uint8_t screen_y = 3; 

char str_x[5];  // Buffer para armazenar a string
char str_y[5];  // Buffer para armazenar a string  
char str_xx[5];  // Buffer para armazenar a string
char str_yy[5];  // Buffer para armazenar a string 




char teclado[4][10] = {
        {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
        {' ', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'}, // '^' representa a tecla que alterna entre maiúsculas e minúsculas
        {'^', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '_', '@'},
        {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'}
    };

//////////////////////////Network///////////////////////////////////////////

// Variável global para indicar se o usuário está logado
static bool loggedIn = false;

char stored_encrypted_passord_host[50];
char stored_encrypted_passord_client[9];
char mode[20];



const char *loginPage =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"  <meta charset='UTF-8'>"
"  <title>Login</title>"
"  <style>"
"    body { font-family: Arial, sans-serif; background: #f0f0f0; }"
"    .container { width: 300px; margin: 100px auto; padding: 20px; background: #fff; "
"                 border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }"
"    input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 5px 0; "
"                 border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; }"
"    button { width: 100%; padding: 10px; margin: 5px 0; border: none; border-radius: 5px; "
"             background: #4CAF50; color: white; font-size: 16px; cursor: pointer; transition: background 0.3s ease; box-sizing: border-box; }"
"    button:hover { background: #45a049; }"
"    button:active { background: #3e8e41; }"
"  </style>"
"</head>"
"<body>"
"  <div class='container'>"
"    <h2>Login</h2>"
"    <form method='POST' action='/login'>"
"      <input type='text' name='username' placeholder='Usuário' required>"
"      <input type='password' name='password' placeholder='Senha' required>"
"      <button type='submit'>Entrar</button>"
"    </form>"
"  </div>"
"</body>"
"</html>";



const char *ledControlPage =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"  <meta charset='UTF-8'>"
"  <title>Controle do LED</title>"
"  <style>"
"    body { font-family: Arial, sans-serif; background: #f0f0f0; margin:0; padding:0; }"
"    .container { width: 300px; margin: 50px auto; padding: 20px; background: #fff; "
"                 border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; }"
"    button { width: 100%; padding: 10px; margin: 10px 0; border: none; border-radius: 5px; "
"             background: #4CAF50; color: white; font-size: 16px; cursor: pointer; transition: background 0.3s ease; box-sizing: border-box; }"
"    button:hover { background: #45a049; }"
"    button:active { background: #3e8e41; }"
"    input[type='password'], select { width: 100%; padding: 10px; margin: 5px 0 10px 0; "
"             border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; }"
"    label { display: block; text-align: left; margin-top: 10px; font-weight: bold; }"
"    /* Área de configurações: inicialmente recolhida */"
"    #configArea { max-height: 0; overflow: hidden; transition: max-height 0.5s ease; }"
"  </style>"
"</head>"
"<body>"
"  <div class='container'>"
"    <h2>Controle do LED</h2>"
"    <form method='GET' action='/porta-aberta'>"
"      <button type='submit'>Abrir Porta</button>"
"    </form>"
"    <form method='GET' action='/porta-fechada'>"
"      <button type='submit'>Trancar Porta</button>"
"    </form>"
"    <button id='configBtn'>Configurações</button>"
"    <div id='configArea'>"
"      <form method='POST' action='/settings'>"
"        <label>Mudar Senha do Host</label>"
"        <input type='password' name='new_pass_host' placeholder='Nova Senha do Host'>"
"        <label>Mudar Senha do Cliente</label>"
"        <input type='password' name='new_pass_client' placeholder='Nova Senha do Cliente'>"
"        <label>Mudar tipo</label>"
"        <select name='new_mode'>"
"          <option value='único'>único</option>"
"          <option value='externo'>externo</option>"
"          <option value='interno'>interno</option>"
"        </select>"
"        <button type='submit'>Salvar</button>"
"      </form>"
"    </div>"
"  </div>"
"  <script>"
"    const configBtn = document.getElementById('configBtn');"
"    const configArea = document.getElementById('configArea');"
"    let configVisible = false;"
"    configBtn.addEventListener('click', function() {"
"      if (!configVisible) {"
"        configArea.style.maxHeight = '350px'; /* Altura suficiente para exibir os campos */"
"        configVisible = true;"
"      } else {"
"        configArea.style.maxHeight = '0';"
"        configVisible = false;"
"      }"
"    });"
"  </script>"
"</body>"
"</html>";

const char *settingsSavedResponse =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<title>Configurações Salvas</title>"
    "<style>"
    "  body { font-family: Arial, sans-serif; background: #f0f0f0; }"
    "  .container { width: 300px; margin: 100px auto; padding: 20px; background: #fff; "
    "               border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; }"
    "  button { width: 100%; padding: 10px; margin: 10px 0; border: none; border-radius: 5px; "
    "           background: #4CAF50; color: white; font-size: 16px; cursor: pointer; transition: background 0.3s ease; }"
    "  button:hover { background: #45a049; }"
    "  button:active { background: #3e8e41; }"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h2>Configurações Salvas</h2>"
    "<p>As novas configurações foram salvas com sucesso.</p>"
    "<a href='/'><button>Voltar</button></a>"
    "</div>"
    "</body>"
    "</html>";


// Resposta padrão para requisições que não se encaixem nos casos esperados
const char *defaultResponse =
"HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html>"
"<html><body><h2>Página não encontrada</h2></body></html>\r\n";

uint wifi_connect();
uint wifi_init();

void abrir_porta() {
    gpio_put(LED_PIN, 1);  // Liga o LED
    tentativas = 5;
    strcpy(senha, "");
    strcpy(senha_oculta, "");
    strcpy(senha_admin, "");
    strcpy(senha_admin_oculta, "");

}

void fechar_porta() {
    gpio_put(LED_PIN, 0);  // Liga o LED
}


// Função simples de criptografia: incrementa cada caractere em 1
void encrypt_password(const char *input, char *output) {
    while (*input) {
        *output = *input + 1;
        input++;
        output++;
    }
    *output = '\0';
}

// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Copia todo o payload para um buffer local, garantindo que seja uma string terminada em '\0'
    char *request = malloc(p->tot_len + 1);
    if (request == NULL) {
        pbuf_free(p);
        return ERR_MEM;
    }
    memcpy(request, p->payload, p->tot_len);
    request[p->tot_len] = '\0';

    // Processamento das requisições conforme a rota
    if (strstr(request, "POST /login") != NULL) {
        // Encontra o início do corpo da requisição (após "\r\n\r\n")
        char *body = strstr(request, "\r\n\r\n");
        if (body != NULL) {
            body += 4; // pula o separador
            // Cria um buffer local para o body
            char *local_body = malloc(strlen(body) + 1);
            if (local_body != NULL) {
                strcpy(local_body, body);

                // Debug: imprime o body da requisição
                printf("\n----- Body da requisição -----\n%s\n----- Fim do body -----\n", local_body);

                // Parsing dos parâmetros: assume o formato "username=...&password=..."
                char username[50] = {0};
                char password[50] = {0};
                sscanf(local_body, "username=%49[^&]&password=%49[^&]", username, password);

                // Criptografa a senha recebida
                char encrypted[50] = {0};
                encrypt_password(password, encrypted);
                printf("\nPassword: %s", password);
                printf("\nEncrypted: %s", encrypted);
                printf("\nstored_encrypted_passord_host: %s", stored_encrypted_passord_host);

                // Valida a senha criptografada
                if (strcmp(encrypted, stored_encrypted_passord_host) == 0) {
                    loggedIn = true;
                    tcp_write(tpcb, ledControlPage, strlen(ledControlPage), TCP_WRITE_FLAG_COPY);
                } else {
                    tcp_write(tpcb, loginPage, strlen(loginPage), TCP_WRITE_FLAG_COPY);
                }
                free(local_body);
            }
        }
    }
    else if (strstr(request, "POST /settings") != NULL) {
        // Encontra o início do corpo da requisição
        char *body = strstr(request, "\r\n\r\n");
        if (body != NULL) {
            body += 4; // pula o separador de headers do corpo
            // Cria um buffer local para o body
            char *local_body = malloc(strlen(body) + 1);
            if (local_body != NULL) {
                strcpy(local_body, body);

                // Debug: imprime o body da requisição
                printf("\n----- Body da requisição -----\n%s\n----- Fim do body -----\n", local_body);

                // Parsing dos parâmetros: assume o formato "new_pass_host=...&new_pass_client=...&new_mode=..."
                char new_pass_host[50] = {0};
                char new_pass_client[50] = {0};
                char new_mode[20] = {0};
                sscanf(local_body, "new_pass_host=%49[^&]&new_pass_client=%49[^&]&new_mode=%19[^&]", new_pass_host, new_pass_client, new_mode);

                // Criptografa as senhas recebidas
                char encrypted_host[50] = {0};
                char encrypted_client[9] = {0};
                encrypt_password(new_pass_host, encrypted_host);
                encrypt_password(new_pass_client, encrypted_client);

                // Salva os valores nas variáveis globais da Pico W
                strcpy(stored_encrypted_passord_host, encrypted_host);
                strcpy(stored_encrypted_passord_client, encrypted_client);
                strcpy(mode, new_mode);

                printf("\nNew Password: %s", new_pass_host);
                printf("\nNew Encrypted: %s", stored_encrypted_passord_host);
                //tentativas = 5;
                tcp_write(tpcb, settingsSavedResponse, strlen(settingsSavedResponse), TCP_WRITE_FLAG_COPY);
                free(local_body);
                
            }
        }
    }
    // Requisições GET para ligar o LED
    else if (strstr(request, "GET /porta-aberta") != NULL) {
        if (!loggedIn) {
            tcp_write(tpcb, loginPage, strlen(loginPage), TCP_WRITE_FLAG_COPY);
        } else {
            //gpio_put(LED_PIN, 1);  // Liga o LED
            abrir_porta();
            //tentativas = 5;
            tcp_write(tpcb, ledControlPage, strlen(ledControlPage), TCP_WRITE_FLAG_COPY);
        }
    }
    // Requisições GET para desligar o LED
    else if (strstr(request, "GET /porta-fechada") != NULL) {
        if (!loggedIn) {
            tcp_write(tpcb, loginPage, strlen(loginPage), TCP_WRITE_FLAG_COPY);
        } else {
            //gpio_put(LED_PIN, 0);  // Desliga o LED
            fechar_porta();
            tcp_write(tpcb, ledControlPage, strlen(ledControlPage), TCP_WRITE_FLAG_COPY);
        }
    }
    // Exibe a página de login para requisições GET iniciais
    else if (strstr(request, "GET /") != NULL || strstr(request, "GET /login") != NULL) {
        tcp_write(tpcb, loginPage, strlen(loginPage), TCP_WRITE_FLAG_COPY);
    }
    // Qualquer outra URL recebe resposta 404
    else {
        tcp_write(tpcb, defaultResponse, strlen(defaultResponse), TCP_WRITE_FLAG_COPY);
    }

    tcp_recved(tpcb, p->tot_len);
    free(request);
    pbuf_free(p);
    return ERR_OK;
}


// Callback de nova conexão: associa o callback HTTP
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);
    return ERR_OK;
}

// Configuração do servidor HTTP na porta 80
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);

    printf("Servidor HTTP rodando na porta 80...\n");
}
//////////////////////////Network-end///////////////////////////////////////////
// Função converte letras maiúsculas para minúsculas e vice-versa.
void toggleCase(char teclado[4][10]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            // Verifica se o caractere é uma letra (mas ignora caracteres especiais, como o da tecla de toggle)
            if (teclado[i][j] >= 'A' && teclado[i][j] <= 'Z') {
                teclado[i][j] = teclado[i][j] + ('a' - 'A');
            } else if (teclado[i][j] >= 'a' && teclado[i][j] <= 'z') {
                teclado[i][j] = teclado[i][j] - ('a' - 'A');
            } else if (teclado[i][j] == '@') {
                teclado[i][j] = '.';
            } else if (teclado[i][j] == '.') {
                teclado[i][j] = '@';
            }
        }
    }
}

// Função que extrai os últimos caracteres de 'src' e os coloca em 'dest'.
void extrair_ultimos(const char *src, char *dest, size_t dest_size) {
    size_t len_src = strlen(src);
    
    if (dest_size == 0) {
        // Se o tamanho do destino for 0, retorna.
        return;
    }
    
    // Determina quantos caracteres cabem em dest (excluindo o \0).
    size_t num_chars = dest_size - 1;
    
    // Se a origem tem menos caracteres que o espaço disponível copia tudo.
    if (len_src < num_chars) {
        num_chars = len_src;
    }
    
    // Copia os últimos caracteres de src para dest.
    strncpy(dest, src + (len_src - num_chars), num_chars);
    

    dest[num_chars] = '\0';
}

void tela_teclado(char *titulo){

    adc_select_input(0); // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    adc_value_y = adc_read(); 
    adc_select_input(1); // Seleciona o ADC para eixo X. O pino 26 como entrada analógica
    adc_value_x = adc_read();   
    sprintf(str_x, "%d", adc_value_x);  // Converte o inteiro em string
    sprintf(str_y, "%d", adc_value_y);  // Converte o inteiro em string

    double screen_x16 = ((((((double) adc_value_x)) / 4095) * 10));
    double screen_y16 = ((((4095 - ((double) adc_value_y)) / 4095) * 4));  
    uint8_t screen_x = (uint8_t) screen_x16;
    uint8_t screen_y = (uint8_t) screen_y16;
    // printf("adc_value_x: %u\n", adc_value_x); 
    // printf("adc_value_y: %u\n", adc_value_y); 
    // printf("screen_x: %u\n", screen_x); 
    // printf("screen_y: %u\n", screen_y); 

    char senha_exibida[9] = ""; // inicialmente vazia

    
    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 0, 0, 128, 16, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, titulo, 4, 4); // Desenha uma string    
    //ssd1306_draw_string(&ssd, senha_oculta, 56, 4); // Desenha uma string 
    if(tela_atual == TELA_TECLADO) {
        ssd1306_draw_string(&ssd, senha_oculta, 56, 4); // Desenha uma string   
    } else if(tela_atual == TELA_TECLADO_ADM){
        extrair_ultimos(senha_admin_oculta, senha_exibida, sizeof(senha_exibida));
        ssd1306_draw_string(&ssd, senha_exibida, 56, 4); // Desenha uma string 
    } else if(tela_atual == TELA_TECLADO_WIFI){
        extrair_ultimos(wifi_ssd_digitado, senha_exibida, sizeof(senha_exibida));
        ssd1306_draw_string(&ssd, senha_exibida, 56, 4); // Desenha uma string 
    } else if(tela_atual == TELA_TECLADO_WIFI_SENHA){
        extrair_ultimos(wifi_pass_digitado, senha_exibida, sizeof(senha_exibida));
        ssd1306_draw_string(&ssd, senha_exibida, 56, 4); // Desenha uma string 
    }

    


    ssd1306_rect(&ssd, screen_y * 12 + 16, screen_x * 12 + 4, 12, 12, cor, 1); // Desenha um retângulo 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            
            if(i == screen_y && j == screen_x) {
              ssd1306_draw_char_invert(&ssd, teclado[i][j],  j * 12 + 2 + 4, i * 12 + 16 + 3);
              tecla_atual = teclado[i][j];
            } else {
              ssd1306_draw_char(&ssd, teclado[i][j],  j * 12 + 2 + 4, i * 12 + 16 + 3);
            }
        }
    }
      
    ssd1306_send_data(&ssd); // Atualiza o display
}

void tela_abrir_config(){

    adc_select_input(0); // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    adc_value_y = adc_read(); 

    sprintf(str_y, "%d", adc_value_y);  // Converte o inteiro em string

    char opcoes_abrir_config[3][16] = {
            "Abrir Porta", 
            "Trancar Porta", 
            "Config"
        };
    uint num_elementos = sizeof(opcoes_abrir_config) / sizeof(opcoes_abrir_config[0]);

    uint8_t ret_heignt = 64 / num_elementos;
    uint8_t ret_width = 128;
    double screen_y16 = ((((4095 - ((double) adc_value_y)) / 4095) * num_elementos));  
    uint8_t screen_x = 0;
    uint8_t screen_y = (uint8_t) screen_y16;
    // if(adc_value_y > 1970 && adc_value_y < 2040) {
    //   screen_y = 0;
    // }
    // printf("adc_value_y: %u\n", adc_value_y); 
    // printf("screen_x: %u\n", screen_x); 
    // printf("screen_y: %u\n", screen_y); 
    

    
    ssd1306_fill(&ssd, !cor); // Limpa o display   


    ssd1306_rect(&ssd, screen_y * ret_heignt, screen_x, ret_width, ret_heignt, cor, 1); // Desenha um retângulo 
    for (int i = 0; i < 3; i++) {
          
          if(i == screen_y) {
            ssd1306_draw_inverted_string(&ssd, opcoes_abrir_config[i], (ret_width - strlen(opcoes_abrir_config[i]) * 8) / 2, 6 + ret_heignt * i); // Desenha uma string 
            strcpy(opcao_atual, opcoes_abrir_config[i]);
          } else {
            ssd1306_draw_string(&ssd, opcoes_abrir_config[i], (ret_width - strlen(opcoes_abrir_config[i]) * 8) / 2, 6 + ret_heignt * i); // Desenha uma string   
            
          }
        
    }
       
    ssd1306_send_data(&ssd); // Atualiza o display
}

void tela_conectar_wifi() {

    adc_select_input(0); // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    adc_value_y = adc_read(); 

    sprintf(str_y, "%d", adc_value_y);  // Converte o inteiro em string

    char opcoes_conectar_wifi[2][16] = {
            "Conectar Wifi", 
            "Voltar"
        };
    uint num_elementos = sizeof(opcoes_conectar_wifi) / sizeof(opcoes_conectar_wifi[0]);

    uint8_t ret_heignt = 64 / num_elementos;
    uint8_t ret_width = 128;
    double screen_y16 = ((((4095 - ((double) adc_value_y)) / 4095) * num_elementos));  
    uint8_t screen_x = 0;
    uint8_t screen_y = (uint8_t) screen_y16;
    // if(adc_value_y > 1970 && adc_value_y < 2040) {
    //   screen_y = 0;
    // }
    // printf("adc_value_y: %u\n", adc_value_y); 
    // printf("screen_x: %u\n", screen_x); 
    // printf("screen_y: %u\n", screen_y); 
    

    
    ssd1306_fill(&ssd, !cor); // Limpa o display   


    ssd1306_rect(&ssd, screen_y * ret_heignt, screen_x, ret_width, ret_heignt, cor, 1); // Desenha um retângulo 
    for (int i = 0; i < 3; i++) {
          
          if(i == screen_y) {
            ssd1306_draw_inverted_string(&ssd, opcoes_conectar_wifi[i], (ret_width - strlen(opcoes_conectar_wifi[i]) * 8) / 2, 6 + ret_heignt * i); // Desenha uma string 
            strcpy(opcao_atual, opcoes_conectar_wifi[i]);
          } else {
            ssd1306_draw_string(&ssd, opcoes_conectar_wifi[i], (ret_width - strlen(opcoes_conectar_wifi[i]) * 8) / 2, 6 + ret_heignt * i); // Desenha uma string   
            
          }
        
    }
       
    ssd1306_send_data(&ssd); // Atualiza o display
}

void tela_tentativa_fracassada() {
    char tent_char[5];
    sprintf(tent_char, "%d", tentativas);  // Converte o inteiro em string
    ssd1306_fill(&ssd, !cor); // Limpa o display   
    ssd1306_draw_string(&ssd, "Tentativas", 24, 24); // Desenha uma string 
    ssd1306_draw_string(&ssd, "Restantes: ", 20, 34); // Desenha uma string 
    ssd1306_draw_string(&ssd, tent_char, 100, 34); // Desenha uma string       
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(2000);   
    tentativa_fracassada = false;
    tela_atual = TELA_TECLADO;
}

void tela_numero_de_tentativas_ultrapassado() {

    ssd1306_fill(&ssd, !cor); // Limpa o display   
    ssd1306_draw_string(&ssd, "Sistema Travado", 4, 20); // Desenha uma string 
    ssd1306_draw_string(&ssd, "Procure o", 28, 30); // Desenha uma string 
    ssd1306_draw_string(&ssd, "Adminstrador", 16, 40); // Desenha uma string   
    ssd1306_send_data(&ssd); // Atualiza o display 
    tentativa_fracassada = false;
    //tela_atual = TELA_SISTEMA_TRAVADO;
}


void tela_wifi_conectado() {

    ssd1306_fill(&ssd, !cor); // Limpa o display    
    ssd1306_draw_string(&ssd, "Wifi Conectado", 8, 30); // Desenha uma string  
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(2000);   

}

void tela_wifi_nao_conectado() {

    ssd1306_fill(&ssd, !cor); // Limpa o display    
    ssd1306_draw_string(&ssd, "Wifi desonectado", 0, 20); // Desenha uma string
    ssd1306_draw_string(&ssd, "Digite o nome", 12, 30); // Desenha uma string   
    ssd1306_draw_string(&ssd, "da rede", 36, 40); // Desenha uma string   
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(2000);   

}

void tela_wifi_conectando() {

    ssd1306_fill(&ssd, !cor); // Limpa o display    
    ssd1306_draw_string(&ssd, "Conectando", 24, 30); // Desenha uma string  
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(500);
    ssd1306_draw_string(&ssd, ".", 52, 40); // Desenha uma string  
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(500);
    ssd1306_draw_string(&ssd, ".", 60, 40); // Desenha uma string  
    ssd1306_send_data(&ssd); // Atualiza o display 
    sleep_ms(500);
    ssd1306_draw_string(&ssd, ".", 68, 40); // Desenha uma string  
    ssd1306_send_data(&ssd); // Atualiza o display 
    wifi_connect();         

}


// Função para lidar com a interrupção de ambos os botões
void gpio_irq_handler(uint gpio, uint32_t events)
{

  // Obtém o tempo atual em microssegundos
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  
  // Verifica se passou tempo suficiente desde o último evento
  if (current_time - last_time > 200000) // 200 ms de debouncing
  {
    last_time = current_time; // Atualiza o tempo do último evento
    
    size_t len;
    if (gpio == JOYSTICK_PB)
    { // Verifica se o botão A foi pressionado
      //gpio_put(LED_PIN_GREEN, !gpio_get(LED_PIN_GREEN));
      //printf("LED verde alternado!\n");
      switch (tela_atual)
      {
      case TELA_ABRIR_CONFIG:
        
        
        break;
      case TELA_TECLADO:
        char encrypted_password[9] = {0};

        encrypt_password(senha, encrypted_password);


        if(strcmp(encrypted_password, stored_encrypted_passord_client) == 0) {
            tentativas = 5;
            abrir_porta();
            tela_atual = TELA_ABRIR_CONFIG;
        } else if(tentativas != 0) {
            tentativas--;
            tentativa_fracassada = true;
        }
  
        break;

      case TELA_TECLADO_ADM:
        char encrypted_password_admin[50] = {0};
        //char senha[9] = {0};

        encrypt_password(senha_admin, encrypted_password_admin);


        if(strcmp(encrypted_password_admin, stored_encrypted_passord_host) == 0) {
            tentativas = 5;
            abrir_porta();
            if(conectado) {
                tela_atual = TELA_WIFI_CONECTADO;
            } else {
                tela_atual = TELA_WIFI_NAO_CONECTADO;
            }
        } else if(tentativas != 0) {
            tentativas--;
            tentativa_fracassada = true;
        }
  
        break;

      case TELA_TECLADO_WIFI:
        strcpy(wifi_ssd, wifi_ssd_digitado);
        tela_atual = TELA_TECLADO_WIFI_SENHA;
  
        break;
      case TELA_TECLADO_WIFI_SENHA:
        strcpy(wifi_pass, wifi_pass_digitado);
        tela_atual = TELA_WIFI_CONECTANDO;

        break;

      default:
        break;
      } 
    }
    if (gpio == Botao_A)
    { // Verifica se o botão B foi pressionado


      switch (tela_atual)
      {
      case TELA_ABRIR_CONFIG:
        if(strcmp(opcao_atual, "Abrir Porta") ==  0) {
          tela_atual = TELA_TECLADO;
        } else if(strcmp(opcao_atual, "Trancar Porta") ==  0) {
            fechar_porta();
            tela_atual = TELA_ABRIR_CONFIG;
        } else if(strcmp(opcao_atual, "Config") ==  0) {
            tela_atual = TELA_TECLADO_ADM;
        }
        
        
        break;
      case TELA_TECLADO:

        if(tecla_atual == '^') {
          toggleCase(teclado);
        } else if(strlen(senha) < (sizeof(senha) - 1)) {
          char temp[2] = {tecla_atual, '\0'};
          strcat(senha, temp);
        }

        // Substitui cada posição de senha por '*' em senha_oculta 
        len = strlen(senha);
        for (size_t i = 0; i < len; i++) {
            senha_oculta[i] = '*';
        }
        senha_oculta[len] = '\0'; // Finaliza a string            
        break;

      case TELA_TECLADO_ADM:

        if(tecla_atual == '^') {
          toggleCase(teclado);
        } else if(strlen(senha_admin) < (sizeof(senha_admin) - 1)) {
          char temp[2] = {tecla_atual, '\0'};
          strcat(senha_admin, temp);
        }

        // Substitui cada posição de senha_admin por '*' em senha_admin_oculta 
        len = strlen(senha_admin);
        
        for (size_t i = 0; i < len; i++) {
            senha_admin_oculta[i] = '*';
        }
        senha_admin_oculta[len] = '\0'; // Finaliza a string            
        break;
      case TELA_CONECTAR_WIFI:

        if(strcmp(opcao_atual, "Conectar Wifi") ==  0) {
          tela_atual = TELA_TECLADO_WIFI;
        } else if(strcmp(opcao_atual, "Voltar") ==  0) {
            tela_atual = TELA_ABRIR_CONFIG;
        }       
        break;
      case TELA_TECLADO_WIFI:
        if(tecla_atual == '^') {
          toggleCase(teclado);
        } else if(strlen(wifi_ssd_digitado) < (sizeof(wifi_ssd_digitado) - 1)) {
          char temp[2] = {tecla_atual, '\0'};
          strcat(wifi_ssd_digitado, temp);
        }
  
        break;

      case TELA_TECLADO_WIFI_SENHA:
        if(tecla_atual == '^') {
          toggleCase(teclado);
        } else if(strlen(wifi_pass_digitado) < (sizeof(wifi_pass_digitado) - 1)) {
          char temp[2] = {tecla_atual, '\0'};
          strcat(wifi_pass_digitado, temp);
        }

        break;            

      
      default:
        break;
      } 
  

    }
    if (gpio == botaoB)
    { // Verifica se o botão B foi pressionado

      switch (tela_atual)
      {
      case TELA_ABRIR_CONFIG:
        
        
        break;
      case TELA_TECLADO:

        if(strlen(senha) == 0) {
          tela_atual = TELA_ABRIR_CONFIG;
        } else {
          // Remove o último caracter da senha
          size_t len = strlen(senha);
          if (len > 0) {
              senha[len - 1] = '\0'; // Apaga o último caractere
          }                 
        }
        // Substitui cada posição de senha por '*' em senha_oculta 
        size_t len = strlen(senha);
        for (size_t i = 0; i < len; i++) {
            senha_oculta[i] = '*';
        }
        senha_oculta[len] = '\0'; // Finaliza a string  
  
        break;
      case TELA_TECLADO_ADM:

        if(strlen(senha_admin) == 0) {
          tela_atual = TELA_ABRIR_CONFIG;
        } else {
          // Remove o último caracter da senha_admin
          size_t len = strlen(senha_admin);
          if (len > 0) {
              senha_admin[len - 1] = '\0'; // Apaga o último caractere
          }                 
        }
        // Substitui cada posição de senha_admin por '*' em senha_admin_oculta 
        len = strlen(senha_admin);
        for (size_t i = 0; i < len; i++) {
            senha_admin_oculta[i] = '*';
        }
        senha_admin_oculta[len] = '\0'; // Finaliza a string  
  
        break;
      case TELA_TECLADO_WIFI:

        if(strlen(wifi_ssd_digitado) == 0) {
          tela_atual = TELA_ABRIR_CONFIG;
        } else {
          // Remove o último caracter da wifi_ssd_digitado
          size_t len = strlen(wifi_ssd_digitado);
          if (len > 0) {
              wifi_ssd_digitado[len - 1] = '\0'; // Apaga o último caractere
          }                 
        } 
  
        break;
      case TELA_TECLADO_WIFI_SENHA:

        if(strlen(wifi_pass_digitado) == 0) {
          tela_atual = TELA_ABRIR_CONFIG;
        } else {
          // Remove o último caracter da wifi_pass_digitado
          size_t len = strlen(wifi_pass_digitado);
          if (len > 0) {
              wifi_pass_digitado[len - 1] = '\0'; // Apaga o último caractere
          }                 
        }

  
        break;

      default:
        break;
      } 
  

    }

    
  }
}

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_enabled(slice_num, true);  
    return slice_num;  
}
uint wifi_init() {
    encrypt_password(INITIAL_STORED_PASSWORD, stored_encrypted_passord_host);
    encrypt_password(INITIAL_STORED_PASSWORD_CLIENT, stored_encrypted_passord_client);
    printf("Iniciando servidor HTTP\n");

    ssd1306_fill(&ssd, !cor); // Limpa o display    
    ssd1306_draw_string(&ssd, "Iniciando HTTP", 8, 10);
    ssd1306_send_data(&ssd);
    sleep_ms(500);

    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");   
        ssd1306_draw_string(&ssd, "Erro ao iniciar", 4, 20);
        ssd1306_draw_string(&ssd, "WiFi", 48, 30);
        ssd1306_send_data(&ssd);
        sleep_ms(500);
        return 1;
    }
}

uint wifi_connect() {
    if (conectado) {
        // Se o Wi-Fi já estiver inicializado, não reinicializa
        return 0;
    }

    uint len;
    char endereco_ip[15];
    //wifi_init();

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");
    ssd1306_draw_string(&ssd, "Conectando Wifi", 0, 20);
    ssd1306_send_data(&ssd);

    if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssd, wifi_pass, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        ssd1306_draw_string(&ssd, "Falha ao", 32, 30);
        ssd1306_draw_string(&ssd, "Conectar Wifi", 8, 40);
        ssd1306_send_data(&ssd);
        sleep_ms(10000);
        return 1;
    } else {
        printf("Connected.\n");
        ssd1306_draw_string(&ssd, "Conectado", 28, 30);
        ssd1306_send_data(&ssd);
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

        sprintf(endereco_ip, "%d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        len = strlen(endereco_ip);
        ssd1306_draw_string(&ssd, "Endereco IP", 20, 30);
        ssd1306_draw_string(&ssd, endereco_ip, (128 - len * 8) / 2, 50);
        ssd1306_send_data(&ssd);
        sleep_ms(2000);
    }
    conectado = true;
    //wifi_inicializado = true;  // Marca que o Wi-Fi já foi configurado

    printf("Wi-Fi conectado!\n");
    printf("Acesse a página para efetuar login e controlar a porta.\n");
    ssd1306_fill(&ssd, !cor);
    ssd1306_draw_string(&ssd, "WiFi conectado", 8, 20);
    ssd1306_draw_string(&ssd, "Acesse", 40, 30);
    ssd1306_draw_string(&ssd, endereco_ip, (128 - len * 8) / 2, 40);
    ssd1306_draw_string(&ssd, "Para login", 24, 50);
    ssd1306_send_data(&ssd);
    sleep_ms(10000);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    start_http_server();

    return 0;
}

uint setup() {

  stdio_init_all();
  sleep_ms(2000);
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  gpio_init(JOYSTICK_PB);
  gpio_set_dir(JOYSTICK_PB, GPIO_IN);
  gpio_pull_up(JOYSTICK_PB); 
  gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  //ssd1306_t ssd; // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(JOYSTICK_X_PIN);
  adc_gpio_init(JOYSTICK_Y_PIN);  

  //adc_gpio_init(VRX_PIN); 
  gpio_init(LED_PIN_RED);
  gpio_set_dir(LED_PIN_RED, GPIO_OUT);
  pwm_set_gpio_level(LED_PIN_RED, 0);

  gpio_init(LED_PIN_GREEN);
  gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);  
  pwm_set_gpio_level(LED_PIN_GREEN, 0); 

  gpio_init(LED_PIN_BLUE);
  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);  
  pwm_set_gpio_level(LED_PIN_BLUE, 0); 

  pwm_slice_red = pwm_init_gpio(LED_PIN_RED, pwm_wrap);  
  pwm_slice_blue = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);
  wifi_init();
  wifi_connect();

  return 0;

}




int main() {
    setup();
    // Loop principal
    while (true) {

        if(tentativas == 0) {
            // tela_numero_de_tentativas_ultrapassado();
            // printf("\nTentativas: %d", tentativas);
            //continue;
            tela_atual = TELA_SISTEMA_TRAVADO;
        } else if(tentativa_fracassada) {
            tela_atual = TELA_TENTATIVAS_FRACASSADAS;       
        }

        

        // if(tela_atual == TELA_ABRIR_CONFIG) {
        // tela_abrir_config();
        // } else {
        // tela_teclado("SENHA: ");
        // }
        switch (tela_atual)
        {
        case TELA_ABRIR_CONFIG:
            tela_abrir_config();
            break;
        case TELA_TECLADO:
            tela_teclado("Senha: ");
            break;
        case TELA_TENTATIVAS_FRACASSADAS:
            tela_tentativa_fracassada();
            break;
        case TELA_SISTEMA_TRAVADO:
            tela_numero_de_tentativas_ultrapassado();
            if(tentativas > 0) {
                // tela_numero_de_tentativas_ultrapassado();
                // printf("\nTentativas: %d", tentativas);
                //continue;
                tela_atual = TELA_ABRIR_CONFIG;
            }            
            break;
        case TELA_TECLADO_ADM:
            tela_teclado("Admin: ");
            break; 
        case TELA_WIFI_CONECTADO:
            tela_wifi_conectado();
            tela_atual = TELA_ABRIR_CONFIG;
            break; 
        case TELA_WIFI_NAO_CONECTADO:
            tela_wifi_nao_conectado();
            tela_atual = TELA_CONECTAR_WIFI;
            break;
        case TELA_CONECTAR_WIFI:
            tela_conectar_wifi();
            break;              
        case TELA_TECLADO_WIFI:
            tela_teclado("SSID: ");
            break; 
        case TELA_TECLADO_WIFI_SENHA:
            tela_teclado("Senha: ");
            break;
        case TELA_WIFI_CONECTANDO:
            tela_wifi_conectando();
            tela_atual = TELA_ABRIR_CONFIG;
            break;    
        default:
            break;
        }        

        //////////////////////////Network///////////////////////////////////////////
        cyw43_arch_poll();
        sleep_ms(100);
        //////////////////////////Network-end/////////////////////////////////////////// 
    }
    //////////////////////////Network///////////////////////////////////////////
    cyw43_arch_deinit();
    //////////////////////////Network-end/////////////////////////////////////////// 
    return 0;
}



//////////////////////////////////////////////////////////////