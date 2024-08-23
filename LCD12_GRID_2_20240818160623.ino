#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL605TnFnNR"
#define BLYNK_TEMPLATE_NAME "gh1"
#define BLYNK_AUTH_TOKEN "pjXqJ1ufzY92Tt-5Ggihf9q7OkRue4Cs"

#include <WiFiManager.h>
#include <TFT_eSPI.h>  // Pastikan untuk menggunakan library TFT yang sesuai
#include "xbm.h"
#include <EEPROM.h>
#include <time.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include <DFRobot_ESP_EC.h>
#include <BlynkSimpleEsp32.h>
TFT_eSPI tft = TFT_eSPI();  // Inisialisasi objek TFT

WiFiManager wm;  // Inisialisasi WifiManager
int timeout_hotspot = 120;

// Konstanta untuk ukuran grid
const int screenWidth = 480;
const int GRID_ROWS = 2;
const int GRID_COLS = 3;
const int CELL_WIDTH = 156;
const int CELL_HEIGHT = 145;
const int GRID_X_OFFSET = 2;
const int GRID_Y_OFFSET = 24;
const int BORDER_RADIUS = 10;  // Radius untuk sudut bulat
const int MARGIN = 10;         // Ukuran margin di setiap sel grid
const int GAP = 4;             // Jarak antar sel grid
const int MAX_STRING_LENGTH = 4;

// Ikon Bitmap
struct Logo {
  const unsigned char* data;
  int width;
  int height;
  int color;
};
const Logo logos[] = {
  { logo1, logo1Width, logo1Height, TFT_CYAN },
  { logo2, logo2Width, logo2Height, TFT_CYAN },
  { logo3, logo3Width, logo3Height, TFT_CYAN },
  { logo4, logo4Width, logo4Height, TFT_CYAN },
  { logo5, logo5Width, logo5Height, TFT_CYAN },
  { logo6, logo6Width, logo6Height, TFT_YELLOW },
};

//NTP SERVER
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;  // GMT+8 for Makassar time

unsigned long previousMillis = 0;  // Stores the last time the time was printed
const long interval = 1000;        // Interval at which to print time (milliseconds)

// PIN MANAGEMENT
// BUTTON
#define RESET_WIFI_PB5_PIN 13  // Tombol konfigurasi Wifi
#define RESET_ESP_PB5_PIN 12  // Tombol konfigurasi Wifi

//DHT21
#define DHT21_1_PIN 5
#define RELAY_1_PIN 14
#define RELAY_2_PIN
#define BUZZER_1_PIN 27

#define DHT_SENSOR_TYPE DHT21
#define TEMP_UPPER_THRESHOLD 32  // upper temperature threshold
#define TEMP_LOWER_THRESHOLD 28  // lower temperature threshold
#define HUM_UPPER_THRESHOLD 50  // upper humidity threshold
#define HUM_LOWER_THRESHOLD 80  // lower humidity threshold
#define W_TEMP_UPPER_THRESHOLD 50  // upper w_temperature threshold
#define W_TEMP_LOWER_THRESHOLD 80  // lower w_temperature threshold
#define TDS_UPPER_THRESHOLD 999  // upper tds threshold
#define TDS_LOWER_THRESHOLD 5  // lower tds threshold
#define PH_UPPER_THRESHOLD 14   // upper ph threshold
#define PH_LOWER_THRESHOLD 0  // lower ph threshold

//DS18B20
#define ONE_WIRE_BUS 17  // this is the gpio pin 18 on esp32.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//BLYNK


//STATE
int status_blower;
String blower;
float temperature;
float humidity;
float w_temperature;

const int potPin=34;
float ph,nilai_ph;
float Value=0;

DFRobot_ESP_EC ec;
Adafruit_ADS1115 ads;

float voltage;
float ecValue;
float tDS;
int16_t adc0;
int16_t adc1;
float volts0;
float volts1;

String status_dht;
String status_ds;
String status_tds;
String status_ph;

DHT dht21(DHT21_1_PIN, DHT_SENSOR_TYPE);



