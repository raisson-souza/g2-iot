/*
 * Leitura de Volume (Amplitude) com Sensor de Som no ESP32
 * Método: Amostragem Pico-a-Pico (Peak-to-Peak)
 */

const int pinoSensor = 36;  // Pino VP (GPIO 36)
const int janelaAmostragem = 50; // Duração da amostra em ms (50ms = 20Hz)

void setup() {
  Serial.begin(115200);
  pinMode(pinoSensor, INPUT);
  
  // O ESP32 tem resolução de 12 bits (valores de 0 a 4095)
  // Diferente do Arduino Uno que é 10 bits (0 a 1023)
  analogReadResolution(12);
}

void loop() {
  unsigned long inicioMillis = millis(); // Marca o tempo inicial
  unsigned int sinalMax = 0;             // Inicializa com o valor mínimo possível
  unsigned int sinalMin = 4095;          // Inicializa com o valor máximo possível

  // Coleta dados durante 50ms (janela de amostragem)
  while (millis() - inicioMillis < janelaAmostragem) {
    int leitura = analogRead(pinoSensor);

    if (leitura < 4095) {  // Filtro básico para leituras espúrias
      if (leitura > sinalMax) {
        sinalMax = leitura;  // Salva o pico mais alto
      }
      if (leitura < sinalMin) {
        sinalMin = leitura;  // Salva o pico mais baixo
      }
    }
  }

  // A amplitude é a diferença entre o teto e o chão da onda
  int amplitudePicoAPico = sinalMax - sinalMin;
  
  // Converte para Volts (Opcional, apenas para visualização)
  // 3.3V é a referência do ESP32, 4095 é a resolução máxima
  float volts = (amplitudePicoAPico * 3.3) / 4095;

  // Exibe os resultados no Serial Monitor
  // Use a ferramenta "Serial Plotter" (Ctrl+Shift+L) para ver o gráfico!

  Serial.println(amplitudePicoAPico); 

}