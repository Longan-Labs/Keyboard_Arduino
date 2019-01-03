// demo for the touch sensor

#include <Wire.h> 
#include <Streaming.h>
#include <pwm_lvc.h>
#include <SoftPWM.h>
#include <TTP229.h>

#include "pitches.h"

#define DELAY 100

const int SCL_PIN = 3;  // The pin number of the clock pin.
const int SDO_PIN = 2;  // The pin number of the data pin.

TTP229 ttp229(SCL_PIN, SDO_PIN); // TTP229(sclPin, sdoPin)

//int __Gfreq[10] = {1,2,3,4,5,6,7,8,9,0};
const int __Gfreq[10] = {NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5};

const int leds[8] = {11, 4, 13, 6, 7, 8, 10, 5};
const int pin_2_led[10] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 7};

int flg         = 0;
int flg2        = 0;
int touch_num   = 0;

#define MODE_MUSIC      0x0
#define MODE_CIRCLE     0x1

int mode = MODE_MUSIC;

void setPwm(int touch_n, int pwm)
{
    int pin_ = pin_2_led[touch_n];
    SoftPWMSet(leds[pin_], pwm);
}


void setFreq(int freq, int touch_n)
{
    if(freq < 0 || freq > 5000)return;
    
    PWM.setPwm(9, 80, freq);

    setPwm(touch_n, 255);           // led
    
    //ledOn(touch_n);
    for(int i=99; i>=0; i--)
    {
        PWM.setPwm(10, i, freq);
        delay(3);
        
        uint8_t key = ttp229.GetKey16(); // Blocking
        if(key)flg = 1;
        
        if(flg)
        {
            flg = 0;
            touch_num = ttp229.GetKey16()-1;
            
            if(touch_num>=0 && touch_num < 10)
            {
                flg2 = 1;
                PWM.setPwm(9, 0, freq);
                PWM.setPwm(10, 0, freq);
                
                //ledOff(touch_n);
                setPwm(touch_n, 0);
                return;
            }
        }
    }
    
    //ledOff(touch_n);
    setPwm(touch_n, 0);
    PWM.setPwm(9, 0, freq);
}


void setup()
{
    Serial.begin(115200);

    PWM.init();  // init
    PWM.setPwm(9, 50, 0);        // pin: 9,  duty: 20%, freq: 5kHz
    
    // softpwm
    SoftPWMBegin();

    for (int i = 0; i < 8; i++)
    SoftPWMSet(leds[i], 0);

    SoftPWMSetFadeTime(ALL, 10, 300);

}


void loop()
{

    uint8_t key = ttp229.GetKey16(); // Blocking
    if(key)
    {
        flg = 1;
        cout << "TOUCH: "<< key-1<<endl;
    }
    
    if(MODE_CIRCLE == mode)
    {
        cout << "circle" << endl;
        circle();
    }
    else if(MODE_MUSIC == mode)
    {
        if(flg)
        {
            flg = 0;
            int _num = ttp229.GetKey16()-1;
            
            if(_num == 10)
            {
                change_mode(MODE_CIRCLE);
                flg2 = 0;
            }
            
            if(_num >= 0 && _num < 10)
            setFreq(__Gfreq[_num], _num);
        }
        
        if(flg2)
        {
            flg2 = 0;
            
            if(touch_num == 10)
            {
                change_mode(MODE_CIRCLE);
                flg2 = 0;
            }
            
            if(touch_num >= 0 && touch_num < 10)
            setFreq(__Gfreq[touch_num], touch_num);
        }
        delay(1);         // check per 100ms
    }
}



void circle()
{
    for(int i=0; i<7; i++)
    {
        SoftPWMSet(leds[i+1], 255);
        SoftPWMSet(leds[i], 0);

        if(delay_check(DELAY))
        {
            for(int i=0; i<8; i++)
            {
                SoftPWMSet(leds[i], 0);
            }
            change_mode(MODE_MUSIC);
            return;
        }

    }
    
    SoftPWMSet(leds[7], 0);
    SoftPWMSet(leds[0], 255);
    
    if(delay_check(DELAY))
    {
        for(int i=0; i<8; i++)
        {
            SoftPWMSet(leds[i], 0);
        }
        
        change_mode(MODE_MUSIC);
        return;
    }

}

// if return 0: delay ok
// if return 1: delay with flg
int delay_check(unsigned long ms)
{
    unsigned long timer_s = millis();
    
    while(1)
    {

        uint8_t key = ttp229.GetKey16(); // Blocking
        if(key)flg = 1;
    
        if(flg)
        {
            flg = 0;
            if(10 == (ttp229.GetKey16()-1))
            {
                return 1;
            }
        }
        
        if(millis()-timer_s > ms)return 1;
    }
    
    return 0;
}

// change mode
void change_mode(int __mode)
{
    cout << "CHANGE MODE: " << __mode << endl;
    mode = __mode;
    for(int i=0; i<8; i++)
    {
        SoftPWMSet(leds[i], 255);
    }
    
    setFreq(__Gfreq[9], 9);

    for(int i=0; i<8; i++)
    {
        SoftPWMSet(leds[i], 0);
    }
    delay(300);
}