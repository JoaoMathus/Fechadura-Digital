#include <EEPROM.h>
#include <Adafruit_LiquidCrystal.h>
#include <Keypad.h>

#define LARGURA_SENHA 4
#define QUANTIDADE_SENHAS 5

Adafruit_LiquidCrystal lcd(0);
int buzzer = 9;

int red_pin = 7;
int green_pin = 5;
int blue_pin = 6;

// Guarda o endereço das senhas
// e se está em uso, ou não
// (0 -> não está em uso, 1 -> em uso)
int senhas_usadas[QUANTIDADE_SENHAS][2] {
  {0, 0},
  {4, 0},
  {8, 0},
  {12, 0},
  {16, 0}
};

const byte linhas = 4;
const byte colunas = 4;
int lcd_coluna = 0;

//Matriz com os valores de cada tecla 
char teclas[linhas][colunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinos_linhas[] = {4, 3, 2, 1}; 
byte pinos_colunas[] = {13, 12, 11, 10}; 

// Criando objeto teclado
Keypad teclado = Keypad(makeKeymap(teclas), pinos_linhas, pinos_colunas, linhas, colunas);

char tecla_temp;
  
// Importante: a senha de adm ficará sempre
// no endereço zero.
// Logo, para ter a senha de adm:
// resgatar_senha(0, senha_destino);
char input_senha[LARGURA_SENHA]; // senha digitada

String senha_para_string(char[]);
bool memoria_vazia();       // verifica se é primeiro uso
int pegar_endereco_livre(); // pega o próximo endereço vazio
bool salvar_senha(char[]);
void resgatar_senha(int, char[]);
bool senhas_iguais(char[], char[]);
bool validar_senha(char[]); // verifica se a senha existe
bool apagar_senha(char[]);
void apaga_todas_senhas();
bool verificar_adm(char[]);

bool porta_aberta = false;
bool adm_logou = false; // condição para adicionar/remover senhas

void lcd_print(String, int);
void blink_color(int , int , int);
void senha_correta_behavior();
void senha_incorreta_behavior();

void ler_senha();
bool validador_botao(char);

void setup() {
  //Serial.begin(9600);
  //Configuração dos pinos RGB
  pinMode(red_pin,  OUTPUT);              
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  
  //Configuração Buzzer
  pinMode(buzzer, OUTPUT);
  
  //Configuração LCD
  lcd.begin(16, 2);
  lcd.clear();
  
  // Primeiro uso da fechadura
  // (Se não há senhas, ou se não há ADM)
  if (memoria_vazia()) {
    // Ler senha do teclado e guardar em input_senha
    ler_senha();
    
    // Salvando a senha de administrador
    if (salvar_senha(input_senha)) {
      adm_logou = true; // funcionalidades disponíveis
      Serial.println("Senha de adm salva.");
      lcd_print("Senha de ADM", 0);
      lcd_print("Salva", 1);
    }
  }
}

void loop() {
  // Pegar senha do teclado e guardar em input_senha
  ler_senha();
  tecla_temp = teclado.waitForKey();
  
  if (tecla_temp == 'C') {
    // Quem entrou a senha é ADM?
    adm_logou = verificar_adm(input_senha);
  
  	if (adm_logou || validar_senha(input_senha)) {
      porta_aberta = true;
      senha_correta_behavior();
    }
  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pressione");
    lcd.setCursor(0,1);
    lcd.print("uma tecla.");
    tecla_temp = teclado.waitForKey();
  
    // Salvar nova senha.
    if (tecla_temp == 'B') {
      if (adm_logou) {
        // Pegar a senha pelo input
        ler_senha();
    
        if (salvar_senha(input_senha)) {
          porta_aberta = true;
          lcd_print("Senha salva", 0);
        } else {
          lcd_print("Erro ao salvar", 0);
        }
      } else {
        // Se não é ADM, lamento...
        lcd_print("Usuario nao e", 0);
        lcd_print("Administrador", 1);
      } 
    }
  
    // Botão para remover uma senha
    if (tecla_temp == '*') {
      if (adm_logou) {
        // Pega a senha pelo input
        ler_senha();
    
        if(apagar_senha(input_senha)) {
          lcd_print("Senha apagada", 0);
        } else {
          lcd_print("Senha nao existe", 0);
        }
      } else {
        // Se não é ADM, lamento...
        lcd_print("Usuario nao e", 0);
        lcd_print("Administrador", 1);
      }
    }
    
    // Botão apagar todas as senhas
    if (tecla_temp == '#') {
      if (adm_logou) {
        apagar_todas_senhas();
        lcd_print("Tudo apagado...", 0);
      }
    }
  }
}

// Converte o array de char para String
String senha_para_string(char senha[]) {
  String s = String();
  for (int i = 0; i < LARGURA_SENHA; i++) {
    s += senha[i];
  }
  
  return s;
}

bool memoria_vazia() {
  // Verificando a memória.
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    // Se tiver algo escrito, marca como usado
    // o endereço.
    if (EEPROM.read(senhas_usadas[i][0]) > 0) {
      senhas_usadas[i][1] = 1;
    }
  }
  
  // Verificando se há adm
  if (senhas_usadas[0][1] > 0)
    return false;
  
  // Loop para verificar se há pelo menos uma senha
  // sem ser a de adm.
  for (int i = 1; i < QUANTIDADE_SENHAS; i++) {
    if (senhas_usadas[i][1] == 1) {
      return false;
    }
  }
  
  return true;
}

