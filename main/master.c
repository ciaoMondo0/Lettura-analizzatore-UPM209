#include <string.h>
#include "esp_log.h"
#include "mbcontroller.h"   // Libreria ESP-Modbus
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_spiffs.h"     // Libreria per SPIFFS
#include "cJSON.h"          // Libreria per JSON (ESP-IDF built-in)

#define MB_PORT_NUM         (1)           // UART1 per RS485
#define MB_DEV_SPEED        (19200)       // Baudrate
#define MB_SLAVE_ADDR       (1)           // Indirizzo slave Modbus
#define MB_PARITY_MODE      (UART_PARITY_DISABLE) // Parità NONE
#define MB_UART_TXD         (7)           // Pin TX RS485
#define MB_UART_RXD         (8)           // Pin RX RS485
#define MB_UART_RTS         (4)           // Pin RTS per abilitazione trasmissione

static const char *TAG = "UPM209"; // Tag per log Modbus

// Struttura registri Modbus
typedef struct {
    const char *nome;    // Nome del parametro
    const char *unita;    // Unità di misura
    uint16_t reg_start;  // Indirizzo registro
    uint8_t reg_size;    // Dimensione (n° parole Modbus)
    uint8_t command;     // Comando Modbus: 0x03 = Holding, 0x04 = Input
    float scala;         // Scala per la conversione
} modbus_register;

// Solo registri monofase
modbus_register registers[] = {
    // Valori istantanei
    { "Tensione L-N Fase 1 (V1)", "mV", 0x0000, 2, 0x04, 0.001 },
    { "Corrente Fase 1 (A1)", "mA", 0x000E, 2, 0x04, 0.001 },
    { "Potenza Attiva Fase 1 (P1)", "mW", 0x0018, 4, 0x04, 0.001 },
    { "Potenza Apparente Fase 1 (S1)", "mVA", 0x0028, 4, 0x04, 0.001 },
    { "Potenza Reattiva Fase 1 (Q1)", "mVAR", 0x0038, 4, 0x04, 0.001 },
    { "Fattore di Potenza Fase 1 (PF1)", "0.001", 0x0048, 2, 0x04, 0.001 },
    { "Tangente Ø Fase 1 (TAN01)", "0.001", 0x0056, 2, 0x04, 0.001 },

    // Valori massimi
    { "Tensione Massima L-N Fase 1 (V1 MAX)", "mV", 0x0200, 2, 0x04, 0.001 },
    { "Corrente Massima Fase 1 (A1 MAX)", "mA", 0x020E, 2, 0x04, 0.001 },
    { "Potenza Attiva Massima Fase 1 (+P1 MAX)", "mW", 0x0218, 4, 0x04, 0.001 },
    { "Potenza Apparente Massima Fase 1 (+S1 MAX)", "mVA", 0x0238, 4, 0x04, 0.001 },
    { "Potenza Reattiva Massima Fase 1 (+Q1 MAX)", "mVAR", 0x0258, 4, 0x04, 0.001 },
    { "Fattore di Potenza Massimo Fase 1 (+PF1 MAX)", "0.001", 0x0278, 2, 0x04, 0.001 },
    { "Tangente Ø Massima Fase 1 (TAN01 MAX)", "0.001", 0x028A, 2, 0x04, 0.001 },

    // Altri parametri
    { "Frequenza (F)", "mHz", 0x0072, 2, 0x04, 0.001 },
    { "Energia Attiva Importata (kWh)", "Wh", 0x0400, 4, 0x04, 0.0001 }
};


// Funzione per convertire quattro registri Modbus (2 word) in IEEE 754
float modbus_16bit_register_to_float(uint16_t a, uint16_t b, float scala) {
    // Combina i registri in un uint32_t 
    uint32_t combined = ((uint32_t)a << 16) | b;

    // Applica la scala
    return combined * scala;
}

// Funzione per convertire quattro registri Modbus (4 word) in double
double modbus_16bit_register_to_double(uint16_t a, uint16_t b, uint16_t c, uint16_t d, float scala) {
    // Combina i registri in un uint64_t 
    uint64_t combined = ((uint64_t)a << 48) | 
                        ((uint64_t)b << 32) | 
                        ((uint64_t)c << 16) | 
                        (uint64_t)d;

    // Applica la scala
    return combined * scala;
}


//Calcola numero dei registri
#define NUM_REGISTERS (sizeof(registers) / sizeof(registers[0]))

// Inizializza SPIFFS
static void init_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Errore nel montaggio SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Dimensione partizione: totale: %d, usato: %d", total, used);
    } else {
        ESP_LOGE(TAG, "Errore nel ricevere informazioni SPIFFS");
    }
}

