/*

  main.cpp

*/
// Include Config-file (moved for code clarity)
#include "main_config.h"
const String fileVersion = __TIMESTAMP__;

// Create Sensor Objects with Specified Slave-Select Pins
ADNS adnsA(CS_PIN_A);
ADNS adnsB(CS_PIN_B);
sensor_pair_t sensor = {adnsA, adnsB};

// Capture Task (on interrupt)
IntervalTimer captureTimer;

// Counter and Timestamp Generator
elapsedMicros microsSinceAcquisitionStart;
elapsedMicros microsSinceFrameStart;
// volatile time_t currentSampleTimestamp;
volatile int32_t sampleCountRemaining = 0;
volatile time_t currentFrameTimestamp;
volatile time_t currentFrameDuration;
volatile uint32_t currentFrameCount;
volatile int32_t nreps = 0;
volatile bool isRunning = false;

// =============================================================================
//   SETUP & LOOP
// =============================================================================
#include "DeviceLib/devicemanager.h"
void setup() {
  delay(400);
  initializeCommunication();

  delay(400);

  initializeSensors();

  initializeTriggering();

  while (Serial.available()) {
    Serial.read();
  }

}

void loop() {
    if ((!isRunning) && (Serial.available() > 0)) {
        char matlab_input[50];
        beginAcquisition(matlab_input, 50);
        while (currentFrameCount < nreps) {
          ;
        }
        endAcquisition();
      }
  }


// =============================================================================
//   TASKS: INITIALIZE
// =============================================================================
inline static bool initializeCommunication() {
  // Begin Serial
  Serial.begin(115200);
  while (!Serial) {
    ;  // may only be needed for native USB port
  }
  delay(10);
  return true;
};
inline static bool initializeSensors() {
  // Begin Sensors
  sensor.left.begin();
  delay(30);
  sensor.right.begin();
  delay(30);
  return true;
};
inline static bool initializeTriggering() {
  fastPinMode(TRIGGER_PIN, OUTPUT);
  delay(1);
  // Setup Sync/Trigger-Output Timing
  // FrequencyTimer2::setPeriod(1e6 / DISPLACEMENT_SAMPLE_RATE)
  return true;
};

static inline void beginAcquisition(char input[], int8_t length) {
    delay(500);
    Serial.readBytes(input, length);
    //Parse input
    char *trial_length_minutes = strtok(input,",");
    float trial_length_minutes_int = atof(trial_length_minutes);
    char *sampling_interval_ms = strtok(NULL,",");
    float sampling_interval_ms_int = atof(sampling_interval_ms);
    nreps = floor(trial_length_minutes_int*60.0*1000.0/sampling_interval_ms_int);
    Serial.println(nreps);
    Serial.println(sampling_interval_ms_int);

    // Print units and Fieldnames (header)
    sendHeader();

    // Trigger start using class methods in ADNS library
    sensor.left.triggerAcquisitionStart();
    sensor.right.triggerAcquisitionStart();

    // Flush sensors (should happen automatically -> needs bug fix)
    sensor.left.triggerSampleCapture();
    sensor.right.triggerSampleCapture();

    // Change State
    isRunning = true;

    // Reset Elapsed Time Counter
    microsSinceAcquisitionStart = 0;
    // currentSampleTimestamp = microsSinceAcquisitionStart;
    microsSinceFrameStart = microsSinceAcquisitionStart;
    currentFrameDuration = microsSinceFrameStart;
    currentFrameCount = 0;
    // typedef void (*GeneralFunction) ();
    // GeneralFunction captureDisplacement();
    captureTimer.begin(captureDisplacement, sampling_interval_ms_int*1000);
}

static inline void beginDataFrame() {

  // Latch timestamp and designate/allocate current sample
  microsSinceFrameStart -= currentFrameDuration;
  currentFrameTimestamp = microsSinceAcquisitionStart;
}
static inline void endDataFrame() {
  // Latch Frame Duration and Send Data
  currentFrameDuration = microsSinceFrameStart;

}
static inline void endAcquisition() {
    // End IntervalTimer
    captureTimer.end();
    endDataFrame();

    // Trigger start using class methods in ADNS library
    sensor.left.triggerAcquisitionStop();
    sensor.right.triggerAcquisitionStop();

    // Change state
    isRunning = false;
}

// =============================================================================
// TASKS: TRIGGERED_ACQUISITION
// =============================================================================
void captureDisplacement() {
  // // Unset Trigger Outputs
  endDataFrame();

  // Initialize container for combined & stamped sample
  sensor_sample_t currentSample;
  currentSample.timestamp = currentFrameTimestamp; // maybe fix this time stamp issue?
  fastDigitalWrite(TRIGGER_PIN,HIGH);
  // Trigger capture from each sensor
  sensor.left.triggerSampleCapture();
  sensor.right.triggerSampleCapture();

  // Store timestamp for next frame
  currentFrameTimestamp = microsSinceAcquisitionStart;
  currentFrameCount += 1;
  currentSample.left = {'L', sensor.left.readDisplacement(units)};
  currentSample.right = {'R', sensor.right.readDisplacement(units)};

  // Send Data
  sendData(currentSample);
  fastDigitalWrite(TRIGGER_PIN,LOW);
  beginDataFrame();
}

// =============================================================================
// TASKS: DATA_TRANSFER
// =============================================================================

void sendHeader() {
  const String dunit = getAbbreviation(units.distance);
  const String tunit = getAbbreviation(units.time);
  // Serial.flush();
  Serial.print(String(
      String("timestamp [us]") + delimiter + flatFieldNames[0] + " [" + dunit +
      "]" + delimiter + flatFieldNames[1] + " [" + dunit + "]" + delimiter +
      flatFieldNames[2] + " [" + tunit + "]" + delimiter + flatFieldNames[3] +
      " [" + dunit + "]" + delimiter + flatFieldNames[4] + " [" + dunit + "]" +
      delimiter + flatFieldNames[5] + " [" + tunit + "]" + "\n"));
}

void sendData(sensor_sample_t sample) {

    // Convert to String class
    const String timestamp = String(sample.timestamp);
    const String dxL = String(sample.left.p.dx, decimalPlaces);
    const String dyL = String(sample.left.p.dy, decimalPlaces);
    const String dtL = String(sample.left.p.dt, decimalPlaces);
    const String dxR = String(sample.right.p.dx, decimalPlaces);
    const String dyR = String(sample.right.p.dy, decimalPlaces);
    const String dtR = String(sample.right.p.dt, decimalPlaces);
    const String endline = String("\n");

    // Serial.availableForWrite
    // Print ASCII Strings
    Serial.print(timestamp + delimiter + dxL + delimiter + dyL + delimiter +
                 dtL + delimiter + dxR + delimiter + dyR + delimiter + dtR +
                 endline);
}