// Gradient Drawing Function
void drawVerticalGradient(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color1, uint16_t color2) {
  for (int16_t i = 0; i < h; i++) {
    uint16_t color = tft.color565(
      (color1 >> 11) * (1 - (float)i / h) + (color2 >> 11) * (float)i / h,
      (((color1 >> 5) & 0x3F) * (1 - (float)i / h)) + (((color2 >> 5) & 0x3F) * (float)i / h),
      (color1 & 0x1F) * (1 - (float)i / h) + (color2 & 0x1F) * (float)i / h);
    tft.drawLine(x, y + i, x + w, y + i, color);
  }
}



// Function to truncate text with ellipsis
String truncateText(String text, int maxWidth) {
  int textWidth = tft.textWidth(text);

  if (textWidth <= (maxWidth - MARGIN)) {
    return text;
  }

  String truncatedText = text;
  int ellipsisWidth = tft.textWidth("...");

  // Remove characters until the text fits within the max width
  while (textWidth + ellipsisWidth > maxWidth && truncatedText.length() > 0) {
    truncatedText.remove(truncatedText.length() - 1);
    textWidth = tft.textWidth(truncatedText);
  }

  // Add ellipsis
  truncatedText += "...";

  return truncatedText;
}

void drawHeader(String waktu_sekarang, String nama_wifi) {
  if (nama_wifi.length() == 0) {
    nama_wifi = "-";
  }
  // Set text color, size, and font
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextFont(2);
  // Text segments
  String text1 = waktu_sekarang;
  String text2 = "GREEN-HOUSE";
  String text3 = "WiFi : " + nama_wifi;

  // Measure text widths
  int text1Width = tft.textWidth(text1);
  int text2Width = tft.textWidth(text2);
  int text3Width = tft.textWidth(text3);

  // Calculate positions
  int text1X = 4;                               // Left-aligned
  int text2X = (screenWidth - text2Width) / 2;  // Center-aligned
  int text3X = screenWidth - text3Width - 4;    // Right-aligned
  // Draw a white line at the top
  tft.fillRect(2, 3, CELL_WIDTH, 15, TFT_BLACK);
  tft.fillRect(161, 3, CELL_WIDTH, 15, TFT_BLACK);
  tft.fillRect(320, 3, CELL_WIDTH, 15, TFT_BLACK);
  tft.drawRoundRect(0, 0, screenWidth, 20, 5, TFT_WHITE);
  // Set text cursor positions and print text
  tft.setCursor(text1X, 2);  // 2 is the Y coordinate, adjust as needed
  tft.print(text1);

  tft.setCursor(text2X, 2);
  tft.print(truncateText(text2, CELL_WIDTH));

  tft.setCursor(text3X, 2);
  tft.print(truncateText(text3, CELL_WIDTH));

  tft.setTextFont(1);
}


// Fungsi untuk menggambar grid dengan sudut bulat
void drawGrid() {
  tft.fillScreen(TFT_BLACK);  // Mengatur latar belakang layar
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int x = GRID_X_OFFSET + col * (CELL_WIDTH + GAP);
      int y = GRID_Y_OFFSET + row * (CELL_HEIGHT + GAP);

      // Mengisi latar belakang sel grid dengan warna solid
      // tft.fillRoundRect(x, y, CELL_WIDTH, CELL_HEIGHT, BORDER_RADIUS, TFT_DARKGREY);

      // Menggambar garis border sel grid dengan sudut bulat
      tft.drawRoundRect(x, y, CELL_WIDTH, CELL_HEIGHT, BORDER_RADIUS, TFT_WHITE);

      // Menggambar ikon di setiap sel (sesuaikan ukuran ikon)

      // Warna gradien untuk latar belakang
      uint16_t color1 = TFT_BLUE;   // Warna awal gradien
      uint16_t color2 = TFT_WHITE;  // Warna akhir gradien
      // drawGradientBackground(x + MARGIN, y + MARGIN, CELL_WIDTH * MARGIN, CELL_HEIGHT * MARGIN, TFT_GREEN, TFT_BLUE);

      // tft.drawBitmap(x + MARGIN, y + MARGIN, logo, 16, 16, TFT_YELLOW, TFT_DARKGREY);
      // Pilih logo berdasarkan indeks
      // Select logo based on the index
      int logoIndex = row * GRID_COLS + col;  // Calculate index for logo selection
      if (logoIndex >= 0 && logoIndex < sizeof(logos) / sizeof(logos[0])) {
        // Get logo data and dimensions
        const Logo& logo = logos[logoIndex];

        // Draw the logo within the cell
        tft.drawXBitmap(x + MARGIN, y + MARGIN, logo.data, logo.width, logo.height, logo.color);
      }
    }
  }
}

