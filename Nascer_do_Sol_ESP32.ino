/******************************************************************************
           Descubra o horário de nascimento e de poente do Sol com ESP32
                                 Sketch Principal

                        Criado em 05 de Janeiro de 2022
                                por Michel Galvão

  Eletrogate - Loja de Arduino \\ Robótica \\ Automação \\ Apostilas \\ Kits
                            https://www.eletrogate.com/
******************************************************************************/

// Inclusão da(s) biblioteca(s)
#include <TimeLib.h> // Armazenamento do horário atual
#include <TinyGPS++.h> // GPS
#include <Wire.h> // Comunicação I2C
#include <Adafruit_GFX.h> // Display OLED
#include <Adafruit_SSD1306.h> // Display OLED
#include <RTClib.h> // Manipulação de Horário
#include "logoEletrogate.h" // Logo do Display

// Configurações do display
#define SCREEN_WIDTH    128 // Largura da tela OLED, em pixels
#define SCREEN_HEIGHT   64 // Altura da tela OLED, em pixels
#define OLED_RESET      -1 // Pino de Reset do display (-1, pois o mesmo é compartilhado do Arduino)
#define SCREEN_ADDRESS  0x3C // endereço I2C

//Configurações GPS
#define RX_U1UXD        16
#define TX_U1UXD        17

// Instanciação dos objetos das classes
TinyGPSPlus gps;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Protótipo das Funções
double converteGrausParaRadianos (double emGraus);
double converteRadianosParaGraus (double emRadianos);
void converteDecimaisParaGraus(double decimal, int *_hora, int *_minuto, int *_segundo);
void horariosSol(TinyGPSPlus _gps, DateTime *_horarioNascer, DateTime *_horarioPoente);
bool validacaoGPS();
int diaDoAno(int dia, int mes, unsigned long ano);
void exibicaoCarregamento();
bool anoEBissexto(unsigned long ano);
int qtdDiasDoMes(int mes, bool eBissexto);

#define time_offset -10800  // FUSO HORÁRIO: é definido um deslocamento de relógio de 10800 segundos (3 horas) ==> UTC -3 

// Variáveis Globais
unsigned long tempoMinimoEntreRequisicoes = 3000; // tempo mínimo para não detectar erros na leitura dos dados do GPS
unsigned long tempo; // variável de controle do tempo
bool controleTela = false; // controla a exibição da tela de informações ou de alerta de erro

// Configurações Iniciais
void setup() {
  Serial.begin(115200);
  delay(1010);
  Serial.println("Serial iniciada");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha na alocação de SSD1306"));
    while (true); // Não prossiga, faça um loop para sempre
  }
  display.clearDisplay(); // limpa display

  display.drawBitmap(0, 0,  logo, 128, 64, WHITE); // exibe logo da Eletrogate no display OLED
  display.display(); // exibe as informações passadas para o display até agora
  delay(3000);

  display.setTextSize(1); // define fonte de texto para tamanho 1
  display.setTextColor(SSD1306_WHITE); // define cor de texto para claro
  display.clearDisplay();
  display.setCursor(0, 0); // coloca cursor na posição x:0 e y:0
  display.println(F("Display Iniciado"));
  display.setCursor(0, 16); // coloca cursor na posição x:0 e y:16
  display.println(F("Horarios"));
  display.println(F("do"));
  display.println(F("Sol"));
  display.println(F("Nascente / Poente"));
  display.display(); // exibe as informações passadas para o display até agora
  delay(1010);


  //Serial2.begin (taxa de transmissão, protocolo, pino RX, pino TX);
  Serial2.begin(9600, SERIAL_8N1, RX_U1UXD, TX_U1UXD);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Serial2 iniciada"));
  display.setCursor(0, 16);
  display.println(F("Horarios"));
  display.println(F("do"));
  display.println(F("Sol"));
  display.println(F("Nascente / Poente"));
  display.display();
  Serial.println("Serial2 iniciada");
  delay(1000);
  display.clearDisplay();


  display.setCursor(0, 0);
  display.println(F("Esperando dados GPS"));
  display.display();
  exibicaoCarregamento();
  display.clearDisplay();
  display.display();

} // fim do setup

