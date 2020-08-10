
//Display the options
//If user doesn't respond within a few seconds, return to main loop
void menuMain()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Main Menu"));

    Serial.println(F("1) Configure Terminal Output"));
    //10 to 1 Hz? Global logging. Overrides the IMU, analog, etc.?
    //User may want to log Analog really fast and IMU or humidity less fast.

    Serial.println(F("2) Configure Time Stamp"));

    Serial.println(F("3) Configure IMU Logging"));
    //Set record freq

    Serial.println(F("4) Configure Serial Logging"));

    Serial.println(F("5) Configure Analog Logging"));

    Serial.println(F("6) Detect / Configure Attached Devices"));

    Serial.println(F("7) Configure Power Options"));

    Serial.println(F("h) Print Sensor Helper Text (and return to logging)"));

    Serial.println(F("r) Reset all settings to default"));

    Serial.println(F("q) Quit: Close log files and power down"));

    //Serial.println(F("d) Debug Menu"));

    Serial.println(F("x) Return to logging"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      menuLogRate();
    else if (incoming == '2')
      menuTimeStamp();
    else if (incoming == '3')
      menuIMU();
    else if (incoming == '4')
      menuSerialLogging();
    else if (incoming == '5')
      menuAnalogLogging();
    else if (incoming == '6')
      menuAttachedDevices();
    else if (incoming == '7')
      menuPower();
    else if (incoming == 'h')
    {
      printHelperText(true); //printHelperText to terminal only
      break; //return to logging
    }
    else if (incoming == 'd')
      menuDebug();
    else if (incoming == 'r')
    {
      Serial.println(F("\n\rResetting to factory defaults. Press 'y' to confirm:"));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        EEPROM.erase();
        if (sd.exists("OLA_settings.txt"))
          sd.remove("OLA_settings.txt");
        if (sd.exists("OLA_deviceSettings.txt"))
          sd.remove("OLA_deviceSettings.txt");

        Serial.print(F("Settings erased. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        Serial.println(F("bps..."));
        while (1);
      }
      else
        Serial.println(F("Reset aborted"));
    }
    else if (incoming == 'q')
    {
      Serial.println(F("\n\rQuit? Press 'y' to confirm:"));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        //Save files before going to sleep
        if (online.dataLogging == true)
        {
          sensorDataFile.sync();
          sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
        }
        if (online.serialLogging == true)
        {
          serialDataFile.sync();
          serialDataFile.close();
        }
        Serial.print(F("Log files are closed. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        Serial.println(F("bps..."));
        delay(sdPowerDownDelay); // Give the SD card time to shut down
        powerDown();
      }
      else
        Serial.println(F("Quit aborted"));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  recordSystemSettings(); //Once all menus have exited, record the new settings to EEPROM and config file

  recordDeviceSettingsToFile(); //Record the current devices settings to device config file

  configureQwiicDevices(); //Reconfigure the qwiic devices in case any settings have changed

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  //Reset measurements
  measurementCount = 0;
  totalCharactersPrinted = 0;
  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    measurementStartTime = rtcMillis();
  else
    measurementStartTime = millis();

  //Edge case: after 10Hz reading, user sets the log rate above 2s mark. We never go to sleep because 
  //takeReading is not true. And since we don't wake up, takeReading never gets set to true.
  //So we force it here.
  takeReading = true; 
}
