## English

# Energy Monitoring with ESP32 and UPM209

## Description

This project allows real-time monitoring of the energy data from the UPM209 meter using an ESP32S3. The data is acquired via RS485 with the Modbus RTU protocol, saved in JSON format on the device's SPIFFS memory, and can be read later.

## Required Hardware

- **ESP32S3**: the main microcontroller for managing Modbus communication and data storage.
- **UPM209**: three-phase energy meter with Modbus RTU support.
- **RS485 Module**: interface for communication with the UPM209 meter.
- **SPIFFS Memory**: used to save the acquired data in JSON format.
- **Wi-Fi**: for potential OTA updates.

## Features

- Real-time reading of electrical parameters from the UPM209 via Modbus RTU.
- Saving data in JSON format on the ESP32S3's SPIFFS memory.
- Ability to read and display the saved data.
- Wi-Fi connectivity for future expansions and firmware updates via OTA.

## Support

This project supports the ESP32S3 with the ESP-IDF development environment. It can be adapted to other ESP32 variants or devices compatible with Modbus RTU. For any issues or support requests, it is recommended to consult the official documentation of ESP-IDF and the libraries used.

---

## Development Environment Setup

To compile the firmware, use ESP-IDF or PlatformIO.

### ESP-IDF

```bash
idf.py set-target esp32s3
idf.py menuconfig   # Configure Wi-Fi and OTA
idf.py build flash monitor
```

### Configuration Parameters

Modify the `main.c` file to insert your Wi-Fi credentials and OTA URL:
```c
#define WIFI_SSID "YourSSID"
#define WIFI_PASS "YourPassword"
#define OTA_URL   "http://yourserver/firmware.bin"
```

## Italian
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
```
### Configurazione dei Parametri

Modifica il file main.c per inserire le credenziali Wi-Fi e l'URL OTA:
```bash
#define WIFI_SSID "TuoSSID"
#define WIFI_PASS "TuaPassword"
#define OTA_URL   "http://tuoserver/firmware.bin"
```