// Loop Infinito
void loop() {
  delay(1); // atraso para não travar core
  while (Serial2.available()) { // enquanto houver bytes para leitura na Serial2, ...
    char recebido = Serial2.read(); // lê os dados recebidos na porta Serial2 e armazena na variável recebido
    gps.encode(recebido); // alimenta repetidamente com caracteres vindos do dispositivo GPS
  } // fim while(Serial2.available())

  if (validacaoGPS()) { // se os dados GPS foram atualizados sem erro, ...
    if (controleTela == false) { // se a varável controleTela ter valor false, ...
      display.clearDisplay(); // limpa display
    } // fim if (controleTela == false)
    controleTela = true; // atribu valor true à variável controleTela
    tempo = millis(); // atribu o valor atual de mills à variável tempo

    //LAT, LONG :
    Serial.print("Localizacao: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.print(gps.location.lng(), 6);
    Serial.println();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Dados Geograficos");
    display.setCursor(0, 16);
    char buff[10]; //
    String lat = "";
    String lng = "";

    dtostrf(gps.location.lat(), 4, 6, buff); // converte a posição de latitude no tipo double para o tipo String
    lat = buff;
    display.print("lat: ");
    display.println(lat);
    //display.println(" --.------");

    dtostrf(gps.location.lng(), 4, 6, buff); // converte a posição de longitude no tipo double para o tipo String
    lng = buff;
    display.print("lng: ");
    display.println(lng);
    //display.println(" --.------");


    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year()); // Configura a hora do sistema para o tempo dado:
    // na ordem hora, minuto, segundo, dia, mês e ano
    adjustTime(time_offset); // Ajuste a hora do sistema adicionando o valor de ajuste (fuso)

    String horario;
    horario += day();
    horario += "/";
    horario += month();
    horario += "/";
    horario += year();
    horario += " ";
    horario += hour();
    horario += "h";
    horario += minute();
    horario += "m";
    horario += second();
    horario += "s";
    Serial.println("horario: " + String(horario));

    Serial.print(F("Qtd de dias no Ano passados: "));
    int dias = diaDoAno(day(), month(), year()); // obtém a quantidade de dias do ano passados até agora (1 a 365[ou 366])
    Serial.print(dias);
    Serial.println();

    display.print("data: ");
    String data;
    data += day();
    data += "/";
    data += month();
    data += "/";
    data += year();
    display.println(data); // mostra a data atual no display OLED

    display.print("tempo: ");
    String tempo;
    tempo += hour();
    tempo += "h";
    tempo += minute();
    tempo += "m";
    tempo += second();
    tempo += "s";
    display.println(tempo); // mostra o horáro atual no display OLED

    DateTime horarioNascerSol; // variável para armazenar o horário de nascimento do Sol
    DateTime horarioPoenteSol; // variável para armazenar o horário de poente do Sol
    horariosSol(gps, &horarioNascerSol, &horarioPoenteSol); // obtém o o horário de nascimento e de poente do Sol

    //Exibe na Serial e no display o horário de nascimento do Sol
    Serial.println("horarioNascerSol: " + String(horarioNascerSol.hour()) + String("h") + String(horarioNascerSol.minute()) + String("m") + String(horarioNascerSol.second()) + String("s"));
    display.println("nascer: " + String(horarioNascerSol.hour()) + String("h") + String(horarioNascerSol.minute()) + String("m") + String(horarioNascerSol.second()) + String("s"));

    //Exibe na Serial e no display o horário de poente do Sol
    Serial.println("horarioPoenteSol: " + String(horarioPoenteSol.hour()) + String("h") + String(horarioPoenteSol.minute()) + String("m") + String(horarioPoenteSol.second()) + String("s"));
    display.println("poente: " + String(horarioPoenteSol.hour()) + String("h") + String(horarioPoenteSol.minute()) + String("m") + String(horarioPoenteSol.second()) + String("s"));

    Serial.println();
    display.display();

  } else if (millis() - tempo > tempoMinimoEntreRequisicoes) { // Se não, se o tempo  atual subtraido da variável tempo for menor que o tempo mínimo, ...
    if (controleTela == true) { // Se controleTela for true, ...
      display.clearDisplay(); // limpa o display
    }
    controleTela = false; // atribui o valor false para controleTela

    // Exibe no display o alerta de que se está esperando receber dados válidos
    display.setCursor(0, 0);
    display.println(F("Esperando dados GPS"));
    display.display();
    exibicaoCarregamento(); // exibe animação de espera

    Serial.print(".");
    delay(100);
  } // fim de else if (millis() - tempo > tempoMinimoEntreRequisicoes)

  delay(200);

}// fim do loop



