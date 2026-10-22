#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

#define POT_ADC_PIN   34
#define ALERT_LED_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Shared telemetry structure passed through FreeRTOS Queue
typedef struct {
  uint32_t timestamp_ms;
  uint16_t raw_adc;
  float calculated_voltage;
  uint8_t alert_state;
} SensorTelemetry_t;

// FreeRTOS Handles
static QueueHandle_t xTelemetryQueue = NULL;
static SemaphoreHandle_t xThresholdMutex = NULL;

// Shared configuration protected by mutex
static float g_voltage_threshold = 2.50f;

// ---------------------------------------------------------------------------
// TASK 1: Sensor Acquisition & Signal Conditioning (Pinned to Core 0)
// ---------------------------------------------------------------------------
void TaskSensorAcquisition(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10 Hz sampling rate

  SensorTelemetry_t telemetry;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    uint16_t adc_val = analogRead(POT_ADC_PIN);
    float voltage = (adc_val / 4095.0f) * 3.3f;

    // Read threshold safely under mutex protection
    float current_limit = 2.50f;
    if (xSemaphoreTake(xThresholdMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      current_limit = g_voltage_threshold;
      xSemaphoreGive(xThresholdMutex);
    }

    telemetry.timestamp_ms = millis();
    telemetry.raw_adc = adc_val;
    telemetry.calculated_voltage = voltage;
    telemetry.alert_state = (voltage >= current_limit) ? 1 : 0;

    // Post to queue without blocking if full (drops oldest frame pattern)
    xQueueOverwrite(xTelemetryQueue, &telemetry);

    // Hardware alert indication
    digitalWrite(ALERT_LED_PIN, telemetry.alert_state ? HIGH : LOW);
  }
}

// ---------------------------------------------------------------------------
// TASK 2: UI & OLED Rendering Engine (Pinned to Core 1)
// ---------------------------------------------------------------------------
void TaskDisplayUI(void *pvParameters) {
  SensorTelemetry_t rx_data;

  for (;;) {
    // Wait up to 500ms for incoming telemetry
    if (xQueueReceive(xTelemetryQueue, &rx_data, pdMS_TO_TICKS(500)) == pdTRUE) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      // Header
      display.setCursor(0, 0);
      display.print(F("RTOS CORE HUB"));

      display.setCursor(90, 0);
      display.print(rx_data.alert_state ? F("[!] ALM") : F("[OK]"));

      display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

      // Data fields
      display.setCursor(0, 16);
      display.print(F("ADC RAW : "));
      display.print(rx_data.raw_adc);

      display.setCursor(0, 28);
      display.print(F("VOLTAGE : "));
      display.print(rx_data.calculated_voltage, 2);
      display.print(F(" V"));

      display.setCursor(0, 40);
      display.print(F("LIMIT   : "));
      if (xSemaphoreTake(xThresholdMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        display.print(g_voltage_threshold, 2);
        xSemaphoreGive(xThresholdMutex);
      }
      display.print(F(" V"));

      // Mini level indicator bar
      int bar_width = map(rx_data.raw_adc, 0, 4095, 0, 124);
      display.drawRect(0, 53, 128, 9, SSD1306_WHITE);
      display.fillRect(2, 55, bar_width, 5, SSD1306_WHITE);

      display.display();
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Refresh UI at 10 FPS
  }
}

// ---------------------------------------------------------------------------
// TASK 3: Non-Blocking UART CLI Parser (Pinned to Core 1)
// ---------------------------------------------------------------------------
void TaskSerialCLI(void *pvParameters) {
  String input_line = "";

  for (;;) {
    while (Serial.available() > 0) {
      char c = Serial.read();

      if (c == '\n' || c == '\r') {
        input_line.trim();
        if (input_line.length() > 0) {
          // Command parser
          if (input_line.startsWith("SET_LIMIT ")) {
            float new_val = input_line.substring(10).toFloat();
            if (new_val > 0.1f && new_val <= 3.3f) {
              if (xSemaphoreTake(xThresholdMutex, portMAX_DELAY) == pdTRUE) {
                g_voltage_threshold = new_val;
                xSemaphoreGive(xThresholdMutex);
              }
              Serial.print(F("[CLI] SUCCESS: Threshold set to "));
              Serial.print(new_val, 2);
              Serial.println(F(" V"));
            } else {
              Serial.println(F("[CLI] ERROR: Limit must be between 0.1 and 3.3 V"));
            }
          } else if (input_line.equalsIgnoreCase("STATUS")) {
            Serial.println(F("--- SYSTEM STATUS ---"));
            Serial.print(F("Free Heap: "));
            Serial.print(esp_get_free_heap_size());
            Serial.println(F(" bytes"));
            Serial.print(F("Core 0 Task: TaskSensorAcquisition\n"));
            Serial.print(F("Core 1 Tasks: TaskDisplayUI, TaskSerialCLI\n"));
            Serial.println(F("---------------------"));
          } else if (input_line.equalsIgnoreCase("HELP")) {
            Serial.println(F("Available Commands:"));
            Serial.println(F("  SET_LIMIT <volts>  (e.g., SET_LIMIT 1.8)"));
            Serial.println(F("  STATUS             (Check memory and core allocation)"));
            Serial.println(F("  HELP               (Display command list)"));
          } else {
            Serial.print(F("[CLI] Unknown command: "));
            Serial.println(input_line);
          }
          input_line = "";
        }
      } else {
        input_line += c;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // Yield to prevent CPU starvation
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED_PIN, OUTPUT);
  digitalWrite(ALERT_LED_PIN, LOW);

  // Initialize I2C SSD1306 Display
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("[FATAL] SSD1306 allocation failed. Halting."));
    while (1);
  }
  display.clearDisplay();
  display.display();

  // Create Synchronization Primitives
  xTelemetryQueue = xQueueCreate(1, sizeof(SensorTelemetry_t));
  xThresholdMutex = xSemaphoreCreateMutex();

  if (xTelemetryQueue == NULL || xThresholdMutex == NULL) {
    Serial.println(F("[FATAL] FreeRTOS primitives creation failed."));
    while (1);
  }

  // Create and Pin Tasks to Specific Cores
  xTaskCreatePinnedToCore(
    TaskSensorAcquisition,
    "Sensor_Acq",
    3072,
    NULL,
    2, // Higher priority for deterministic sampling
    NULL,
    0  // Core 0
  );

  xTaskCreatePinnedToCore(
    TaskDisplayUI,
    "Display_UI",
    4096,
    NULL,
    1, // Medium priority
    NULL,
    1  // Core 1
  );

  xTaskCreatePinnedToCore(
    TaskSerialCLI,
    "Serial_CLI",
    3072,
    NULL,
    1, // Medium priority
    NULL,
    1  // Core 1
  );

  Serial.println(F("[SYSTEM] Multi-Core FreeRTOS Hub Initialized."));
  Serial.println(F("[SYSTEM] Type 'HELP' in the serial terminal for CLI commands."));
}

void loop() {
  // Empty: FreeRTOS task scheduler manages execution
  vTaskDelete(NULL);
}