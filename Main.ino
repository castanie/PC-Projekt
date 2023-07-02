#include <EEPROM.h>

/////////////
// Global: //
/////////////

int IONIAN[7] = {65, 73, 82, 87, 98, 110, 123};  //Major scale.
int AEOLIAN[7] = {65, 73, 78, 87, 98, 104, 117}; //Minor scale.

int *scales[] = {IONIAN, AEOLIAN}; //Array holding available musical scales.
int scale;                         //Currently selected musical scale.

int notes[16];        //Array holding the playback sequence.
int notePosition = 0; //Current playback position.

bool buttons[7]; //Array holding the button states during current cycle.
bool writeFlag;  //Flag used to control overwriting of notes.
bool waitFlag;   //Flag used to await button combinations after an initial button press.
int lastButton;  //Variable that holds last pressed button ID.

unsigned long noteLength = 300;     //Playback length of every note in sequence in milliseconds.
unsigned long lastSound = millis(); //Timestamp of last note playback.
unsigned long lastEvent = millis(); //Timestamp of last button press.

int getOption() //Function to select an option in menu mode.
{
    while (true)
    {
        for (int i = 0; i < 7; ++i)
        {
            if (digitalRead(4 + i))
            {
                lastEvent = millis();
                return i;
            }
        }
    }
}

////////////
// Setup: //
////////////

void setup()
{
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(10, INPUT);

    pinMode(2, OUTPUT);
}

///////////
// Loop: //
///////////

void loop()
{
    //Play sequence:

    if (millis() > lastSound + noteLength)
    {
        tone(2, scales[scale][notes[notePosition]], 200);
        lastSound = millis();

        notePosition =
            notePosition < 15 ? ++notePosition : 0;
    }

    //Update button states:

    for (int i = 0; i < 7; ++i)
    {
        if (digitalRead(4 + i))
        {
            tone(2, scales[scale][i], 200);

            if (buttons[i])
            {
                continue;
            }

            buttons[i] = true;
            lastEvent = millis();
            lastButton = i;

            if (!waitFlag)
            {
                waitFlag = true;
                writeFlag = true;
            }
            else
            {
                waitFlag = false;
                writeFlag = true;
                break;
            }
        }
        else
        {
            buttons[i] = false;
        }
    }

    //Reset waitFlag:

    if (millis() > lastEvent + 100)
    {
        waitFlag = false;
    }

    //Update sequence:

    if (!waitFlag)
    {
        //Menu combinations:
        if (buttons[0] && buttons[6]) //PAUSE SEQUENCE
        {
            while (!digitalRead(7))
            {
                /* nothing*/
            }
            delay(500);
        }
        else if (buttons[0] && buttons[5]) //SAVE SEQUENCE
        {
            delay(500);
            lastButton = 16 * getOption();
            for (int i = 0; i < 16; ++i)
            {
                EEPROM.write(lastButton + i, notes[i]);
            }
            delay(500);
        }
        else if (buttons[0] && buttons[4]) //LOAD SEQUENCE
        {
            delay(500);
            lastButton = 16 * getOption();
            for (int i = 0; i < 16; ++i)
            {
                notes[i] = EEPROM.read(lastButton + i);
            }
            delay(500);
        }
        else if (buttons[0] && buttons[3]) //CHOOSE SCALE
        {
            delay(500);
            scale = getOption() % 2;
            delay(500);
        }
        else if (writeFlag) //UPDATE SEQUENCE
        {
            if (lastEvent < lastSound) //Fires in case a button was pressed during last beat but not registered yet.
            {
                if (notePosition == 0)
                {
                    notes[15] = lastButton;
                }
                else
                {
                    notes[notePosition - 1] = lastButton;
                }
            }
            else
            {
                notes[notePosition] = lastButton;
            }
        }

        writeFlag = false;
    }
}