// Desenvolvimento das Funções


double converteGrausParaRadianos (double emGraus) {
  /**
      Converte o valor informado em Graus para Radianos.

      @param emGraus O valor em Graus para conversão
      @return O valor em double convertido.
  */
  double emRadianos = emGraus * PI / 180; // efetua a fórmula de conversão: https://www.google.com/search?q=graus+para+radianos
  return emRadianos;
}

double converteRadianosParaGraus (double emRadianos) {
  /**
    Converte o valor informado em Radianos para Graus.

    @param emRadianos O valor em Radianos para conversão
    @return O valor em double convertido.
  */
  double emGraus = emRadianos * 180 / PI; // efetua a fórmula de conversão: https://www.google.com/search?q=radianos+para+graus
  return emGraus;
}

void converteDecimaisParaGraus(double decimal, int *_hora, int *_minuto, int *_segundo) {
  /**
    Converte o valor informado em Graus-Decimais para Graus-Minutos-Segundos.

    @param decimal O valor em Graus-Decimais para conversão
    @param *_hora O ponteiro para a hora
    @param *_minuto O ponteiro para o minuto
    @param *_segundo O ponteiro para o segundo
  */
  *_hora = (int)decimal; // passa para o ponteiro de hora a parte inteira de @param decimal
  *_minuto = (decimal - *_hora) * 60; // passa para o ponteiro de minuto a parte decimal de @param decimal multiplicado por 60
  *_segundo = (((decimal - *_hora) * 60) - *_minuto) * 60;  // passa para o ponteiro de segundo a parte decimal de
  //                                                        //@param decimal multiplicado por 60 subtraido de @param *_minuto multiplicado por 60
}

