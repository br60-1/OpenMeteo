German:
Auf der Suche nach einem ansprechenden Projekt zur Anzeige einer Wettervorhersage wurde ich bei https://github.com/Bodmer/OpenWeather fündig.
Sehr schön finde ich die Umsetzung der Texte, Bilder/Grafiken. Dabei entdeckte ich die sehr gute Bibliothek https://github.com/Bodmer/TFT_eSPI für die farbige Anzeige der Wetterdaten.

Vermisst wurde eine Vorhersage für die nächsten Stunden, störend war die Notwendigkeit einer Registrierung und die Hinterlegung von Kreditkartendaten bei Openweather.
Eine Weitergabe des Projektes an Freunde und Bekannte mit persönlichen Zugangsdaten war so eher nicht möglich.

Auf der Suche nach Alternativen wird man bei Open-Meteo.com fündig. Der Zugriff benötigt keine Anmeldung, es werden nur die angeforderten Daten (das minimiert das parsen) 
geliefert und bis zu 10.000 Abfragen pro Tag sind für den privaten Gebrauch kostenfrei.

Eine ansprechende Verpackung findet man bei "ESP8266 Weather Widget" von OpenGreenEnergy (https://www.thingiverse.com/thing:4928974). Da passen genau alle benötigten Module hinein.
Noch zwei Bohrungen für ein Stück Draht (Büroklammer) an der oberen Seite zum Zwecke der Umschaltung der Bildschirme.

Die Bibliothek von Bodmer (https://github.com/Bodmer/OpenWeather) wurde für die Verwendung von https://api.open-meteo.com geändert. Der Umfang beschränkt sich nun jedoch
auf die tatsächlich für das Projekt benötigten Daten. Sollen weitere meteorologischen Daten abgerufen werden, muss die Bibliothek angepasst werden.

Alle Bildschirmanzeigen und (optionalen) Ausgaben auf der seriellen Schnittstelle wurden in deutscher Sprache verfasst, jedoch kann dies einfach geändert werden.
Für die Anzeige der Wettervorhersage für die nächsten Stunden wurde ein zweiter Bildschirm geschaffen. Die Umschaltung wird durch eine Fingerberührung an einem analogen Pin ausgelöst.
Die Wiederherstellung der Daten auf den zwei Bildschirmen erfordert, dass die Daten bis zum nächsten Abruf in der Struktur erhalten werden.

Das Beispiel OpenMeteoTFT arbeitet mit ESP32 und ESP8266, von mir wurde ein LOLIN WEMOS D1 mini (clon) verwendet, da dieser in das oben beschriebe Gehäuse passt. Weitere Controller sind möglich, aber hier nicht getestet.
Als TFT-Display kam ein ILI9341 in 2.8" (320x240 Hochformat) zur Anwendung. Eine Touch-Funktion ist dabei nicht nötig, man kann diese aber möglicherweise auch nutzen, um die Bildschirmanzeigen zu wechseln.

Das Beispiel verwendet zur Anzeige Bodmers Bibliothek https://github.com/Bodmer/TFT_eSPI und die von mir etwas erweiterten Icons von Bodmer aus https://github.com/Bodmer/OpenWeather.    
Es verwendet das LittleFS Filesystem, welches derzeit bereits im Arduino-Kern enthalten ist. Für das Hochladen der Schriften, Bilder und Grafiken müssen diese in einem Unterordner "data" des Sketchordners vorliegen.
Es ist auf die Verwendung eines aktuellen (01/2026) plugin der Arduino-IDE für das Hochladen zu achten.
Mit geöffneter Arduino IDE kann man mit der Tastenkombination [Strg] + [Shift] + [P] das Command-Menü aufrufen.
Dort sollte ein Eintrag "Upload LittleFS to Pico/ESP8266/ESP32" zu finden sein. Die hochzuladenden Daten müssen in [Sketchordner]\data vorhanden sein.

Die Verbindungen zwischen ESP8266/ESP32/RP2040 zum TFT sind in der TFT_eSPI Bibliothek beschrieben (File: User_Setup_Select.h).
Als JSON parser wird der von Bodmer https://github.com/Bodmer/JSON_Decoder verwendet (nicht den aus der Arduino Bibliothek Sammlung verwenden).

English:
While searching for an appealing project to display a weather forecast, I came across https://github.com/Bodmer/OpenWeather.
I really like the way the text and images/graphics have been implemented. I also discovered the excellent library https://github.com/Bodmer/TFT_eSPI for displaying weather data in color.
However, a forecast for the next few hours was missing, and the requirement to register and provide credit card information with OpenWeather was inconvenient. Sharing the project with friends and acquaintances using
their personal login credentials was therefore not really feasible.

If you're looking for alternatives, you'll find Open-Meteo.com. Access doesn't require registration, only the requested data is delivered (which minimizes parsing), and up to 10,000 queries per day are free for private use.
An attractive package can be found at "ESP8266 Weather Widget" by OpenGreenEnergy (https://www.thingiverse.com/thing:4928974). It perfectly accommodates all the necessary modules.
Two holes are needed at the top for a piece of wire (paperclip) to switch between screens.

The Bodmer library (https://github.com/Bodmer/OpenWeather) has been modified for use with https://api.open-meteo.com. However, its scope is now limited to the data actually required for the project. If additional
meteorological data is to be retrieved, the library must be modified.

All screen displays and (optional) serial interface outputs are written in German, but this can easily be changed.
A second screen has been created to display the weather forecast for the next few hours. Switching between screens is triggered by touching an analog pin.
Restoring the data on the two screens requires that the data be retained in the structure until the next request.

The OpenMeteoTFT example uses ESP32 and ESP8266. I used a LOLIN WEMOS D1 mini (clone) because it fits in the enclosure described above. Other controllers are possible, but not tested here.
A 2.8" ILI9341 TFT display (320x240 portrait format) was used. A touch function isn't necessary, but it could potentially be used to switch between screen displays.
This example uses Bodmer's library (https://github.com/Bodmer/TFT_eSPI) and Bodmer's slightly modified icons (https://github.com/Bodmer/OpenWeather).
It uses the LittleFS filesystem, which is already included in the Arduino kernel. To upload fonts, images, and graphics, these must be located in a "data" subfolder of the sketch folder.
Ensure you are using a current (01/2026) Arduino IDE plugin for uploading. With the Arduino IDE open, you can access the command menu using the keyboard shortcut [Ctrl] + [Shift] + [P]. There should be an entry there
called "Upload LittleFS to The file should be located in the "Pico/ESP8266/ESP32" folder. The data to be uploaded must be located in [Sketch folder]\data.

The connections between the ESP8266/ESP32/xxx and the TFT are described in the TFT_eSPI library (File: User_Setup_Select.h).
The JSON parser used is the one from Bodmer (https://github.com/Bodmer/JSON_Decoder) (do not use the one from the Arduino library collection).

Das hier abgebildete Gehäuse ist ein Versuch, es zu beschichten. Ein sauberer 3D-Druck ist ansehnlicher...
The housing shown here is an attempt at coating it. A clean 3D print looks better...

<picture>
  <img alt="first screen" src="/pictures/1.jpg">
</picture>
<picture>
  <img alt="second screen" src="/pictures/2.jpg">
</picture>
<picture>
  <img alt="board" src="/pictures/3.jpg">
</picture>
<picture>
  <img alt="TFT" src="/pictures/4.jpg">
</picture>
<picture>
  <img alt="3d-print" src="/pictures/5.jpg">
</picture>
