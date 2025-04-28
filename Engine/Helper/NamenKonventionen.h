#pragma once

#include <string>

const struct namenKonventionenStruct {
    const struct kreatur {
        std::string _self = "Kreatur";
        std::string id = "ID";
        std::string abenteuerPunkte = "abenteuerPunkte";
        std::string steigerungsPunkte = "steigerungsPunkte";
        std::string stufe = "stufe";
    }kreatur;
    const struct basiswerte {
        std::string _self = "basiswerte";
        std::string lebensenergie = "LE";
        std::string astralenergie = "AE";
        std::string ausdauer = "AU";
        std::string magieresistenz = "MR";
        std::string ausweichen = "AW";
        std::string attacke = "AT";
        std::string parade = "PA";
        std::string fernkampf = "FK";
        std::string trefferPunkteWaffe = "TP_Waffe";
        std::string trefferPunkteBoni = "TP_Boni";
    }basiswerte;
    const struct eigenschaften {
        std::string _self = "eigenschaften";
        std::string mut = "MU";
        std::string klugheit = "KL";
        std::string intuition = "IN";
        std::string charisma = "CH";
        std::string fingerfertigkeit = "FF";
        std::string gewandheit = "GE";
        std::string konstitution = "KO";
        std::string koerperkraft = "KK";
    }eigenschaften;
    const struct talente {
        std::string _self = "talente";
        struct koerper {
            std::string _self = "koerper";
            std::string schleichen = "schleichen";
            std::string selbstbeherrschung = "selbstbeherrschung";
            std::string sinnesschaerfe = "sinnesschaerfe";
            std::string taschendiebstahl = "taschendiebstahl";
            std::string zwergennase = "zwergennase";
        } koerper;
        struct natur {
            std::string _self = "natur";
            std::string tierkunde = "tierkunde";
            std::string pflanzenkunde = "pflanzenkunde";
            std::string wildnisleben = "wildnisleben";
            std::string fallenstellen = "fallenstellen";
        } natur;
        struct wissen {
            std::string _self = "wissen";
            std::string gassenwissen = "gassenwissen";
            std::string heilkundeGift = "heilkundeGift";
            std::string heilkundeWunden = "heilkundeWunden";
            std::string magiekunde = "magiekunde";
        } wissen;
        struct gesellschaft {
            std::string _self = "gesellschaft";
            std::string betoeren = "betoeren";
            std::string etikette = "etikette";
            std::string feilschen = "feilschen";
            std::string menschenkenntnis = "menschenkenntnis";
            std::string ueberreden = "ueberreden";
        } gesellschaft;
        struct handwerk {
            std::string _self = "handwerk";
            std::string alchimie = "alchimie";
            std::string bogenbau = "bogenbau";
            std::string fallenEntschaerfen = "fallenEntschaerfen";
            std::string schloesserKnacken = "schloesserKnacken";
            std::string schmieden = "schmieden";
        } handwerk;
        struct nahkampf {
            std::string _self = "nahkampf";
            std::string dolche = "dolche";
            std::string fechtwaffen = "fechtwaffen";
            std::string hiebwaffen = "hiebwaffen";
            std::string saebel = "saebel";
            std::string schwerter = "schwerter";
            std::string speere = "speere";
            std::string staebe = "staebe";
            std::string zweihandHiebwaffen = "zweihandHiebwaffen";
            std::string zweihandSchwerter = "zweihandSchwerter";
            std::string raufen = "raufen";
        } nahkampf;
        struct fernkampf {
            std::string _self = "fernkampf";
            std::string armbrust = "armbrust";
            std::string bogen = "bogen";
            std::string wurfwaffen = "wurfwaffen";
        } fernkampf;
    }talente;
    const struct zauber {
        std::string _self = "Zauber";
    }zauber;
    const struct inventarObjekt {
        std::string _self = "inventarObjekt";
        std::string id = "id";
        std::string effektDauer = "effektDauer";    //in Sekunden
        struct effekt {
            std::string _self = "effekt";
            std::string Eigenschaften = "Eigenschaften";
            std::string Talente = "Talente";
            std::string Basiswerte = "Basiswerte";
        }effekt; 
        struct benötigt{
            std::string _self = "benötigt";
            std::string Eigenschaften = "Eigenschaften";
            std::string Talente = "Talente";
            std::string Basiswerte = "Basiswerte";
        }benötigt;
        struct typ{
            std::string _self = "typ";
            struct equipment {
                std::string _self = "equipment";
                std::string Kopf = "Kopf";
                std::string Schulter = "Schulter";
                std::string EinArmWaffe = "EinArmWaffe";
                std::string ZweiArmWaffe = "ZweiArmWaffe";
                std::string Schild = "Schild";
                std::string Oberkoerper = "Oberkoerper";
                std::string Schuhe = "Schuhe";
                std::string Beinschienen = "Beinschienen";
                std::string Handschuhe = "Handschuhe";
                std::string Armschienen = "Armschienen";
                std::string Köcher = "Köcher";
                std::string Halskette = "Halskette";
                std::string Ring = "Ring";
            }equipment;
            struct verbrauchsGegenstand {
                std::string _self = "verbrauchsGegenstand";
                std::string trank = "trank";
            }verbrauchsGegenstand;
            struct text {
                std::string _self = "text";
                std::string buch = "buch";
                std::string dokument = "dokument";
                std::string notiz = "notiz"; //für eigene Notizen?
            }text;
        }typ;
    }inventarObjekt;
    const struct inventar {
        std::string _self = "Inventar";
        std::string equipment = "Equipment";
        std::string mainInventory = "Hauptinventar";
        std::string questInventory = "QuestInventar";
    }inventar;
    const struct options {
        std::string _self = "optionen";
        std::string dispResX = "dispResX";
        std::string dispResY = "dispResY";
        std::string windowName = "windowName";
    }options;
    const struct renderObject{
        std::string _self = "renderObject";
        std::string id = "id";
        std::string isOverlay = "isOverlay";
        std::string positionX = "posX";
        std::string positionY = "posY";
        std::string pixelSizeX = "pixSizeX";
        std::string pixelSizeY = "pixSizeY";
        std::string imageLocation = "imgLocation";
        std::string layer = "layer";
        std::string isSpritesheet = "isSpritesheet";
        std::string spritesheetSizeX = "spritesheetSizeX";
        std::string spritesheetSizeY = "spritesheetSizeY";
        std::string spritesheetOffsetX = "spritesheetOffsetX";
        std::string spritesheetOffsetY = "spritesheetOffsetY";
        std::string deleteFlag = "deleteFlag";
        std::string reloadInvokes = "reloadInvokes";
        std::string textFontsize = "text.fontsize";
        std::string textStr = "text.str";
        std::string textDx = "text.dx";
        std::string textDy = "text.dy";
    }renderObject;
    const struct MyTemplate {
        std::string _self = "MyTemplate";
        std::string bsp1 = "Beispiel1";
    }MyTemplate;
    const struct MoveRuleSet {
        std::string _self = "moveRuleSet";
        std::string variable = "variable";
        std::string counter = "counter";
        std::string arrRuleSet = "varRuleSet";  
        std::string tick = "tick";
        std::string startValue = "startValue";
        std::string repeat = "repeat";
        std::string wait = "wait";
        std::string loop = "loop";
    }moveRuleSet;
    const struct Calculator {
        std::string add = "add";
        std::string sin_b = "sin_b";
        std::string cos_b = "cos_b";
        std::string a_sin_b = "a_sin_b";
        std::string a_cos_b = "a_cos_b";
        std::string b_sin_a = "b_sin_a";
        std::string b_cos_a = "b_cos_a";
        std::string a_times_b = "a_times_b";
        std::string setNoSkip = "setNoSkip";
        std::string set = "set";
        
    }calculator;
    const struct Environment {
        std::string renderObjectContainer = "RenderObjectContainer";
    }environment;
    const struct testImages {
        std::string folder001 = "Resources/Sprites/TEST001P/";
        std::string folder100 = "Resources/Sprites/TEST100P/";
    }testImages;
}namenKonvention;