void horariosSol(TinyGPSPlus _gps, DateTime *_horarioNascer, DateTime *_horarioPoente) {
  /**
    Obtém o horário de nascer e de poente do Sol a partir do objeto gps

    @param _gps O objeto gps informado
    @param *_horarioNascer O ponteiro para o horário de nascimento do Sol
    @param *_horarioPoente  ponteiro para o horário de poente do Sol
  */
  double latitude = _gps.location.lat(); // variável para armazenar a latitude atual
  double longitude = _gps.location.lng(); // variável para armazenar a longitude atual

  // NOTA: Todas funções de trigonometrias do Framework Arduino estão em radianos.


  double declinacaoSolar = 23.45 * sin(
                             converteGrausParaRadianos(
                               (360.0 / 365.0) * (284 + diaDoAno(day(), month(), year()))
                             )
                           ); // obtém a declinação Solar

  Serial.println("declinacaoSolar: " + String(declinacaoSolar) + String("°"));

  int hora = 0, minuto = 0, segundo = 0; // variáveis locais de hora, minuto e segundo
  double tempoDeDuracaoDoDia = 2.0 / 15.0 *
                               converteRadianosParaGraus(acos(
                                     -tan(converteGrausParaRadianos(latitude))
                                     * tan(converteGrausParaRadianos(declinacaoSolar))
                                   )); // obtém o tempo de duração total do dia

  converteDecimaisParaGraus(tempoDeDuracaoDoDia, &hora, &minuto, &segundo); // converte o tempo de duração do dia do formato Graus-Decimais para o formato Graus-Minutos-Segundos
  Serial.print("Qtd de horas de Sol: " + String(tempoDeDuracaoDoDia, 7)); // Exibe na Serial a quantidade de horas de Sol, em Graus-Decimais
  Serial.println(String(" (") + hora + String("h") + minuto + String("m") + segundo + String("s)")); // Exibe na Serial a quantidade de horas de Sol, em Graus-Minutos-Segundos


  Serial.println("Antes da conversão: ");
  double tempoMeioDia = tempoDeDuracaoDoDia / 2; // Tempo da metade do dia
  double nascer = 12 - tempoMeioDia; // horário de nascimento do Sol
  double poente = 12 + tempoMeioDia; // horário de poente do Sol

  converteDecimaisParaGraus(nascer, &hora, &minuto, &segundo); // converte o horário de nascimento do Sol do formato Graus-Decimais para o formato Graus-Minutos-Segundos
  Serial.print("    Horario do Nascer: " + String(nascer, 7)); // Exibe na Serial o horário do Nascer em Graus-Decimais
  Serial.println(String(" (") + hora + String("h") + minuto + String("m") + segundo + String("s)")); // exibe o horário na Serial em Graus-Minutos-Segundos

  converteDecimaisParaGraus(poente, &hora, &minuto, &segundo);  // converte o horário de poente do Sol do formato Graus-Decimais para o formato Graus-Minutos-Segundos
  Serial.print("    Horario do Poente: " + String(poente, 7)); // Exibe na Serial o horário do Poente em Graus-Decimais
  Serial.println(String(" (") + hora + String("h") + minuto + String("m") + segundo + String("s)")); // exibe o horário na Serial em Graus-Minutos-Segundos

  double fusoHorario = (time_offset / 3600); // obtém o fuso horário, em horas

  double meridianoDoFuso = fusoHorario * 15; // obtém o fuso do meridiano, em graus graus-decimais

  double diferencaDeFusos = (longitude) - meridianoDoFuso; // obtém a diferença entre a longitude atual e o fuso do meridiano, em graus-decimais

  if (diferencaDeFusos != 0) { // se a diferencaDeFusos for diferente de 0, ...
    double correcaoLongitude; // cria variável para armazenar a correção de longitude
    correcaoLongitude = ((diferencaDeFusos * 60) / 15) / 60; // aplica a fórmula de Tempo de correção
    Serial.println("correcaoLongitude: " + String(correcaoLongitude, 7)); // Exibe a correção de longitude na Serial
    if (diferencaDeFusos < 0) { // se a diferencaDeFusos for menor que 0, ...
      nascer = nascer - correcaoLongitude; // horário de nascer é igual ao horário de nascer subtráido de correcaoLongitude
      poente = poente - correcaoLongitude; // horário de poente é igual ao horário de poente subtráido de correcaoLongitude
    } else if (diferencaDeFusos > 0) { // se não, se a diferencaDeFusos for maior que 0, ...
      nascer = nascer + correcaoLongitude; // horário de nascer é igual ao horário de nascer somado de correcaoLongitude
      poente = poente + correcaoLongitude; // horário de poente é igual ao horário de poente somado de correcaoLongitude
    }
    Serial.println("Depois da conversão: ");
    converteDecimaisParaGraus(nascer, &hora, &minuto, &segundo); // converte o horário de nascer do Sol do formato Graus-Decimais para o formato Graus-Minutos-Segundos
    *_horarioNascer = DateTime(0, 0, 0, hora, minuto, segundo); // atribui ao ponteiro do horário de nascer a hora, o minuto e o segundo
    Serial.print("    Horario do Nascer: " + String(nascer, 7)); // Exibe na Serial o horário do Nascer em Graus-Decimais, após a correção
    Serial.println(String(" (") + hora + String("h") + minuto + String("m") + segundo + String("s)")); // exibe o horário na Serial em Graus-Minutos-Segundos, após a correção

    converteDecimaisParaGraus(poente, &hora, &minuto, &segundo); // converte o horário de poente do Sol do formato Graus-Decimais para o formato Graus-Minutos-Segundos
    *_horarioPoente = DateTime(0, 0, 0, hora, minuto, segundo); // atribui ao ponteiro do horário de poente a hora, o minuto e o segundo
    Serial.print("    Horario do Poente: " + String(poente, 7)); // Exibe na Serial o horário do Poente em Graus-Decimais, após a correção
    Serial.println(String(" (") + hora + String("h") + minuto + String("m") + segundo + String("s)")); // exibe o horário na Serial em Graus-Minutos-Segundos, após a correção
  } else {
    Serial.print("Depois da conversão: Não necessária conversão!"); // não foi necessário realizar nenhuma correção de longitude
  }

} // fim de void horariosSol(TinyGPSPlus _gps, DateTime *_horarioNascer, DateTime *_horarioPoente)