// Inizializzazione Modbus
static esp_err_t master_init(void) {
     mb_communication_info_t comm = {
        .port = MB_PORT_NUM,
        .mode = MB_MODE_RTU,
        .baudrate = MB_DEV_SPEED,
        .parity = MB_PARITY_MODE
    };

    void *master_handler = NULL;
    esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
    MB_RETURN_ON_FALSE((master_handler != NULL), ESP_ERR_INVALID_STATE, TAG,
                       "mb controller initialization fail.");
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                       "mb controller initialization fail, returns(0x%x).", (int)err);

    err = mbc_master_setup((void *)&comm);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                       "mb controller setup fail, returns(0x%x).", (int)err);

    err = uart_set_pin(MB_PORT_NUM, MB_UART_TXD, MB_UART_RXD, MB_UART_RTS, UART_PIN_NO_CHANGE);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                       "mb serial set pin failure, uart_set_pin() returned (0x%x).", (int)err);

    err = mbc_master_start();
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                       "mb controller start fail, returned (0x%x).", (int)err);

    ESP_LOGI(TAG, "Modbus master inizializzato correttamente.");
    return ESP_OK;
}


// Legge registri e salva su JSON
static void registers_to_json(void) {
   
    esp_err_t err;

    cJSON *json_root = cJSON_CreateObject();

    for (size_t i = 0; i < NUM_REGISTERS; i++) {
        modbus_register reg = registers[i];
        uint16_t register_data[4] = {0}; // Buffer per i dati Modbus 

        mb_param_request_t request = {
            .slave_addr = MB_SLAVE_ADDR,
            .command = reg.command,
            .reg_start = reg.reg_start,
            .reg_size = reg.reg_size
        };

        // Lettura Modbus
        err = mbc_master_send_request(&request, (void *)&register_data);
        if (err == ESP_OK) {
            if (reg.reg_size == 4) {
                // Unione di 4 word in un uint64_t e conversione a double
                double value_double = modbus_16bit_register_to_double(register_data[0], register_data[1], register_data[2], register_data[3], reg.scala);

                // Aggiunta dei valori al JSON
                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "value", value_double);
                cJSON_AddStringToObject(item, "unit", reg.unita); 

                cJSON_AddItemToObject(json_root, reg.nome, item); 
                ESP_LOGI(TAG, "%s: %.6f %s", reg.nome, value_double, reg.unita);
            } else if (reg.reg_size == 2) {
                // Unione di 2 word in un uint32_t e conversione a float
                float value_float = modbus_16bit_register_to_float(register_data[0], register_data[1], reg.scala);

                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "value", value_float);
                cJSON_AddStringToObject(item, "unit", reg.unita); 

                cJSON_AddItemToObject(json_root, reg.nome, item); 
                ESP_LOGI(TAG, "%s: %.3f %s", reg.nome, value_float, reg.unita);
            } else {
                // Registro a 1 word, conversione diretta a float
                float value_float = register_data[0] * reg.scala;

                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "value", value_float);
                cJSON_AddStringToObject(item, "unit", reg.unita); 

                cJSON_AddItemToObject(json_root, reg.nome, item); 
                ESP_LOGI(TAG, "%s: %.3f %s", reg.nome, value_float, reg.unita);
            }
        } else {
            ESP_LOGE(TAG, "Errore lettura registro %s. Codice errore: %s", reg.nome, esp_err_to_name(err)); 
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Ritardo tra letture
    }

    // Scrittura JSON su file
    char *json_string = cJSON_PrintUnformatted(json_root);
    if (json_string != NULL) {
        FILE *file = fopen("/spiffs/data_monofase.json", "w");
        if (file) {
            fwrite(json_string, sizeof(char), strlen(json_string), file);
            fclose(file);
            ESP_LOGI(TAG, "File JSON salvato correttamente.");
        } else {
            ESP_LOGE(TAG, "Errore nella scrittura del file JSON.");
        }
        free(json_string);
    }

    cJSON_Delete(json_root); 
}

static void read_json_file(void) {
    FILE *file = fopen("/spiffs/data_monofase.json", "r");
   
        char buffer[2048];
        size_t bytes = fread(buffer, 1, 2047, file);

            buffer[bytes] = '\0'; 
            printf("Contenuto del file JSON:\n%s\n", buffer);

             fclose(file);

}



void app_main(void) {
    init_spiffs();       // Inizializza SPIFFS
    master_init();       // Inizializza Modbus Master

  read_json_file();


    while (1) {
        registers_to_json();
        vTaskDelay(pdMS_TO_TICKS(5000)); // Lettura ogni 5 secondi
    }
}