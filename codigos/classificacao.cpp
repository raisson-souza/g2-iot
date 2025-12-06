/*
 * Integração ESP32 + Edge Impulse (Classificação de Som/Amplitude)
 */

// ---------------------------------------------------------
// 1. INCLUA A BIBLIOTECA DO EDGE IMPULSE AQUI
// Substitua pelo nome correto da sua lib (veja nos exemplos da IDE)
#include <ei-raisson-souza-project-1-arduino-1.0.3.h> 
// ---------------------------------------------------------

const int pinoSensor = 36;
const int janelaAmostragem = 50; 

// Buffer para armazenar os dados brutos antes de classificar
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

void setup() {
    Serial.begin(115200);
    pinMode(pinoSensor, INPUT);
    analogReadResolution(12);

    Serial.println("Iniciando inferencia...");
    
    // Verifica se o tamanho do buffer faz sentido
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 1) {
        Serial.println("ERRO: O modelo espera mais de 1 eixo de dados (ex: acelerometro x,y,z).");
        return;
    }
}

void loop() {
    ei_printf("Amostrando dados...\n");

    // ---------------------------------------------------------
    // 2. PREENCHIMENTO DO BUFFER (COLETA DE DADOS)
    // O modelo precisa de 'X' amostras para poder classificar.
    // Vamos rodar o seu código várias vezes até encher o vetor 'features'.
    // ---------------------------------------------------------
    
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
        
        // --- SEU CÓDIGO ORIGINAL DE LEITURA (INICIO) ---
        unsigned long inicioMillis = millis(); 
        unsigned int sinalMax = 0;             
        unsigned int sinalMin = 4095;          

        while (millis() - inicioMillis < janelaAmostragem) {
            int leitura = analogRead(pinoSensor);
            if (leitura < 4095) {  
                if (leitura > sinalMax) sinalMax = leitura;
                if (leitura < sinalMin) sinalMin = leitura;
            }
        }
        int amplitudePicoAPico = sinalMax - sinalMin;
        // --- SEU CÓDIGO ORIGINAL DE LEITURA (FIM) ---

        // Armazena a leitura no buffer do Edge Impulse
        features[i] = (float)amplitudePicoAPico; 
        
        // Opcional: Imprimir progresso de coleta (pode deixar lento)
        // Serial.print("."); 
    }
    
    // ---------------------------------------------------------
    // 3. CLASSIFICAÇÃO (RODAR O MODELO)
    // Agora que o vetor 'features' está cheio, mandamos para a IA.
    // ---------------------------------------------------------

    // Cria a estrutura que aponta para os dados brutos
    signal_t signal;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    
    if (err != 0) {
        ei_printf("Erro ao criar sinal: %d\n", err);
        return;
    }

    // Roda o classificador
    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, false /* debug = false */);

    if (err != EI_IMPULSE_OK) {
        ei_printf("ERRO na execucao do classificador (%d)\n", err);
        return;
    }

    // ---------------------------------------------------------
    // 4. MOSTRAR RESULTADOS
    // ---------------------------------------------------------
    
    ei_printf("\nResultados da Classificacao:\n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        // Imprime: Nome da Classe: Probabilidade
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        
        // Exemplo de lógica para acionar algo se a certeza for > 80%
        if (result.classification[ix].value > 0.8) {
            // Ação específica (ex: ligar LED)
            // if (String(result.classification[ix].label) == "bater_palma") { ... }
        }
    }
    ei_printf("\n");
    
    // Pequena pausa antes da próxima coleta completa
    delay(100); 
}