bool validacaoGPS() {
  /**
    Verifica de determinados dados obtidos do módulo GPS estão válidos

    @return true Se os dados estão válidos
  */
  bool retorno = false;
  if (gps.date.isUpdated() &&
      gps.satellites.isUpdated() &&
      gps.location.isUpdated() &&
      gps.location.isValid())
  { // Se os dados de data, de satélites e de localização estão válidos, ...
    retorno = true; // atribui true para retorno
  }
  return retorno; // retorno
} // fim de validacaoGPS()

int diaDoAno(int dia, int mes, int ano) {
  /**
    Informa o dia atual do ano (1 a 365 [ou 366])

    @param dia O dia atual (1 a 31)
    @param mes O mês atual (1 a 12)
    @param ano O ano atual (0 a ?)
    @return O dia do Ano
  */
  int qtd = 0; // varável para armazenar a quantidade de dias do ano passados

  for (int i = 1; i <= mes - 1; i++) { // enquanto i for menor ou igual que o número do mês atual, ...
    qtd = qtd + qtdDiasDoMes(i, anoEBissexto(ano)); // soma a quantidade de dias do mês com a varável qtd
  }
  qtd = qtd + dia; // após, soma o valor de dias atuais com qtd
  return qtd; // retorna qtd
} // fim de diaDoAno(int dia, int mes, int ano)

void exibicaoCarregamento() {
  /**
      Exibe no display OLED a animação de carregamento
  */
  for (int i = 17; i <= 20; i++) { // enquanto i for menor ou igual que 20, ...
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, 20, SSD1306_WHITE); // desenha círculo claro maior no meio do display
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, 17, SSD1306_BLACK); // desenha círculo escuro menor no meio do display
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, i, SSD1306_BLACK); // desenha cículo intermediário escuro no meio do diplay, com tamanho variável
    display.display(); // mostra no display os círculos desenhados
    delay(100);
  }
  for (int i = 20; i >= 17; i--) { // enquanto i for maor ou igual que 17, ...
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, 20, SSD1306_WHITE); // desenha círculo claro maior no meio do display
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, 17, SSD1306_BLACK); // desenha círculo escuro menor no meio do display
    display.fillCircle(display.width() / 2, (display.height() / 2) - 16 + 20, i, SSD1306_BLACK); // desenha cículo intermediário escuro no meio do diplay, com tamanho variável
    display.display(); // mostra no display os círculos desenhados
    delay(100);
  }
} // fim de exibicaoCarregamento()


bool anoEBissexto(unsigned long ano) {
  /**
    Verifica se o ano atual é bissexto

    @param ano O ano atual (0 a ?)
    @return true Se o ano atual for bissexto
  */
  bool retorno; // cria variável para retorno
  if (ano % 400 == 0) { // Se o ano atual for múltiplo de 400, ...
    // é bissexto
    retorno = true; // retorna true
  } else { // se não, ...
    if (ano % 4 == 0 && ano % 100 != 0) { // se o ano atual for múltplo de 4 E não for múltplo de 100, ...
      // é bissexto
      retorno = true; // retorna true
    } else { // se não, ...
      // não é bissexto
      retorno = false; // retorna false
    }
  }
  return retorno; // retorna o retorno
} // fim de anoEBissexto(unsigned long ano)

int qtdDiasDoMes(int mes, bool eBissexto) {
  /**
    Retorna a quantidade de dias do mês atual

    @param mes O mês atual atual (1 a 12)
    @param eBissexto Se o ano atual é bissexto ou não
    @return O número de dias do mês atual
  */
  int retorno; // variável para armazenar o número de dias do mês
  if (mes == 1 || mes == 3 || mes == 5 || mes == 7 || mes == 8 || mes == 10 || mes == 12)
  { // Se o número do mês for 1, 3, 5, 7, 8, 10 ou 12, ...
    retorno = 31; // número de dias é 31
  }
  else if (mes == 2)
  { // Se não, se o número do mês for 2, ...
    if (eBissexto)
    {// Se o ano for bissexto, ...
      retorno = 29; // número de dias é 29
    }
    else { // Se não, ...
      retorno = 28;  // número de dias é 28
    }
  }
  else if (mes == 4 || mes == 6 || mes == 9 || mes == 11)
  { // Se não, se o número do mês for 4, 6, 9 ou 11, ...
    retorno = 30; //   // número de dias é 30
  }

  return retorno; // retorna o retorno
}
