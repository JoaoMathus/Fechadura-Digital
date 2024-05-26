#define LARGURA_SENHA 4
#define QUANTIDADE_SENHAS 5
#define TAMANHO_MEMORIA 20

// Para testes, apenas.
char memoria[] = {
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
};
// Endereço de cada senha na memória.
// Criado apenas para o código ficar mais legível.
const int senha_enderecos[] = {
  0, 4, 8, 12, 16
};

// Importante: a senha de adm ficará sempre
// no endereço zero.
// Logo, para ter a senha de adm:
// resgatar_senha(0, adm_senha);
char adm_senha[LARGURA_SENHA] = {'1', '2', '3', '4'};
char senha_teste[] = { '4', '3', '2', '1' };

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
    
    if (salvar_senha(adm_senha))
      adm_logou = true; // funcionalidades disponíveis
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
      // Morreu a senha
    } else {
      // Existe não hein
    }
  } else {
    // Se não é ADM, lamento...
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
  for (int i = 0; i < TAMANHO_MEMORIA; i++) {
    if (memoria[i] != ' ')
      return false;
  }
  return true;
}

// Percorre a memória para encontrar um espaço livre.
// Caso não encontre, retorna -1.
int pegar_endereco_livre() {
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (memoria[i] == ' ')
      return i;
  }
  return -1; // não há endereços livres
}

bool salvar_senha(char senha[]) {
  int endereco = pegar_endereco_livre();
  if (endereco < 0) return false; // não há endereços livres
  
  for (int i = 0; i < LARGURA_SENHA; i++) {
    memoria[endereco+i] = senha[i]; // escrevendo na memória
  }
  
  return true;
}

void resgatar_senha(int endereco, char destino[]) {
  for (int i = 0; i < LARGURA_SENHA; i++) {
    destino[i] = memoria[endereco+i];
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
    resgatar_senha(senha_enderecos[i], s);
    if (senhas_iguais(senha, s))
      return true;
  }
  return false;
}

bool apagar_senha(char senha[]) {
  char s[LARGURA_SENHA];
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    resgatar_senha(senha_enderecos[i], s);
    if (senhas_iguais(senha, s)) {
      for (int j = 0; j < LARGURA_SENHA; j++) {
        memoria[senha_enderecos[i]+j] = ' ';
      }
      return true;
    }
  }
  return false; // senha não existe
}
