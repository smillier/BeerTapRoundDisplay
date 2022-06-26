#include <config.h>

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
}