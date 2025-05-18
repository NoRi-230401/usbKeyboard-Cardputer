/**
 * @file usbKeyboard.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer USB Keyboard test
 * @version 0.1
 * @date 2023-10-13
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 *
 * modified by @shikarunochi
 * modified by @NoRi 2025-05-17
 */

#include <Arduino.h>
#include <SD.h>
#include <M5Cardputer.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <M5StackUpdater.h>

void m5stack_begin();
void SDU_lobby();
bool SD_begin();
void disp_start();
bool SD_ENABLE;
String const arrow_key[] = {"[ left ]", "[ down ]", "[ up ]", "[ right ]"};
int arrow_key_index = -1;

SPIClass SPI2;
USBHIDKeyboard Keyboard;

void setup()
{
    m5stack_begin();
    if (SD_ENABLE)
        SDU_lobby();

    disp_start();
    Keyboard.begin();
    USB.begin();
}

void loop()
{
    M5Cardputer.update();
    // max press 3 button at the same time
    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            KeyReport report = {0};
            report.modifiers = status.modifiers;
            uint8_t index = 0;
            for (auto i : status.hid_keys)
            {
                auto fixedI = i;
                if (status.fn)
                {
                    switch (i)
                    {
                    // https://wiki.onakasuita.org/pukiwiki/?HID%2F%E3%82%AD%E3%83%BC%E3%82%B3%E3%83%BC%E3%83%89
                    case 0x36:
                        fixedI = 0x50;
                        arrow_key_index = 0;
                        break; //, LeftArrow
                    case 0x37:
                        fixedI = 0x51;
                        arrow_key_index = 1;
                        break; //. DownArrow
                    case 0x33:
                        fixedI = 0x52;
                        arrow_key_index = 2;
                        break; //; UpArrow
                    case 0x38:
                        fixedI = 0x4F;
                        arrow_key_index = 3;
                        break; /// RightArrow
                    }
                }
                report.keys[index] = fixedI;
                index++;
                if (index > 5)
                {
                    index = 5;
                }
            }
            Keyboard.sendReport(&report);

            // only text for display
            String keyStr = "";
            for (auto i : status.word)
            {
                if (keyStr != "")
                {
                    keyStr = keyStr + " + " + i;
                }
                else
                {
                    keyStr += i;
                }
            }

            if (keyStr.length() > 0)
            {
                // clear half of lower display
                M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() / 2, M5Cardputer.Display.width(), M5Cardputer.Display.height(), TFT_BLACK);

                if (arrow_key_index >= 0 && arrow_key_index <= 3)
                {
                    M5Cardputer.Display.drawString(arrow_key[arrow_key_index],
                                                   M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() * 2 / 4);
                    arrow_key_index = -1;
                }
                else
                {
                    M5Cardputer.Display.drawString(keyStr,
                                                   M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() * 2 / 4);
                }
            }
        }
        else
        {
            Keyboard.releaseAll();
        }
    }
}

void m5stack_begin()
{
    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;

    M5Cardputer.begin(cfg, true);

    // initial display setup
    M5Cardputer.Display.setBrightness(70);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Cardputer.Display.setFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextWrap(false);
    M5Cardputer.Display.setCursor(0, 0);

    SPI2.begin(
        M5.getPin(m5::pin_name_t::sd_spi_sclk),
        M5.getPin(m5::pin_name_t::sd_spi_miso),
        M5.getPin(m5::pin_name_t::sd_spi_mosi),
        M5.getPin(m5::pin_name_t::sd_spi_ss));

    SD_ENABLE = SD_begin();
}

// ------------------------------------------
// SDU_lobby : SD_uploader lobby
// ------------------------------------------
// load "/menu.bin" on SD
//    if 'a' pressed at booting
// ------------------------------------------
void SDU_lobby()
{
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed('a'))
    {
        updateFromFS(SD, "/menu.bin");
        ESP.restart();

        while (true)
            ;
    }
}

bool SD_begin()
{
    int i = 0;
    while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2) && i < 10)
    {
        delay(500);
        i++;
    }

    if (i >= 10)
    {
        Serial.println("ERR: SD begin erro...");
        return false;
    }

    return true;
}

void disp_start()
{
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextDatum(top_center);
    M5Cardputer.Display.setTextFont(&fonts::Orbitron_Light_24);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString(
        "USB Keyboard", M5Cardputer.Display.width() / 2, 10);
}
