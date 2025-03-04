#include <Wire.h>
const byte PIN_QWIIC_SCL = 8;
const byte PIN_QWIIC_SDA = 9;
TwoWire qwiic(PIN_QWIIC_SDA,PIN_QWIIC_SCL); //Will use pads 8/9

const byte PIN_QWIIC_POWER = 18;

//Define the pin functions
//Depends on hardware version. This can be found as a marking on the PCB.
//x04 was the SparkX 'black' version.
//v10 was the first red version.
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0

//Header files for all possible Qwiic sensors
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "ICM_20948.h" // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
ICM_20948_I2C IMU_ICM20948; // Otherwise create an ICM_20948_I2C object

#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_NAU7802
NAU7802 loadcellSensor_NAU7802;

#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
SFEVL53L1X distanceSensor_VL53L1X(qwiic);

#include "SparkFun_u-blox_GNSS_Arduino_Library.h" //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS gpsSensor_ublox;

#include "SparkFun_VCNL4040_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VCNL4040
VCNL4040 proximitySensor_VCNL4040;

#include "SparkFun_MCP9600.h" //Click here to get the library: http://librarymanager/All#SparkFun_MCP9600
MCP9600 thermoSensor_MCP9600;

#include "SparkFun_TMP117.h" //Click here to get the library: http://librarymanager/All#SparkFun_TMP117
TMP117 tempSensor_TMP117;

#include "SparkFun_MS5637_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_MS5637
MS5637 pressureSensor_MS5637;

#include "SparkFun_LPS25HB_Arduino_Library.h"  //Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB pressureSensor_LPS25HB;

#include "SparkFunBME280.h" //Click here to get the library: http://librarymanager/All#SparkFun_BME280
BME280 phtSensor_BME280;

#include "SparkFun_VEML6075_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VEML6075
VEML6075 uvSensor_VEML6075;

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#define ADR_CCS811_1 0x5B
CCS811 vocSensor_CCS811(ADR_CCS811_1);

#include "SparkFun_SGP30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SGP30
SGP30 vocSensor_SGP30;

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 co2Sensor_SCD30;

#include "SparkFun_PHT_MS8607_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_PHT_MS8607
MS8607 pressureSensor_MS8607;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct struct_QwiicSensors {
  bool ICM_20948_I2C;
  bool LPS25HB;
  bool MCP9600;
  bool BH1749NUC;
  bool NAU7802;
  bool uBlox;
  bool VL53L1X;
  bool VCNL4040;
  bool TMP117;
  bool CCS811;
  bool BME280;
  bool SGP30;
  bool VEML6075;
  bool MS5637;
  bool SCD30;
  bool MS8607;
};

struct_QwiicSensors qwiicAvailable = {
  .ICM_20948_I2C = false,
  .LPS25HB = false,
  .MCP9600 = false,
  .BH1749NUC = false,
  .NAU7802 = false,
  .uBlox = false,
  .VL53L1X = false,
  .VCNL4040 = false,
  .TMP117 = false,
  .CCS811 = false,
  .BME280 = false,
  .SGP30 = false,
  .VEML6075 = false,
  .MS5637 = false,
  .SCD30 = false,
  .MS8607 = false,
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Scanning...");

  qwiicPowerOn();

  qwiic.begin();
  qwiic.setClock(100000);

  setQwiicPullups(1); //Set pullups to 1K5
}

void loop()
{
  qwiicPowerOff();
  delay(1000);
  qwiicPowerOn();
  //delay(1000); //SCD30 acks and responds
  //delay(100); //SCD30 acks but fails to start
  delay(1000); //u-blox GNSS needs at least 1s
  Serial.println("On!");
  Serial.flush();

  bool somethingDetected = false;

  for (uint8_t address = 0x10 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      Serial.printf("Device found at address 0x%02X\n", address);
      if (testDevice(address) == false)
        Serial.printf("Unknown I2C device found at address 0x%02X\n", address);
      else
        somethingDetected = true;
    }
  }

  if (somethingDetected == false)
    Serial.println("Nothing detected");
}

// Available Qwiic devices
#define ADR_VEML6075 0x10
#define ADR_NAU7802 0x2A
#define ADR_VL53L1X 0x29
#define ADR_MS8607 0x40 //Humidity portion of the MS8607 sensor
#define ADR_UBLOX 0x42
#define ADR_TMP117 0x48 //Alternates: 0x49, 0x4A, and 0x4B
#define ADR_SGP30 0x58
#define ADR_CCS811_2 0x5A
//#define ADR_CCS811_1 0x5B
#define ADR_LPS25HB_2 0x5C
#define ADR_LPS25HB_1 0x5D
#define ADR_VCNL4040_OR_MCP9600 0x60
#define ADR_SCD30 0x61
#define ADR_MCP9600_1 0x66
#define ADR_ICM_20948_AD0 0x68 // Or 0x69 when AD0 is high
#define ADR_ICM_20948_AD1 0x69
#define ADR_BME280_2 0x76
#define ADR_MS5637 0x76
//#define ADR_MS8607 0x76 //Pressure portion of the MS8607 sensor. We'll catch the 0x40 first
#define ADR_BME280_1 0x77

