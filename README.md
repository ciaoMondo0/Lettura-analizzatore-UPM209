# Monitoraggio Energetico con ESP32 e UPM209

## Descrizione

Questo progetto permette di monitorare in tempo reale i dati energetici del contatore UPM209 utilizzando un ESP32S3. I dati vengono acquisiti tramite RS485 con protocollo Modbus RTU, salvati in formato JSON sulla memoria SPIFFS del dispositivo e possono essere letti successivamente.

## Hardware Necessario

- **ESP32S3**: microcontrollore principale per la gestione della comunicazione Modbus e la memorizzazione dei dati.
- **UPM209**: contatore di energia trifase con supporto Modbus RTU.
- **Modulo RS485**: interfaccia per la comunicazione con il contatore UPM209.
- **Memoria SPIFFS**: utilizzata per salvare i dati acquisiti in formato JSON.
- **Wi-Fi**: per eventuali aggiornamenti OTA.

## Funzionalità

- Lettura in tempo reale dei parametri elettrici dell'UPM209 tramite Modbus RTU.
- Salvataggio dei dati in formato JSON sulla memoria SPIFFS dell'ESP32S3.
- Possibilità di leggere i dati salvati e visualizzarli.
- Connessione Wi-Fi per future espansioni e aggiornamenti firmware via OTA.

## Supporto

Questo progetto supporta l'ESP32S3 con ambiente di sviluppo ESP-IDF. È possibile adattarlo ad altre varianti di ESP32 o dispositivi compatibili con Modbus RTU. Per eventuali problemi o richieste di supporto, si consiglia di consultare la documentazione ufficiale di ESP-IDF e delle librerie utilizzate.

---

## Configurazione Ambiente di Sviluppo

Per compilare il firmware, usa ESP-IDF o PlatformIO.

### ESP-IDF

```bash
idf.py set-target esp32s3
idf.py menuconfig   # Configura Wi-Fi e OTA
idf.py build flash monitor

### Configurazione dei Parametri

Modifica il file main.c per inserire le credenziali Wi-Fi e l'URL OTA:

#define WIFI_SSID "TuoSSID"
#define WIFI_PASS "TuaPassword"
#define OTA_URL   "http://tuoserver/firmware.bin"