// Fungsi untuk menggambar latar belakang gradien di setiap sel grid
void drawGradientBackground(int x, int y, int width, int height, uint16_t color1, uint16_t color2) {
  for (int i = 0; i < height; i++) {
    uint16_t color = tft.color565(
      (color1 >> 11) * (1 - (float)i / height) + (color2 >> 11) * (float)i / height,
      (((color1 >> 5) & 0x3F) * (1 - (float)i / height)) + (((color2 >> 5) & 0x3F) * (float)i / height),
      (color1 & 0x1F) * (1 - (float)i / height) + (color2 & 0x1F) * (float)i / height);
    tft.drawLine(x, y + i, x + width, y + i, color);
  }
}

// Fungsi untuk menampilkan nilai di sel grid
void drawJudul(String judul[GRID_ROWS][GRID_COLS]) {
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int x = GRID_X_OFFSET + col * (CELL_WIDTH + GAP) + MARGIN + 45;  // Sesuaikan posisi agar tidak tertutup ikon
      int y = GRID_Y_OFFSET + row * (CELL_HEIGHT + GAP) + MARGIN + 10;

      // Mengatur posisi kursor untuk teks agar berada di dalam margin
      tft.setCursor(x, y);

      // Menampilkan nilai di dalam sel
      tft.println(judul[row][col]);
    }
  }
}

void drawValues(String values[GRID_ROWS][GRID_COLS]) {
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int x = GRID_X_OFFSET + col * (CELL_WIDTH + GAP) + MARGIN + 10;  // Sesuaikan posisi agar tidak tertutup ikon
      int y = GRID_Y_OFFSET + row * (CELL_HEIGHT + GAP) + MARGIN + 50;

      // Prepare the string to display
      String displayValue = values[row][col];

      // Truncate to the maximum length of MAX_STRING_LENGTH
      if (displayValue.length() > MAX_STRING_LENGTH) {
        displayValue = displayValue.substring(0, MAX_STRING_LENGTH);
      }

      // Optionally, pad with spaces if you want each cell to display exactly MAX_STRING_LENGTH characters
      while (displayValue.length() < MAX_STRING_LENGTH) {
        displayValue += ' ';  // Add a space to pad
      }

      // Mengatur posisi kursor untuk teks agar berada di dalam margin
      tft.setCursor(x, y);

      // Reset nilai
      tft.fillRect(x, y, 130, 30, TFT_BLACK);

      // Menampilkan nilai di dalam sel
      tft.println(displayValue);
    }
  }
}

void drawInfo(String info[GRID_ROWS][GRID_COLS]) {
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int x = GRID_X_OFFSET + col * (CELL_WIDTH + GAP) + MARGIN + 10;  // Sesuaikan posisi agar tidak tertutup ikon
      int y = GRID_Y_OFFSET + row * (CELL_HEIGHT + GAP) + MARGIN + 90;

      // Mengatur posisi kursor untuk teks agar berada di dalam margin
      tft.setCursor(x, y);
      if (info[row][col] == "NORMAL") {
        tft.setTextColor(TFT_GREEN);
      } else if (info[row][col] == "ABNORMAL") {
        tft.setTextColor(TFT_YELLOW);
      } else {
        tft.setTextColor(TFT_RED);
      }

      // reset nilai
      tft.fillRect(x, y, 130, 15, TFT_BLACK);

      // Menampilkan nilai di dalam sel
      tft.println(info[row][col]);
    }
  }
}

String printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String("00-00-0000 00:00:00");
  }

  // Create a buffer to hold the formatted time
  char timeBuffer[20];  // Sufficient size for "DD-MM-YYYY HH:MM:SS"

  // Format the time as "DD-MM-YYYY HH:MM:SS"
  snprintf(timeBuffer, sizeof(timeBuffer),
           "%02d-%02d-%04d %02d:%02d:%02d",
           timeinfo.tm_mday,         // Day of month
           timeinfo.tm_mon + 1,      // Month (0-based, so add 1)
           timeinfo.tm_year + 1900,  // Year (years since 1900)
           timeinfo.tm_hour,         // Hour
           timeinfo.tm_min,          // Minute
           timeinfo.tm_sec);         // Second

  // Serial.println("Formatted Time: " + String(timeBuffer));
  // Print the formatted time
  return String(timeBuffer);
}

