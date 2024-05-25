#define LARGURA_SENHA 4
#define QUANTIDADE_SENHAS 5
#define TAMANHO_MEMORIA 20

// Para testes, apenas.
int memoria[] = {
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
};
// Endereço de cada senha na memória.
// Criado apenas para o código ficar mais legível.
const int senha_enderecos[] = {
  0, 4, 8, 12, 16
};

bool porta_aberta = false;

// Importante: a senha de adm ficará sempre
// no endereço zero.
// Logo, para ter a senha de adm:
// resgatar_senha(0, adm_senha);
int adm_senha[LARGURA_SENHA] = {1, 2, 3, 4};
int senha_teste[] = { 4, 3, 2, 1 };


int senha_para_int(int[]);
bool memoria_vazia();
int pegar_endereco_livre();
bool salvar_senha(int[]);
bool senhas_iguais(int[], int[]);
void resgatar_senha(int, int[]);
bool senha_valida(int[]);

void setup() {
  Serial.begin(9600);
  
  if (memoria_vazia())
    if (salvar_senha(adm_senha))
      Serial.println("Senha de adm salva.");
    else
      Serial.println("Deu erro, amigo. Ferrou!");
  
  // Pegando a senha de adm
  resgatar_senha(0, adm_senha);
  Serial.print("Senha de adm: ");
  Serial.println(senha_para_int(adm_senha));
  
  if (senhas_iguais(senha_teste, adm_senha))
    Serial.println("Eh o adm");
  else
    Serial.println("Eh um qualquer");
  
  if(!salvar_senha(senha_teste))
    Serial.println("Deu M");
  
  if (senha_valida(senha_teste))
    porta_aberta = true;
  
  // Abrindo a porta
  if (porta_aberta)
    Serial.println("Abriu-se a porta!");
  else
    Serial.println("Escreveu errado, amigo.");
}

void loop() {
  
}

// Converte o array de inteiros para um número.
int senha_para_int(int arr[]) {
  int n = 0;
  int casa = 1;
  
  for (int i = LARGURA_SENHA-1; i >= 0; i--) {
    n += arr[i] * casa;
    casa *= 10;
  }
  
  return n;
}

bool memoria_vazia() {
  for (int i = 0; i < TAMANHO_MEMORIA; i++) {
    if (memoria[i] > 0)
      return false;
  }
  return true;
}

// Percorre a memória para encontrar um espaço livre.
// Caso não encontre, retorna -1.
int pegar_endereco_livre() {
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    if (memoria[i] < 0)
      return i;
  }
  return -1; // não há endereços livres
}

bool salvar_senha(int senha[]) {
  int endereco = pegar_endereco_livre();
  if (endereco < 0) return false; // não há endereços livres
  
  for (int i = 0; i < LARGURA_SENHA; i++) {
    memoria[endereco+i] = senha[i]; // escrevendo na memória
  }
  
  return true;
}

bool senhas_iguais(int s1[], int s2[]) {
  for (int i = 0; i < LARGURA_SENHA; i++) {
    if (s1[i] != s2[i])
      return false;
  }
  return true;
}

void resgatar_senha(int endereco, int destino[]) {
  for (int i = 0; i < LARGURA_SENHA; i++) {
    destino[i] = memoria[endereco+i];
  }
}

// Compara uma senha com todas salvas
bool senha_valida(int senha[]) {
  int s[LARGURA_SENHA];
  for (int i = 0; i < QUANTIDADE_SENHAS; i++) {
    resgatar_senha(senha_enderecos[i], s); // pegando a próxima senha
    if (senhas_iguais(senha, s))
      return true;
  }
  return false;
}
