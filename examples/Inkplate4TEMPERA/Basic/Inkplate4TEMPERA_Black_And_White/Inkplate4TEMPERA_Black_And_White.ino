/*
    Inkplate4TEMPERA_Black_And_White_No_Logo_File.ino example for Soldered Inkplate 4 TEMPERA

    This example shows how to draw simple black and white graphics using
    Adafruit GFX-compatible Inkplate functions, without requiring an external
    bitmap header like logo.h.
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "Inkplate.h"

Inkplate display(INKPLATE_1BIT);

#define DELAY_MS 5000

void displayCurrentAction(String text);
void drawGeneratedLogo();

void setup()
{
    display.begin();
    display.clearDisplay();
    display.display();

    display.setCursor(40, 300);
    display.setTextSize(3);
    display.print("Welcome to Inkplate 4TEMPERA!");
    display.display();
    delay(5000);
}

void loop()
{
    // Draw one pixel
    display.clearDisplay();
    displayCurrentAction("Drawing a pixel");
    display.drawPixel(100, 50, BLACK);
    display.display();
    delay(DELAY_MS);

    // Draw random pixels
    display.clearDisplay();
    for (int i = 0; i < 600; i++)
    {
        display.drawPixel(random(0, 599), random(0, 599), BLACK);
    }
    displayCurrentAction("Drawing 600 random pixels");
    display.display();
    delay(DELAY_MS);

    // Draw diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 599, 599, BLACK);
    display.drawLine(599, 0, 0, 599, BLACK);
    displayCurrentAction("Drawing two diagonal lines");
    display.display();
    delay(DELAY_MS);

    // Random lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawLine(random(0, 599), random(0, 599), random(0, 599), random(0, 599), BLACK);
    }
    displayCurrentAction("Drawing 50 random lines");
    display.display();
    delay(DELAY_MS);

    // Random thick lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawThickLine(random(0, 599), random(0, 599), random(0, 599), random(0, 599), BLACK,
                              (float)random(1, 20));
    }
    displayCurrentAction("Drawing 50 random thick lines");
    display.display();
    delay(DELAY_MS);

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(100, 300, 400, BLACK);
    displayCurrentAction("Drawing one horizontal line");
    display.display();
    delay(DELAY_MS);

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(300, 100, 400, BLACK);
    displayCurrentAction("Drawing one vertical line");
    display.display();
    delay(DELAY_MS);

    // Grid
    display.clearDisplay();
    for (int i = 0; i < 600; i += 8)
    {
        display.drawFastVLine(i, 0, 600, BLACK);
    }
    for (int i = 0; i < 600; i += 4)
    {
        display.drawFastHLine(0, i, 600, BLACK);
    }
    displayCurrentAction("Drawing a grid using horizontal and vertical lines");
    display.display();
    delay(DELAY_MS);

    // Rectangle
    display.clearDisplay();
    display.drawRect(150, 200, 300, 200, BLACK);
    displayCurrentAction("Drawing rectangle");
    display.display();
    delay(DELAY_MS);

    // Many rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawRect(random(0, 599), random(0, 599), 100, 150, BLACK);
    }
    displayCurrentAction("Drawing many rectangles");
    display.display();
    delay(DELAY_MS);

    // Filled rectangle
    display.clearDisplay();
    display.fillRect(150, 200, 300, 200, BLACK);
    displayCurrentAction("Drawing black rectangle");
    display.display();
    delay(DELAY_MS);

    // Many filled rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.fillRect(random(0, 599), random(0, 599), 30, 30, BLACK);
    }
    displayCurrentAction("Drawing many filled rectangles randomly");
    display.display();
    delay(DELAY_MS);

    // Circle
    display.clearDisplay();
    display.drawCircle(300, 300, 75, BLACK);
    displayCurrentAction("Drawing a circle");
    display.display();
    delay(DELAY_MS);

    // Many circles
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
    {
        display.drawCircle(random(0, 599), random(0, 599), 25, BLACK);
    }
    displayCurrentAction("Drawing many circles randomly");
    display.display();
    delay(DELAY_MS);

    // Filled circle
    display.clearDisplay();
    display.fillCircle(300, 300, 75, BLACK);
    displayCurrentAction("Drawing black-filled circle");
    display.display();
    delay(DELAY_MS);

    // Many filled circles
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
    {
        display.fillCircle(random(0, 599), random(0, 599), 15, BLACK);
    }
    displayCurrentAction("Drawing many filled circles randomly");
    display.display();
    delay(DELAY_MS);

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(150, 200, 300, 200, 10, BLACK);
    displayCurrentAction("Drawing rectangle with rounded edges");
    display.display();
    delay(DELAY_MS);

    // Many rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.drawRoundRect(random(0, 599), random(0, 599), 100, 150, 5, BLACK);
    }
    displayCurrentAction("Drawing many rounded edges rectangles");
    display.display();
    delay(DELAY_MS);

    // Filled rounded rectangle
    display.clearDisplay();
    display.fillRoundRect(150, 200, 300, 200, 20, BLACK);
    displayCurrentAction("Drawing filled rectangle with rounded edges");
    display.display();
    delay(DELAY_MS);

    // Many filled rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
    {
        display.fillRoundRect(random(0, 599), random(0, 599), 30, 30, 3, BLACK);
    }
    displayCurrentAction("Random rounded edge filled rectangles");
    display.display();
    delay(DELAY_MS);

    // Triangle
    display.clearDisplay();
    displayCurrentAction("Drawing triangle");
    display.drawTriangle(150, 400, 450, 400, 300, 100, BLACK);
    display.display();
    delay(DELAY_MS);

    // Filled triangle inside
    display.fillTriangle(200, 350, 400, 350, 300, 150, BLACK);
    displayCurrentAction("Drawing filled triangle inside");
    display.display();
    delay(DELAY_MS);

    // Generated "logo" banner instead of external bitmap
    display.clearDisplay();
    drawGeneratedLogo();
    displayCurrentAction("Drawing generated logo");
    display.display();
    delay(DELAY_MS);

    // Text in different sizes
    display.clearDisplay();
    for (int i = 0; i < 5; i++)
    {
        display.setTextSize(i + 1);
        display.setCursor(100, (i * i * 8));
        display.print("Inkplate 4TEMPERA");
    }
    displayCurrentAction("Text in different sizes and shadings");
    display.display();
    delay(DELAY_MS);

    // Inverted text
    display.setTextColor(WHITE, BLACK);
    for (int i = 0; i < 5; i++)
    {
        display.setTextSize(i + 1);
        display.setCursor(100, 260 + (i * i * 8));
        display.print("Inkplate 4TEMPERA!");
    }
    display.display();
    delay(DELAY_MS);

    display.setTextColor(BLACK, WHITE);

    // Ellipse
    display.clearDisplay();
    display.drawElipse(400, 200, 300, 300, BLACK);
    displayCurrentAction("Drawing an ellipse");
    display.display();
    delay(DELAY_MS);

    // Filled ellipse
    display.clearDisplay();
    display.fillElipse(400, 200, 300, 300, BLACK);
    displayCurrentAction("Drawing a filled ellipse");
    display.display();
    delay(DELAY_MS);

    // Random polygon points
    int xt[10];
    int yt[10];
    int n = 10;

    for (int i = 0; i < n; ++i)
    {
        xt[i] = random(50, 550);
        yt[i] = random(50, 550);
    }

    int k;
    for (int i = 0; i < n - 1; ++i)
        for (int j = i + 1; j < n; ++j)
            if (atan2(yt[j] - 300, xt[j] - 400) < atan2(yt[i] - 300, xt[i] - 400))
            {
                k = xt[i], xt[i] = xt[j], xt[j] = k;
                k = yt[i], yt[i] = yt[j], yt[j] = k;
            }

    // Polygon
    display.clearDisplay();
    display.drawPolygon(xt, yt, n, BLACK);
    displayCurrentAction("Drawing a polygon");
    display.display();
    delay(DELAY_MS);

    // Filled polygon
    display.clearDisplay();
    display.fillPolygon(xt, yt, n, BLACK);
    displayCurrentAction("Drawing a filled polygon");
    display.display();
    delay(DELAY_MS);

    // Rotate text forever
    int r = 0;
    display.setTextSize(5);
    display.setTextColor(WHITE, BLACK);

    while (true)
    {
        display.setCursor(120, 250);
        display.clearDisplay();
        display.setRotation(r);
        display.print("Inkplate 4TEMPERA");
        display.display();
        r++;
        delay(DELAY_MS);
    }
}

void drawGeneratedLogo()
{
    // Outer banner
    display.drawRoundRect(40, 220, 520, 160, 20, BLACK);

    // Decorative horizontal lines
    display.drawFastHLine(80, 245, 440, BLACK);
    display.drawFastHLine(80, 355, 440, BLACK);

    // Main title
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(4);
    display.setCursor(120, 265);
    display.print("Inkplate");

    // Subtitle
    display.setTextSize(2);
    display.setCursor(210, 320);
    display.print("4 TEMPERA");

    // Small filled circles for accent
    display.fillCircle(95, 300, 10, BLACK);
    display.fillCircle(505, 300, 10, BLACK);

    // Reset text color
    display.setTextColor(BLACK, WHITE);
}

void displayCurrentAction(String text)
{
    display.setTextSize(2);
    display.setCursor(20, 560);
    display.print(text);
}