


<img width=100% src="https://capsule-render.vercel.app/api?type=waving&color=FCE7C8&height=120&section=header"/>

<h1 align="center">Embarcatec | Projeto Final</h1>

<div align="center">  
  <img width=40% src="http://img.shields.io/static/v1?label=STATUS&message=FINALIZADO&color=B1C29E&style=for-the-badge"/>
</div>

## Objetivo do Projeto

Esse é um sistema, que usando a placa Bitdoglab ou o Raspberry PI Pico W, tranca e destranca uma porta para Airbnb ou similares, é possível controlar a porta tanto usando a placa quanto via Wi-Fi, com mudança de senha feita apenas por Wi-Fi. Um led foi usado para simbolizar a abertura da porta.


## 🗒️ Lista de requisitos

- Cabo USB
- Placa Bitdoglab ou os itens abaixo:
    - Protoboard;
    - Joystick Analógico (Plugin 13x13mm Multi-Dir ROHS);
    - Display OLED ssd1306 (0.96 polegadas I2C 128x64 oled display);
    - 3 Resistores de 1kΩ;
    - 1 LED RGB;
    - Fios e jumpers; 
    - Microcontrolador Raspberry Pi Pico W; 

## 🛠 Tecnologias

1. **Git e Github**;
2. **VScode**;
3. **Linguagem C**;
4. **Extensões no VScode do Raspberry Pi Pico Project e CMake**

## 💻 Instruções para Importar, Compilar e Rodar o Código Localmente

Siga os passos abaixo para clonar o repositório, importar no VS Code usando a extensão do **Raspberry Pi Pico Project**, compilar e executar o código.

1. **Clone o repositório para sua máquina local**  
   Abra o terminal e execute os comandos abaixo:
   ```bash
   git clone https://github.com/alexsami-lopes/EmbarcaTech-Projeto-Final.git
   cd EmbarcaTech-Projeto-Final

2. **Abra o VS Code e instale a extensão "Raspberry Pi Pico Project" (caso não já a tenha instalada)**
 - No VS Code, vá até "Extensões" (Ctrl+Shift+X)
 - Pesquise por "Raspberry Pi Pico Project"
 - Instale a extensão oficial

3. **Importe o projeto no VS Code**
 - No VS Code, na barra lateral do lado esquerdo clique em "Raspberry Pi Pico Project" <img src="images/icon_raspberry_pico_project.png" width="25px">
 - No menu que aparecer clique em <img src="images/icon_import_project.png" height="25px">
 - Clicando em "Change" escolha a pasta clonada do repositório
 - Escolha a versão do SDK 2.1.0
 - Clique em "Import"


    <img src="images/icon_import_project_settings.png" width="500px">


4. **Compile o projeto**
 - Com o projeto aberto no VS Code, pressione <img src="images/icon_compile.png" height="25px">
 - Aguarde a finalização do processo de build

5. **Rode o código no Raspberry Pi Pico**
 - Conecte o Raspberry Pi Pico ao PC segurando o botão "BOOTSEL".
 - Arraste e solte o arquivo `.uf2`, localizado dentro da pasta "build" do seu projeto, gerado na unidade USB montada.
 - O código será carregado automaticamente e o Pico será reiniciado.
 - Caso tenha instalado o driver com o Zadig clique em "Run" ao lado do botão <img src="images/icon_compile.png" height="25px">


## 🔧 Funcionalidades Implementadas:

O joystick fornece valores analógicos correspondentes aos eixos X e Y, que são utilizados para controlar os menus onde é possível:
1. Abrir a porta (ligar o led).
2. Fechar a porta (desligar o led).
3. Conectar ao WiFi.
4. Ver o IP do host.

Via host acessando o IP dado ao conectar:
1. Fazer Login.
2. Mudar senhas.
3. Abrir a porta (ligar o led).
4. Fechar a porta (desligar o led).
5. Desbloquear o dispositivo após 5 tentativas de senhas erradas (basta clicar em Abrir Porta).

## 💻 Desenvolvedor
 
<table>
  <tr>

<td align="center"><img style="" src="https://avatars.githubusercontent.com/u/103523809?v=4" width="100px;" alt=""/><br /><sub><b> Alexsami Lopes </b></sub></a><br />👨‍💻</a></td>

  </tr>
</table>




## 🎥 Demonstração na Placa (Video): 

<div align="center">
  <a href="https://youtu.be/MJKoZzyWy5I" target="_blank">
    <img src="images/Demo_Placa_Video.png" width="500px">
  </a>
</div>
