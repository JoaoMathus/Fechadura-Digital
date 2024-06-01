#include <EEPROM.h>
#include <Adafruit_LiquidCrystal.h>
#include <Keypad.h>

#define LARGURA_SENHA 4
#define QUANTIDADE_SENHAS 5
#define ADM_ENDERECO 0 // endereço da senha de administrador
#define TENTATIVAS_MAXIMAS 5

// Globais
Adafruit_LiquidCrystal lcd(0);
int buzzer = 9;
int red_pin = 7;
int green_pin = 5;
int blue_pin = 6;
const byte linhas = 4;
const byte colunas = 4;
int lcd_coluna = 0;
byte pinos_linhas[] = {4, 3, 2, 1};
byte pinos_colunas[] = {13, 12, 11, 10};
// Matriz com os valores de cada tecla
char teclas[linhas][colunas] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
// Criando objeto teclado
Keypad teclado = Keypad(makeKeymap(teclas), pinos_linhas, pinos_colunas, linhas, colunas);
char tecla_temp;
char input_senha[LARGURA_SENHA];
bool porta_aberta = false;
bool adm_logou = false;
int tentativas = 0;
unsigned long segundos = 1000UL;
unsigned long minutos = segundos * 60;

// Guarda o endereço das senhas.
int enderecos_senhas[QUANTIDADE_SENHAS]{
    ADM_ENDERECO,
    ADM_ENDERECO + LARGURA_SENHA,
    ADM_ENDERECO + 2 * LARGURA_SENHA,
    ADM_ENDERECO + 3 * LARGURA_SENHA,
    ADM_ENDERECO + 4 * LARGURA_SENHA};

// Procedimentos
// String senha_para_string(char[]); usado na antiguidade do projeto para debug
bool tem_adm();             // verifica se é primeiro uso
int pegar_endereco_livre(); // pega o próximo endereço vazio
bool salvar_senha(char[]);
bool resgatar_senha(int, char[]);
bool senhas_iguais(char[], char[]);
bool validar_senha(char[]); // verifica se a senha existe
bool apagar_senha(char[]);
void modificar_adm_senha(char[]);
void apagar_todas_senhas(); // apaga todas, exceto a de administrador
bool verificar_adm(char[]); // verifica se a senha é a de administrador
void fechar_porta(int);     // simula o evento da porta ser fechada
void lcd_print(String, int);
void blink_color(int, int, int);
void senha_correta_behavior();
void senha_adm_behavior();
void senha_incorreta_behavior();
void ler_senha();
bool tem_certeza();
bool validador_botao(char);

void setup()
{
    // Configuração dos pinos RGB
    pinMode(red_pin, OUTPUT);
    pinMode(green_pin, OUTPUT);
    pinMode(blue_pin, OUTPUT);

    // Configuração Buzzer
    pinMode(buzzer, OUTPUT);

    // Configuração LCD
    lcd.begin(16, 2);
    lcd.clear();

    // Primeiro uso da fechadura
    if (tem_adm())
    {
        lcd_print("Primeiro uso", 0);
        delay(600);

        // Ler senha do teclado e guardar em input_senha
        ler_senha();

        // Salvando a senha de administrador
        if (salvar_senha(input_senha))
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Senha de ADM");
            lcd.setCursor(0, 1);
            lcd.print("salva...");
            delay(1000);
        }
    }
}

