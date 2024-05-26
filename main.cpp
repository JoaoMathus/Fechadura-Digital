#include <EEPROM.h>

#define LARGURA_SENHA 4
#define QUANTIDADE_SENHAS 5

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

// Importante: a senha de adm ficará sempre
// no endereço zero.
// Logo, para ter a senha de adm:
// resgatar_senha(0, adm_senha);
char adm_senha[LARGURA_SENHA] = {'1', '2', '3', '4'};
char senha_teste[] = {'4', '3', '2', '1'};

String senha_para_string(char[]);
bool memoria_vazia();
int pegar_endereco_livre();
bool salvar_senha(char[]);
void resgatar_senha(int, char[]);
bool senhas_iguais(char[], char[]);
bool validar_senha(char[]);
bool apagar_senha(char[]);

bool porta_aberta = false;
bool adm_logou = false;

void setup() {
  Serial.begin(9600);
  
  if (memoria_vazia()) {
    // Lógica para registrar a primeira senha
    // ...
    Serial.println("Pegando a primeria senha");
    
    if (salvar_senha(adm_senha)) {
      adm_logou = true; // funcionalidades disponíveis
      Serial.println("Senha de adm salva.");
    }
  }
  
  // Botão para abrir a porta
  // ...
  if (adm_logou || validar_senha(senha_teste)) {
    porta_aberta = true;
  }
  
  // Botão para adicionar nova senha
  // ...
  if (adm_logou) {
    // Pegar a senha pelo input
    // ...
    
    if (salvar_senha(senha_teste)) {
      porta_aberta = true;
      Serial.println("Senha de teste salva.");
    } else {
      // É, fica por conta do output
    }
  } else {
    // Se não é ADM, lamento...
  }
  
  // Botão para remover uma senha
  // ...
  if (adm_logou) {
    // Pega a senha pelo input
    // ...
    
    if(apagar_senha(senha_teste)) {
      Serial.println("Senha de teste apagada.");
    } else {
      // Existe não hein
    }
  } else {
    // Se não é ADM, lamento...
  }
  
  // Debug
  char debug_senha[LARGURA_SENHA];
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (senhas_usadas[i][1]) {
      resgatar_senha(senhas_usadas[i][0], debug_senha);
      Serial.println(senha_para_string(debug_senha));
    }
  }
  
}

void loop() {
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
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (senhas_usadas[i][1] > 0)
      return false;
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
