#include <config.h>
#include <esp_system.h>

Preferences preference;

String getBeerName()
{
    preference.begin("TapDisplay",false);
    String value = preference.getString("beer_name","Undefined");
    preference.end();
    return value;
}

void setBeerName(String value)
{
    preference.begin("TapDisplay",false);
    preference.putString("beer_name",value);
    preference.end();
    
}

float getBeerSg()
{
    preference.begin("TapDisplay",false);
    float value = preference.getFloat("beer_Sg",1.0);
    preference.end();
    return value;
}

void setBeerSg(float value)
{
    preference.begin("TapDisplay",false);
    preference.putFloat("beer_Sg",value);
    preference.end();
    
}

float getBeerCapacity()
{
    preference.begin("TapDisplay",false);
    float value = preference.getFloat("beer_cap",19.0);
    preference.end();
    return value;
}

void setBeerCapacity(float value)
{
    preference.begin("TapDisplay",false);
    preference.putFloat("beer_cap",value);
    preference.end();
}

float getBeerABV()
{
    preference.begin("TapDisplay",false);
    float value = preference.getFloat("beer_ABV",0.0);
    preference.end();
    return value;
}

void setBeerABV(float value)
{
    preference.begin("TapDisplay",false);
    preference.putFloat("beer_ABV",value);
    preference.end();    
}

int getBeerIBU()
{
    preference.begin("TapDisplay",false);
    int value = preference.getInt("beer_IBU",0);
    preference.end();
    return value;
}
void setBeerIBU(int value)
{
    preference.begin("TapDisplay",false);
    preference.putInt("beer_IBU",value);
    preference.end();    
}


int getBeerEBC()
{
    preference.begin("TapDisplay",false);
    int value = preference.getInt("beer_EBC",0);
    preference.end();
    return value;
}

void setBeerEBC(int value)
{
    preference.begin("TapDisplay",false);
    preference.putInt("beer_EBC",value);
    preference.end();    
}

String getApiPassword()
{
    preference.begin("TapDisplay",false);
    String value = preference.getString("api_pass","");
    if (value.length() == 0)
    {
        // Never configured (first boot after flashing this version): generate
        // a random per-device password using the ESP32's hardware RNG rather
        // than shipping one fixed default across every device running this
        // firmware. Excludes visually-ambiguous characters (I/l/1, O/0) since
        // this gets read off a Serial console and typed by hand.
        const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz23456789";
        char buf[13];
        for (int i = 0; i < 12; i++)
            buf[i] = charset[esp_random() % (sizeof(charset) - 1)];
        buf[12] = '\0';
        value = String(buf);
        preference.putString("api_pass",value);
    }
    preference.end();
    return value;
}

void setApiPassword(String value)
{
    preference.begin("TapDisplay",false);
    preference.putString("api_pass",value);
    preference.end();
}

String getFileForEBC()
{
    int value = getBeerEBC();   
    if (value >= 0 && value < 10)
        return "NSRM4.bmp";
    if (value >= 10 && value < 20)
        return "NSRM5.bmp";
    if (value >= 20 && value < 30)
        return "NSRM7.bmp";
     if (value >= 30 && value < 40)
        return "NSRM12.bmp";
     if (value >= 40)
        return "NSRM36.bmp";
    return "NSRM4.bmp";
}