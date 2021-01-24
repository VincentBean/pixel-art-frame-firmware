#include <list>
#include <iterator>
#include <SD.h>
#include "Global.h"
#include "MatrixGif.hpp"

bool gifsLoaded = false;
unsigned long gifStart = 0;
int minPlaytime = 4000;
char *gifDir = "/gifs";
int currentGifIndex = 0;

std::vector<String> gifs;

void loadGifs()
{
    gifs.clear();
    File root = SD.open(gifDir);

    if (!root)
    {
        Serial.println("Failed to open gif dir");
        return;
    }

    File currentGif = root.openNextFile();

    while (currentGif)
    {
        if (currentGif.isDirectory())
        {
            currentGif.close();
            continue;
        }

        gifs.push_back(String((char *)currentGif.name()));

        currentGif.close();
        currentGif = root.openNextFile();
    }

    gifsLoaded = true;
}

char* getCurrentGif()
{
    return (char*)gifs.at(currentGifIndex).c_str();
}

void nextGif()
{
    gifStart = millis();

    currentGifIndex++;

    if (currentGifIndex > (gifs.size()-1))
    {
        currentGifIndex = 0;
    }

    interruptGif = true;
}

void prevGif()
{
    gifStart = millis();

    if (currentGifIndex > 0)
    {
        currentGifIndex--;
    }
    else
    {
        currentGifIndex = gifs.size() - 1;
    }
    interruptGif = true;    
}

void playGif()
{
    if (!gifsLoaded) {
        loadGifs();
        gifsLoaded = true;
    }

    if (millis() - gifStart > minPlaytime)
    {
        nextGif();
    }

    ShowGIF(getCurrentGif());
}