// Retorna o próximo endereço livre,
// caso contrário retorna -1.
int pegar_endereco_livre() {
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (senhas_usadas[i][1] == 0)
      return senhas_usadas[i][0]; // retornando o endereço
  }
  
  return -1;
}

bool salvar_senha(char senha[]) {
  int endereco = pegar_endereco_livre();
  if (endereco < 0) return false; // não há endereços livres
  
  // Marcando o endereço como usado.
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (senhas_usadas[i][0] == endereco)
      senhas_usadas[i][1] = 1;
  }
  
  // Salvando a senha.
  for (int i = 0; i < LARGURA_SENHA; i++) {
    EEPROM.update(endereco+i, senha[i]);
  }
  
  return true;
}

void resgatar_senha(int endereco, char destino[]) {
  for (int i = 0; i < LARGURA_SENHA; i++) {
    destino[i] = EEPROM.read(endereco+i);
  }
}

bool senhas_iguais(char s1[], char s2[]) {
  for (int i = 0; i < LARGURA_SENHA; i++) {
    if (s1[i] != s2[i])
      return false;
  }
  return true;
}

// Compara uma senha com todas salvas
bool validar_senha(char senha[]) {
  char s[LARGURA_SENHA];
  
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    resgatar_senha(senhas_usadas[i][0], s);
    if (senhas_iguais(senha, s))
      return true;
  }
  
  return false;
}

bool apagar_senha(char senha[]) {
  char s[LARGURA_SENHA];
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    resgatar_senha(senhas_usadas[i][0], s);
    if (senhas_iguais(senha, s)) {
      senhas_usadas[i][1] = 0; // marcando como não usada.
      return true; // sucesso na operação.
    }
  }
  
  return false; // não existe essa senha.
}

// Apaga todas as senhas exceto a senha de
// administrador
void apagar_todas_senhas() {
  for (int i = 4; i < QUANTIDADE_SENHAS-1; i++) {
    senhas_usadas[i][1] = 0;
  }
}

bool verificar_adm(char senha[]) {
  char adm[LARGURA_SENHA];
  
  // Pegando a senha de adm
  resgatar_senha(senhas_usadas[0][1], adm);
  
  return senhas_iguais(senha, adm);
}

void lcd_print(String texto, int pos) {
	lcd.clear();
  	lcd.setCursor(0, pos);
  	lcd.print(texto);
}

void blink_color(int red_value, int green_value, int blue_value) {
	analogWrite(red_pin, red_value);
  	analogWrite(green_pin, green_value);
  	analogWrite(blue_pin, blue_value);
  	delay(3000);
  	analogWrite(red_pin, 0);
  	analogWrite(green_pin, 0);
  	analogWrite(blue_pin, 0);
}

void senha_incorreta_behavior() {
  	lcd_print("Senha incorreta", 0);
  	tone(buzzer, 90, 700);
  	blink_color(255, 0, 0);
}

void senha_correta_behavior() {
  	lcd_print("Porta aberta", 0);
  	tone(buzzer, 390, 300);
  	delay(350);
  	tone(buzzer, 400, 500);
  	blink_color(0, 255, 0);
}

void ler_senha() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Insira Senha: ");
  while(true) {
  // Posicionando o cursor antes de coletar o valor pressionado
  lcd.setCursor(lcd_coluna, 1);
  // Pegando o valor pressionado
  char pressionado = teclado.waitForKey();
  // Salvando o valor pressionado em uma lista de char e 
  // colocando um * na tela
  if (validador_botao(pressionado)) {
    lcd.print('*');
    input_senha[lcd_coluna] = pressionado; 
    lcd_coluna += 1;
  }
  // Se a tecla A for apagada, volta o cursor em uma posição
  // e printa um espaço para poder apagar o valor ali.
  else if (pressionado == 'A') {
    lcd_coluna -= 1;
    lcd.setCursor(lcd_coluna, 1);
    lcd.print(' ');
  }
  // Se a tecla D for pressionada, apaga tudo que está na tela e
  // reseta a posição do cursor.
  else if (pressionado == 'D') {
  	lcd.clear();
    lcd.print("Insira Senha: ");
    lcd_coluna = 0;
  }
  // Se a senha for completamente preenchida, 
  // retorna para fora da função
  if (lcd_coluna == LARGURA_SENHA) {
    lcd_coluna = 0;
    //Serial.println(input_senha);
    return;
  }
}
}

bool validador_botao(char t) {
  // Checando se a tecla pressionado é válida ou não
  if (t == 'A' or t == 'B' or t == 'C' or t == 'D' or t == '*' or t == '#') {
    return false;
  }
  else {
    return true;
  }
}
