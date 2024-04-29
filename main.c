#include <EEPROM.h>   // para EEPROM.write(),
					            //      EEPROM.read(),
						          //      EEPROM.update()

#define SENHA_LARGURA 4

// Senha para testes internos
int senha_teste[SENHA_LARGURA] = {
  1, 2, 3, 4
};

void setup() {
  Serial.begin(9600);
  // Salvando senha de testes
  salvar_senha(senha_teste, SENHA_LARGURA);
}

void loop() {
  // Adquirindo senha armazenada na EEPROM
  int senha_secreta[SENHA_LARGURA];
  resgatar_senha(senha_secreta, SENHA_LARGURA);
  
  // Imprimindo a senha
  for (int i = 0; i < SENHA_LARGURA; i++) {
    Serial.print(senha_secreta[i]);
    Serial.print(" ");
  }
  Serial.println();
}

// Parâmetros: 	senha, int[]
// 				      largura, int
// Armazena a senha passada como argumento na EEPROM
void salvar_senha(int senha[], int largura) {
  for (int i = 0; i < largura; i++) {
    EEPROM.update(i, senha[i]);
  }
}

// Parâmetros: 	destino, int[]
// 				      largura, int
// Adquire a senha armazenada na EEPROM e salva no destino
void resgatar_senha(int destino[], int largura) {
  for (int i = 0; i < largura; i++) {
    destino[i] = EEPROM.read(i);
  }
}
