//Md Babla Islam
#include <SoftwareSerial.h>

SoftwareSerial sim(3, 2); // Arduino TX = pin 3, RX = pin 2

int motionPin = 10;
bool isCalling = false;
unsigned long lastCallTime = 0;
const unsigned long callCooldown = 15000; // 15 seconds
unsigned long lastStatusPrintTime = 0;

int _timeout;
String _buffer;
String number = "+8801*********";//Enter phone number where you want to call/sms

void setup()
{
    delay(5000);
    Serial.begin(9600);
    _buffer.reserve(50);
    Serial.println("System Started...");
    sim.begin(9600);

    pinMode(motionPin, INPUT);
    pinMode(13, OUTPUT); // Built-in LED as motion indicator

    delay(1000);
    Serial.println("Type 'sms' to send SMS, 'receive' to receive a call/SMS, and 'call' to make a call");
}

void loop()
{
    // Handle user command
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "sms")
        {
            SendMessage();
        }
        else if (command == "receive")
        {
            ReceiveCallOrSMS();
        }
        else if (command == "call")
        {
            callNumber();
        }
        else
        {
            Serial.println("Unknown command. Type 'sms', 'receive', or 'call'");
        }
    }

    // Forward SIM800L response to Serial
    if (sim.available())
    {
        Serial.write(sim.read());
    }

    bool motion = digitalRead(motionPin);

    if (motion)
    {
        digitalWrite(13, HIGH); // Motion LED ON
        if (millis() - lastCallTime > callCooldown)
        {
            Serial.println("Motion detected! Making a call...");
            callNumber();
            lastCallTime = millis();
        }
    }
    else
    {
        digitalWrite(13, LOW); // Motion LED OFF
    }

    // Print status every 2 seconds
    if (millis() - lastStatusPrintTime >= 2000)
    {
        lastStatusPrintTime = millis();

        if (motion)
        {
            Serial.println("Motion detected. LED ON.");
        }
        else
        {
            unsigned long timeSinceLastCall = millis() - lastCallTime;
            if (timeSinceLastCall < callCooldown)
            {
                unsigned long remaining = (callCooldown - timeSinceLastCall) / 1000;
                Serial.print("Cooldown time remaining: ");
                Serial.print(remaining);
                Serial.println(" seconds");
            }
            else
            {
                Serial.println("System ready. Waiting for motion...");
            }
        }
    }
}

void SendMessage()
{
    Serial.println("Sending Message...");
    sim.println("AT+CMGF=1");
    delay(1000);
    sim.println("AT+CMGS=\"" + number + "\"");
    delay(1000);
    sim.println("Hi I am Babla");
    delay(100);
    sim.write(26); // CTRL+Z
    delay(2000);
    _buffer = _readSerial();
    Serial.println("Message sent:");
    Serial.println(_buffer);
}

void ReceiveCallOrSMS()
{
    Serial.println("Setting module to receive call/SMS...");
    sim.println("AT+CNMI=2,2,0,0,0");
    delay(1000);
    sim.println("ATA");
    delay(2000);
    _buffer = _readSerial();
    Serial.println("Receive output:");
    Serial.println(_buffer);
}

void callNumber()
{
    Serial.println("Calling...");
    sim.print("ATD");
    sim.print(number);
    sim.println(";");
    delay(1000);
    _buffer = _readSerial();
    Serial.println("Call initiated:");
    Serial.println(_buffer);
}

String _readSerial()
{
    _timeout = 0;
    while (!sim.available() && _timeout < 12000)
    {
        delay(13);
        _timeout++;
    }
    String response = "";
    if (sim.available())
    {
        response = sim.readString();
    }
    return response;
}