void cek_dht() {
    temperature = dht21.readTemperature();
    humidity = dht21.readHumidity();
}

void cek_ds() {
    sensors.requestTemperatures();
    w_temperature = sensors.getTempCByIndex(0); 
}

void cek_ph() {
    Value= analogRead(potPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    nilai_ph = ph ;
}

void cek_tds(){
    adc0 = ads.readADC_SingleEnded(0);
    volts0 = ads.computeVolts(adc0);
    voltage = ads.readADC_SingleEnded(0) / 10;
    ecValue = ec.readEC(voltage, w_temperature);  // convert voltage to EC with temperature compensation
    tDS = ((ecValue * 100) / 1.1);
}

void setup() {
  Serial.begin(115200);

  // Inisial pinMode
  pinMode(RESET_WIFI_PB5_PIN, INPUT_PULLUP);
  pinMode(RESET_ESP_PB5_PIN, INPUT_PULLUP);
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(potPin,INPUT);
  digitalWrite(RELAY_1_PIN, LOW);

  // ======================================================================== Inisial TFT
  tft.init();
  tft.setRotation(1);         // Menyesuaikan orientasi layar jika perlu
  tft.fillScreen(TFT_BLACK);  // Mengatur latar belakang layar
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  // Mengatur posisi kursor untuk teks agar berada di dalam margin
  tft.setCursor(0, 0);
  // Menampilkan nilai di dalam sel
  tft.print("Memulai");
  delay(1000);
  tft.print(".");
  delay(1000);
  tft.print(".");
  delay(1000);
  tft.println(".");
  delay(1000);
  // ======================================================================== Inisial TFT

  //======================================================================= Setup Wifi Manager
  //wm.resetSettings();  // Pakai ini jika dalam mode DEV
  tft.println("Menghubungkan Wifi");
  bool res = wm.autoConnect("GREEN-HOUSE", "adalah12345");

  if (res) {
    //if you get here you have connected to the WiFi
    Serial.println("connected... :)");
    tft.print("Wifi Terhubung : ");
    tft.println(WiFi.SSID());
    delay(500);
    configTime(gmtOffset_sec, 0, ntpServer);
    tft.print("Mencari Waktu : ");
    Serial.println(printLocalTime());
    tft.println(printLocalTime());
    delay(1000);
    tft.print("Mengecek sensor");

    //=======================================================================SETUP DHT21
    dht21.begin();  // initialize the DHT sensor
    //=======================================================================SETUP DHT21

    //=======================================================================SETUP DS18B20
    sensors.begin();
    //=======================================================================SETUP DS18B20
    
    //=======================================================================SETUP TDS
    EEPROM.begin(32);  //needed EEPROM.begin to store calibration k in eeprom
    ec.begin();
    ads.setGain(GAIN_ONE);
    ads.begin();
    //=======================================================================SETUP TDS

    delay(200);
    cek_dht();
    delay(200);
    tft.print(".");
    cek_ph();
    delay(200);
    tft.print(".");
    cek_ds();
    delay(200);
    tft.println(".");
    cek_tds();
    delay(500);
    tft.print("Sensor TDS : ");
    if (tDS > 2.2 && tDS <= 990 ) {
    tft.setTextColor(TFT_GREEN);
    tft.println("NORMAL");
    status_tds = "NORMAL";
    } else if (tDS < 2.2 ) {
    tft.setTextColor(TFT_RED);
    tft.println("ERROR");
    status_tds = "ERROR";
    } else {
    tft.setTextColor(TFT_YELLOW);
    tft.println("ABNORMAL");
    status_tds = "ABNORMAL";
    }

    tft.setTextColor(TFT_WHITE);
    delay(200);
    tft.print("Sensor pH : ");
    if (nilai_ph > 0 && nilai_ph <= 14) {
        if (nilai_ph >= 6.5 && nilai_ph <= 8.5) {
            tft.setTextColor(TFT_GREEN);
            tft.println("NORMAL");
            status_ph = "NORMAL";
        } else {
            tft.setTextColor(TFT_YELLOW);
            tft.println("ABNORMAL");
            status_ph = "ABNORMAL";
        }
    } else {
        tft.setTextColor(TFT_RED);
        tft.println("ERROR");
        status_ph = "ERROR";
    }
    delay(200);
    tft.setTextColor(TFT_WHITE);
    tft.print("Sensor suhu air : ");
    if (w_temperature < TEMP_UPPER_THRESHOLD || w_temperature > TEMP_LOWER_THRESHOLD) {
    tft.setTextColor(TFT_GREEN);
    tft.println("NORMAL");
    status_ds = "NORMAL";
    }
    else if (w_temperature > TEMP_UPPER_THRESHOLD || w_temperature < TEMP_LOWER_THRESHOLD) {
    tft.setTextColor(TFT_YELLOW);
    tft.println("ABNORMAL");
    status_ds = "ABNORMAL";
    } else {
    tft.setTextColor(TFT_RED);
    tft.println("ERROR");
    status_ds = "ERROR";
    }
    delay(200);
    tft.setTextColor(TFT_WHITE);
    tft.print("Sensor suhu & kelembapan : ");
    if (humidity < HUM_UPPER_THRESHOLD || humidity > HUM_LOWER_THRESHOLD) {
    tft.setTextColor(TFT_GREEN);
    tft.println("NORMAL");
    status_dht = "NORMAL";
    }
    else if (humidity > HUM_UPPER_THRESHOLD || humidity < HUM_LOWER_THRESHOLD) {
    tft.setTextColor(TFT_YELLOW);
    tft.println("ABNORMAL");
    status_dht = "ABNORMAL";
    } else {
    tft.setTextColor(TFT_RED);
    tft.println("ERROR");
    status_dht = "ERROR";
    }
    delay(1000);
    tft.setTextColor(TFT_WHITE);
    tft.print("Menghubungkan BLYNK :");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    delay(1000);
   
    if (Blynk.connected()) {
    tft.setTextColor(TFT_GREEN);
    tft.println("OK");
    Serial.println("Blynk connected.");
  } else {
    tft.setTextColor(TFT_RED);
    tft.println("GAGAL");
    Serial.println("Blynk not connected.");
  }
    delay(5000);
    // ESP.restart();
    

  } else {
    Serial.println("Gagal Mengkoneksikan Wifi");
    tft.println("Gagal Mengkoneksikan Wifi");
    tft.println("Konfigurasi Wifi");
    tft.println("Konek ke WiFi GREEN-HOUSE untuk melakukan konfigurasi");
    tft.println("atau scan kode QR");
  }
  //=======================================================================Setup Wifi Manager

  // Contoh nilai yang akan ditampilkan
  String judul[GRID_ROWS][GRID_COLS] = {
    { "TDS(PPM)", "KADAR pH", "SUHU-AIR" },
    { "SUHU", "LEMBAP", "BLOWER" },
  };
  String values[GRID_ROWS][GRID_COLS] = {
    { "1000", "7.8", "33.0C" },
    { "33.0C", "20%", "OFF" },
  };
  String info[GRID_ROWS][GRID_COLS] = {
    { "NORMAL", "NORMAL", "NORMAL" },
    { "NORMAL", "NORMAL", "NORMAL" },
  };

  // drawValues(values);
  // drawInfo(info);
  drawGrid();
  drawJudul(judul);

}

void loop() {
  // ======================================================================= Wifi Handle
  // Print Wi-Fi status
  if (WiFi.status() == WL_CONNECTED) {
    // Serial.println("Wi-Fi status: Connected");
  } else {
    Serial.print("Wi-Fi status: ");
    Serial.println(WiFi.status());
  }

   if (digitalRead(RESET_ESP_PB5_PIN) == LOW) {
    ESP.restart();
   }

  // Check if RESET_WIFI_PB5_PIN is pressed
  if (digitalRead(RESET_WIFI_PB5_PIN) == LOW) {
    Serial.println("Button pressed, starting configuration portal...");
    tft.fillScreen(TFT_BLACK);  // Mengatur latar belakang layar
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    // Mengatur posisi kursor untuk teks agar berada di dalam margin
    tft.setCursor(0, 0);
    // Menampilkan nilai di dalam sel
    tft.println("Konfigurasi Wifi");
    tft.println("Konek ke WiFi GREEN-HOUSE untuk melakukan konfigurasi");
    tft.println("atau scan kode QR");
    // Set a timeout for the configuration portal
    wm.setConfigPortalTimeout(timeout_hotspot);

    // Start configuration portal
    if (!wm.startConfigPortal("GREEN-HOUSE", "adalah12345")) {
      Serial.println("Failed to connect and hit timeout");
      tft.println("Waktu habis");
      delay(1000);
      Serial.println("Restarting");
      tft.println("Mereset...");
      delay(3000);
      // Reset the ESP or take other action
      ESP.restart();
      delay(5000);
    } else {
      Serial.println("Connected to WiFi after configuration!");
      delay(1000);
      tft.print("Wifi Terhubung : ");
      tft.println(WiFi.SSID());
    }
    delay(2000);  // Debounce delay
  }
  // ======================================================================= Wifi Handle


  // ======================================================================= Mengambil Waktu
  unsigned long currentMillis = millis();
  // Check if a second has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the last print time
    printLocalTime();                // Print the current time

    temperature = dht21.readTemperature();
    humidity = dht21.readHumidity();
    Serial.print(temperature);
    Serial.print(" C | ");
    Serial.println(humidity);

    if (isnan(temperature)) {
      Serial.println("Failed to read from DHT21 sensor!");
    } else {
      if (temperature > TEMP_UPPER_THRESHOLD) {
        // Serial.println("Turn the relay ON");
        digitalWrite(RELAY_1_PIN, HIGH);  // turn on
        status_blower = 1;
        blower = "ON";
      } else {
        // Serial.println("Turn the relay OFF");
        digitalWrite(RELAY_1_PIN, LOW);  // turn on
        status_blower = 0;
        blower = "OFF";
      }
    }

    //DS18B20
    sensors.requestTemperatures();
    w_temperature = sensors.getTempCByIndex(0); 

    //PH Sensor
    Value= analogRead(potPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    nilai_ph = ph ;
    Serial.println(nilai_ph);
    delay(200);

    adc0 = ads.readADC_SingleEnded(0);
    volts0 = ads.computeVolts(adc0);
    voltage = ads.readADC_SingleEnded(0) / 10;
    ecValue = ec.readEC(voltage, w_temperature);  // convert voltage to EC with temperature compensation
    tDS = ((ecValue * 100) / 1.1);
    delay(200);
    Serial.print("TDS : ");
    Serial.println(tDS);
  }
  // ======================================================================= Mengambil Waktu

  String values[GRID_ROWS][GRID_COLS] = {
    { String(tDS), String(nilai_ph), String(w_temperature) },
    { String(temperature), String(humidity), String(blower) },
  };

  delay(100);

    if (tDS >= 2.2 && tDS <= 990) {
        status_tds = "NORMAL";
    } else if (tDS < 22.2) {
        status_tds = "ERROR";
    } else {
        status_tds = "ABNORMAL";
    }

    delay(200);
    if (nilai_ph > 0 && nilai_ph <= 14) {
        if (nilai_ph >= 6.5 && nilai_ph <= 8.5) {
            status_ph = "NORMAL";
        } else {
            status_ph = "ABNORMAL";
        }
    } else {
        status_ph = "ERROR";
    }
    delay(200);

    if (w_temperature < TEMP_UPPER_THRESHOLD || w_temperature > TEMP_LOWER_THRESHOLD) {
    status_ds = "NORMAL";
    }
    else if (w_temperature > TEMP_UPPER_THRESHOLD || w_temperature < TEMP_LOWER_THRESHOLD) {
    status_ds = "ABNORMAL";
    } else {
    status_ds = "ERROR";
    }
    delay(200);

    if (humidity < HUM_UPPER_THRESHOLD || humidity > HUM_LOWER_THRESHOLD) {
    status_dht = "NORMAL";
    }
    else if (humidity > HUM_UPPER_THRESHOLD || humidity < HUM_LOWER_THRESHOLD) {
    status_dht = "ABNORMAL";
    } else {
    status_dht = "ERROR";
    }
    delay(2000);

  
    String info[GRID_ROWS][GRID_COLS] = {
    { status_tds, status_ph, status_ds },
    { status_dht, status_dht, "" },
  };
  
  delay(2000);

  // ======================================================================= TFT DRAW
  drawHeader(printLocalTime(), WiFi.SSID());
  drawValues(values);
  drawInfo(info);
  // ======================================================================= TFT DRAW

  Blynk.virtualWrite(V0, tDS);
  Blynk.virtualWrite(V1, nilai_ph);
  Blynk.virtualWrite(V2, w_temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, status_blower);
}