//Given an address, see if it repsonds as we would expect
//Returns false if I2C address is not known
bool testDevice(uint8_t i2cAddress)
{
  switch (i2cAddress)
  {
    case ADR_ICM_20948_AD0:
      if (IMU_ICM20948.begin(qwiic, false) == true) //Wire port, ad0val
        qwiicAvailable.ICM_20948_I2C = true;
      break;
    case ADR_ICM_20948_AD1:
      if (IMU_ICM20948.begin(qwiic, true) == true) //Wire port, ad0val
        qwiicAvailable.ICM_20948_I2C = true;
      break;
    case ADR_LPS25HB_1:
      if (pressureSensor_LPS25HB.begin(qwiic, ADR_LPS25HB_1) == true) //Wire port, Address.
        if (pressureSensor_LPS25HB.isConnected() == true)
          qwiicAvailable.LPS25HB = true;
      break;
    case ADR_LPS25HB_2:
      if (pressureSensor_LPS25HB.begin(qwiic, ADR_LPS25HB_2) == true) //Wire port, Address.
        if (pressureSensor_LPS25HB.isConnected() == true)
          qwiicAvailable.LPS25HB = true;
      break;
    case ADR_MCP9600_1:
      if (thermoSensor_MCP9600.begin(ADR_MCP9600_1, qwiic) == true) //Address, Wire port
        if (thermoSensor_MCP9600.isConnected() == true)
          qwiicAvailable.MCP9600 = true;
      break;
    case ADR_VCNL4040_OR_MCP9600:
      //      if (thermoSensor_MCP9600.begin(ADR_MCP9600_1, qwiic) == true) //Address, Wire port
      //        if (thermoSensor_MCP9600.isConnected() == true)
      //          qwiicAvailable.MCP9600 = true;
      if (proximitySensor_VCNL4040.begin(qwiic) == true) //Wire port. Checks ID so should avoid collision with MCP9600
        qwiicAvailable.VCNL4040 = true;
      break;
    case ADR_NAU7802:
      if (loadcellSensor_NAU7802.begin(qwiic) == true) //Wire port
        qwiicAvailable.NAU7802 = true;
      break;
    case ADR_UBLOX:
      if (gpsSensor_ublox.begin(qwiic, ADR_UBLOX) == true) //Wire port, address
        qwiicAvailable.uBlox = true;
      break;
    case ADR_VL53L1X:
      if (distanceSensor_VL53L1X.begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
        qwiicAvailable.VL53L1X = true;
      break;
    case ADR_TMP117:
      if (tempSensor_TMP117.begin(ADR_TMP117, qwiic) == true) //Adr, Wire port
        qwiicAvailable.TMP117 = true;
      break;
    case ADR_CCS811_1:
      if (vocSensor_CCS811.begin(qwiic) == true) //Wire port
        qwiicAvailable.CCS811 = true;
      break;
    case ADR_BME280_1:
      if (phtSensor_BME280.beginI2C(qwiic) == true) //Wire port
        qwiicAvailable.BME280 = true;
      break;
    case ADR_SGP30:
      if (vocSensor_SGP30.begin(qwiic) == true) //Wire port
        qwiicAvailable.SGP30 = true;
      break;
    case ADR_VEML6075:
      if (uvSensor_VEML6075.begin(qwiic) == true) //Wire port
        qwiicAvailable.VEML6075 = true;
      break;
    case ADR_MS5637:
      {
        //By the time we hit this address, MS8607 should have already been started by its first address
        if (qwiicAvailable.MS8607 == false)
        {
          if (pressureSensor_MS5637.begin(qwiic) == true) //Wire port
            qwiicAvailable.MS5637 = true;
        }
        break;
      }
    case ADR_SCD30:
      if (co2Sensor_SCD30.begin(qwiic) == true) //Wire port
        qwiicAvailable.SCD30 = true;
      else
      {
        //Give it 2s to boot and then try again
        delay(2000);
        if (co2Sensor_SCD30.begin(qwiic) == true) //Wire port
        {
          qwiicAvailable.SCD30 = true;
          Serial.println("SCD30 found on second attempt!");
        }
      }
      break;
    case ADR_MS8607:
      if (pressureSensor_MS8607.begin(qwiic) == true) //Wire port. Tests for both 0x40 and 0x76 I2C addresses.
        qwiicAvailable.MS8607 = true;
      break;
    default:
      Serial.printf("Unknown device at address 0x%02X\n", i2cAddress);
      return false;
      break;
  }
  return true;
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#else
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#else
  digitalWrite(PIN_QWIIC_POWER, LOW);
#endif
}

void setQwiicPullups(uint32_t qwiicBusPullUps)
{
  //Change SCL and SDA pull-ups manually using pin_config
  am_hal_gpio_pincfg_t sclPinCfg = g_AM_BSP_GPIO_IOM1_SCL;
  am_hal_gpio_pincfg_t sdaPinCfg = g_AM_BSP_GPIO_IOM1_SDA;

  if (qwiicBusPullUps == 0)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE; // No pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE;
  }
  else if (qwiicBusPullUps == 1)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K; // Use 1K5 pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  }
  else if (qwiicBusPullUps == 6)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K; // Use 6K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K;
  }
  else if (qwiicBusPullUps == 12)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K; // Use 12K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K;
  }
  else
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K; // Use 24K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K;
  }

  pin_config(PinName(PIN_QWIIC_SCL), sclPinCfg);
  pin_config(PinName(PIN_QWIIC_SDA), sdaPinCfg);
}