void loop()
{
    porta_aberta = false;

    if (tentativas >= TENTATIVAS_MAXIMAS)
    {
        tentativas = 0;
        lcd_print("PORTA TRAVADA...", 0);
        delay(20 * minutos);
    }

    // Pegar senha do teclado e guardar em input_senha
    ler_senha();

    // Quem entrou a senha é administrador?
    adm_logou = verificar_adm(input_senha);

    if (adm_logou)
    {
        tentativas = 0;
        senha_adm_behavior();

        while (adm_logou)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Pressione");
            lcd.setCursor(0, 1);
            lcd.print("uma tecla.");
            tecla_temp = teclado.waitForKey();

            // Abre a porta
            if (tecla_temp == 'A')
            {
                porta_aberta = true;
                adm_logou = false;
                senha_correta_behavior();
                fechar_porta(3000);
            }

            // Salvar nova senha.
            if (tecla_temp == 'B')
            {
                ler_senha();

                if (salvar_senha(input_senha))
                {
                    lcd_print("Senha salva", 0);
                    delay(700);
                }
                else
                {
                    lcd_print("Erro ao salvar", 0);
                }
            }

            // Nova senha de adm
            if (tecla_temp == 'D')
            {
                modificar_adm_senha(input_senha);
                lcd_print("Senha adm salva", 0);
                delay(700);
            }

            // Botão para remover uma senha
            if (tecla_temp == '*')
            {
                // Pega a senha pelo input
                ler_senha();

                if (tem_certeza())
                {
                    if (apagar_senha(input_senha))
                    {
                        lcd_print("Senha apagada", 0);
                        delay(700);
                    }
                    else
                    {
                        lcd_print("Senha nao existe", 0);
                        delay(700);
                    }
                }
                else
                {
                    continue;
                }
            }

            // Botão apagar todas as senhas
            if (tecla_temp == '#')
            {
                if (tem_certeza())
                {
                    apagar_todas_senhas();
                    lcd_print("Tudo apagado...", 0);
                    delay(700);
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }
    else
    {
        // Usuário normal pode apenas abrir a porta
        if (validar_senha(input_senha))
        {
            tentativas = 0;
            porta_aberta = true;
            senha_correta_behavior();
            fechar_porta(3000);
        }
        else
        {
            // Senha errada
            lcd.clear();
            senha_incorreta_behavior();
            tentativas++;
        }
    }
}

// INFO: Função usada na antiguidade do projeto para depuragem.
// // Converte o array de char para String
// String senha_para_string(char senha[])
// {
//     String s = String();
//     for (int i = 0; i < LARGURA_SENHA; i++)
//     {
//         s += senha[i];
//     }
//
//     return s;
// }

// Verifica se há administrador.
bool tem_adm()
{
    // Verificando se há adm
    if (EEPROM.read(enderecos_senhas[0]) != 0)
        return false;

    return true;
}

// Retorna o próximo endereço livre,
// caso contrário retorna -1.
int pegar_endereco_livre()
{
    for (int i = 0; i < QUANTIDADE_SENHAS; i++)
    {
        if (EEPROM.read(enderecos_senhas[i]) == 0)
            return enderecos_senhas[i];
    }

    return -1;
}

// Salvando a senha na EEPROM.
bool salvar_senha(char senha[])
{
    char adm[LARGURA_SENHA];

    resgatar_senha(0, adm);
    if (senhas_iguais(senha, adm))
        return false;

    int endereco = pegar_endereco_livre();
    if (endereco < 0)
        return false; // não há endereços livres

    for (int i = 0; i < QUANTIDADE_SENHAS; i++)
    {
        if (enderecos_senhas[i] == endereco)
        {
            // Salvando a senha.
            for (int j = 0; j < LARGURA_SENHA; j++)
            {
                EEPROM.update(endereco + j, senha[j]);
            }
        }
    }

    return true;
}

// Resgada uma senha da memória e armazena em "destino"
bool resgatar_senha(int endereco, char destino[])
{
    for (int i = 0; i < QUANTIDADE_SENHAS; i++)
    {
        if ((enderecos_senhas[i] == endereco) &&
            EEPROM.read(enderecos_senhas[i]) != 0)
        {
            for (int j = 0; j < LARGURA_SENHA; j++)
            {
                destino[j] = EEPROM.read(endereco + j);
            }
            return true;
        }
    }
    return false;
}

// Compara duas senhas.
bool senhas_iguais(char s1[], char s2[])
{
    for (int i = 0; i < LARGURA_SENHA; i++)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}

// Compara uma senha com todas salvas.
bool validar_senha(char senha[])
{
    char s[LARGURA_SENHA];

    for (int i = 0; i < QUANTIDADE_SENHAS; i++)
    {
        if (resgatar_senha(enderecos_senhas[i], s))
            if (senhas_iguais(senha, s))
                return true;
    }

    return false;
}

// Apaga uma senha marcando endereço na matriz como zero
// (dessa forma evita escrever constantemente na EEPROM).
bool apagar_senha(char senha[])
{
    char s[LARGURA_SENHA];
    for (int i = 0; i < QUANTIDADE_SENHAS; i++)
    {
        if (resgatar_senha(enderecos_senhas[i], s))
        {
            if (senhas_iguais(senha, s))
            {
                // Primeiro local é zerado para representar ausência de senha.
                EEPROM.update(enderecos_senhas[i], 0);
                return true; // sucesso na operação.
            }
        }
    }

    return false; // não existe essa senha.
}

// Modifica a senha de administrador
void modificar_adm_senha(char nova_senha[])
{
    lcd_print("Nova senha adm..", 0);
    delay(700);
    ler_senha();

    for (int i = 0; i < LARGURA_SENHA; i++)
    {
        EEPROM.update(i, nova_senha[i]);
    }
}

// Apaga todas as senhas exceto a senha de
// administrador
void apagar_todas_senhas()
{
    for (int i = 1; i < QUANTIDADE_SENHAS; i++)
    {
        // Todos os primeiros locais zerados para representar ausência de senha.
        EEPROM.update(enderecos_senhas[i], 0);
    }
}

// Verifica se a senha provida é a de administrador.
bool verificar_adm(char senha[])
{
    char adm[LARGURA_SENHA];

    // Pegando a senha de adm
    resgatar_senha(ADM_ENDERECO, adm);

    return senhas_iguais(senha, adm);
}

// Simulando a porta ser fechada.
void fechar_porta(int tempo)
{
    if (porta_aberta)
    {
        delay(tempo);
        porta_aberta = false;
        adm_logou = false;
        lcd_print("Fechando...", 0);
        delay(2000);
    }
}

void lcd_print(String texto, int pos)
{
    lcd.clear();
    lcd.setCursor(0, pos);
    lcd.print(texto);
}

void blink_color(int red_value, int green_value, int blue_value)
{
    analogWrite(red_pin, red_value);
    analogWrite(green_pin, green_value);
    analogWrite(blue_pin, blue_value);
    delay(3000);
    analogWrite(red_pin, 0);
    analogWrite(green_pin, 0);
    analogWrite(blue_pin, 0);
}

void senha_incorreta_behavior()
{
    lcd_print("Senha incorreta", 0);
    tone(buzzer, 90, 700);
    blink_color(255, 0, 0);
}

void senha_correta_behavior()
{
    lcd_print("Porta aberta", 0);
    tone(buzzer, 390, 300);
    delay(350);
    tone(buzzer, 400, 500);
    blink_color(0, 255, 0);
}

void senha_adm_behavior()
{
    lcd_print("ADM entrou", 0);
    tone(buzzer, 262, 300);
    delay(350);
    tone(buzzer, 400, 500);
    blink_color(0, 0, 255);
}

void ler_senha()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insira Senha: ");
    while (true)
    {
        // Posicionando o cursor antes de coletar o valor pressionado
        lcd.setCursor(lcd_coluna, 1);
        // Pegando o valor pressionado
        char pressionado = teclado.waitForKey();
        // Salvando o valor pressionado em uma lista de char e
        // colocando um * na tela
        if (validador_botao(pressionado))
        {
            lcd.print('*');
            input_senha[lcd_coluna] = pressionado;
            lcd_coluna += 1;
        }
        // Se a tecla A for apagada, volta o cursor em uma posição
        // e printa um espaço para poder apagar o valor ali.
        else if (pressionado == 'A')
        {
            lcd_coluna -= 1;
            lcd.setCursor(lcd_coluna, 1);
            lcd.print(' ');
        }
        // Se a tecla D for pressionada, apaga tudo que está na tela e
        // reseta a posição do cursor.
        else if (pressionado == 'D')
        {
            lcd.clear();
            lcd.print("Insira Senha: ");
            lcd_coluna = 0;
        }
        // Se a senha for completamente preenchida,
        // retorna para fora da função
        if (lcd_coluna == LARGURA_SENHA)
        {
            lcd_coluna = 0;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Confirma a");
            lcd.setCursor(0, 1);
            lcd.print("senha?");
            pressionado = teclado.waitForKey();
            if (pressionado == 'C')
            {
                return;
            }
            else
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Insira Senha: ");
            }
        }
    }
}

// Confirmar a mudança que o admnistrador pretende fazer
bool tem_certeza()
{
    bool respondido = false;
    lcd_print("Certeza?", 0);
    do
    {
        char resposta = teclado.waitForKey();
        switch (resposta)
        {
        case 'C':
            return true;
        case '*':
            return false;
        default:
            continue;
        }
    } while (respondido == false);
}

bool validador_botao(char t)
{
    // Checando se a tecla pressionado é válida ou não
    if (t == 'A' or t == 'B' or t == 'C' or t == 'D' or t == '*' or t == '#')
    {
        return false;
    }
    else
    {
        return true;
    }
}