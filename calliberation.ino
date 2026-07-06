#include <Wire.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

NAU7802 scale;

// ---------------- USER SETTINGS ----------------
const unsigned long WARMUP_TIME_MS = 60000; // 60s warm-up
// ------------------------------------------------

// Calibration values (found during calibration)
long zeroOffset = 0;
float calibrationFactor = 1.0;
float knownWeight = 0.0; // grams (set via serial)

// System state flags
bool warmupDone = false;
bool zeroDone = false;
bool scaleDone = false;

// Timing
unsigned long warmupStartTime;

// ------------------------------------------------
void printMenu()
{
  Serial.println("\n========== NAU7802 CALIBRATION MENU ==========");
  Serial.println("h : Show menu");
  Serial.println("s : Show system status");
  Serial.println("r : Read raw ADC values");
  Serial.println("k : Set known calibration weight (grams)");
  Serial.println("z : Zero calibration (tare)");
  Serial.println("c : Scale calibration (known weight)");
  Serial.println("w : Read calibrated weight (grams)");
  Serial.println("=============================================");
}

void printStatus()
{
  Serial.println("\n----- SYSTEM STATUS -----");
  Serial.print("Warm-up completed : ");
  Serial.println(warmupDone ? "YES" : "NO");

  Serial.print("Zero calibrated   : ");
  Serial.println(zeroDone ? "YES" : "NO");

  Serial.print("Scale calibrated  : ");
  Serial.println(scaleDone ? "YES" : "NO");

  Serial.print("Known weight (g)  : ");
  Serial.println(knownWeight);

  Serial.print("Zero Offset       : ");
  Serial.println(zeroOffset);

  Serial.print("Cal Factor        : ");
  Serial.println(calibrationFactor);
  Serial.println("-------------------------");
}

// ------------------------------------------------
float readFloatFromSerial()
{
  while (!Serial.available());
  String input = Serial.readStringUntil('\n');
  input.trim();
  return input.toFloat();
}

// ------------------------------------------------
void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();

  Serial.println("\nInitializing NAU7802...");

  if (!scale.begin())
  {
    Serial.println("FATAL ERROR: NAU7802 not detected.");
    Serial.println("Check wiring and 3.3V supply.");
    while (1);
  }

  // Proper load cell configuration
  scale.setGain(128);
  scale.setSampleRate(320);
  scale.calibrateAFE();

  Serial.println("NAU7802 initialized successfully.");

  // Start warm-up
  Serial.println("\n*** LOAD CELL WARM-UP STARTED ***");
  Serial.println("Do NOT place any load.");
  Serial.print("Warm-up time: ");
  Serial.print(WARMUP_TIME_MS / 1000);
  Serial.println(" seconds");

  warmupStartTime = millis();
  printMenu();
}

// ------------------------------------------------
void handleWarmup()
{
  unsigned long elapsed = millis() - warmupStartTime;

  if (scale.available())
  {
    long raw = scale.getReading();
    Serial.print("WARM-UP RAW ADC: ");
    Serial.println(raw);
  }

  if (elapsed >= WARMUP_TIME_MS)
  {
    warmupDone = true;
    Serial.println("\n*** WARM-UP COMPLETE ***");
    Serial.println("System is thermally stable.");
  }

  delay(500);
}

// ------------------------------------------------
void loop()
{
  // -------- BLOCK EVERYTHING UNTIL WARM-UP --------
  if (!warmupDone)
  {
    handleWarmup();
    return;
  }

  // -------- SERIAL COMMAND HANDLER --------
  if (Serial.available())
  {
    char cmd = Serial.read();

    // ---- HELP ----
    if (cmd == 'h')
    {
      printMenu();
    }

    // ---- STATUS ----
    else if (cmd == 's')
    {
      printStatus();
    }

    // ---- RAW ADC ----
    else if (cmd == 'r')
    {
      Serial.println("\nRaw ADC readings:");
      for (int i = 0; i < 10; i++)
      {
        while (!scale.available());
        Serial.println(scale.getReading());
        delay(300);
      }
    }

    // ---- SET KNOWN WEIGHT ----
    else if (cmd == 'k')
    {
      Serial.println("\nSET KNOWN CALIBRATION WEIGHT");
      Serial.println("Enter weight in grams and press ENTER:");

      float temp = readFloatFromSerial();

      if (temp <= 0)
      {
        Serial.println("ERROR: Weight must be greater than zero.");
        return;
      }

      knownWeight = temp;

      Serial.print("Known weight set to: ");
      Serial.print(knownWeight, 2);
      Serial.println(" g");
    }

    // ---- ZERO CALIBRATION ----
    else if (cmd == 'z')
    {
      Serial.println("\nZERO CALIBRATION (TARE)");
      Serial.println("Remove ALL load from the load cell.");
      Serial.println("Stabilizing...");
      delay(3000);

      scale.calculateZeroOffset();
      zeroOffset = scale.getZeroOffset();
      zeroDone = true;

      Serial.print("Zero Offset = ");
      Serial.println(zeroOffset);
      Serial.println("SAVE THIS VALUE.");
    }

    // ---- SCALE CALIBRATION ----
    else if (cmd == 'c')
    {
      if (!zeroDone)
      {
        Serial.println("ERROR: Perform zero calibration first (z).");
        return;
      }

      if (knownWeight <= 0)
      {
        Serial.println("ERROR: Known weight not set.");
        Serial.println("Use 'k' command to set weight.");
        return;
      }

      Serial.println("\nSCALE CALIBRATION");
      Serial.print("Place ");
      Serial.print(knownWeight, 2);
      Serial.println(" g on the load cell.");
      Serial.println("Waiting...");
      delay(5000);

      scale.calculateCalibrationFactor(knownWeight);
      calibrationFactor = scale.getCalibrationFactor();
      scaleDone = true;

      Serial.print("Calibration Factor = ");
      Serial.println(calibrationFactor);
      Serial.println("SAVE THIS VALUE.");
    }

    // ---- WEIGHT READ ----
    else if (cmd == 'w')
    {
      if (!zeroDone || !scaleDone)
      {
        Serial.println("ERROR: Calibration incomplete.");
        Serial.println("Perform z and c first.");
        return;
      }

      scale.setZeroOffset(zeroOffset);
      scale.setCalibrationFactor(calibrationFactor);

      Serial.println("\nLive weight readings (grams):");
      for (int i = 0; i < 10; i++)
      {
        while (!scale.available());
        Serial.println(scale.getWeight());
        delay(500);
      }
    }
  }
}
