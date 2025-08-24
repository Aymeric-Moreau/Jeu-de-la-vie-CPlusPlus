#include <raylib.h>
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <utility>

// --- Paramètres globaux ---
constexpr int CELL_SIZE = 25;     // Taille d'une case
constexpr int CELL_SPACING = 1;   // Espace entre les cases

// struct perso pour les infos de chaque Case de la grille
struct Case
{
    Vector2 Coordoner; // Coordonnées en pixels
    bool EstActiver{false};
    int nombreVoisin{0};
};

// On utilise une clé grille (x, y) entière
using Coords = std::pair<int, int>;
std::map<Coords, Case> mapGrille;

// pré-déclarations de mes fonctions
std::vector<Coords> getVoisins(Coords c);
Coords FindCaseWithCoo(Vector2 mousePos);
void verifVoisins(float dt);

int main()
{

    Color background = BLUE;
    constexpr int screenWidth = 1540;
    constexpr int screenHeight = 990;

    InitWindow(screenWidth, screenHeight, "Raylib jeu de la vie");
    SetTargetFPS(60);

    // Initialisation de la grille
    for (int gx = 0; gx < 75; gx++)
    {
        for (int gy = 0; gy < 45; gy++)
        {
            mapGrille[{gx, gy}] = Case{
                {(float)gx * (CELL_SIZE + CELL_SPACING), (float)gy * (CELL_SIZE + CELL_SPACING)},
                false,
                0
            };
        }
    }
// Variables de gestion du OnTick : permettent d’exécuter du code à intervalle régulier
float timer = 0.0f;              
const float interval = 1.0f;     
float showTextTimer = 0.0f;      // Compteur pour l’affichage temporaire de texte
bool timerActive{true};          







    while (!WindowShouldClose())
    {
        // Toggle pause
        if (IsKeyPressed(KEY_S))
        {
            timerActive = !timerActive;
        }

        float dt = GetFrameTime();


        // Mise à jour du timer
        
        timer += dt;

        // while (!WindowShouldClose()) / event tick mais avec un delay entre chaque execution pour optimiser ce qui n'a pas besoin d'être fais a chaque frame
        if (timer >= interval && timerActive)
        {
            timer = 0.0f;
            verifVoisins(dt);
            showTextTimer = 1.0f;
        }

        if (showTextTimer > 0.0f)
            showTextTimer -= dt;



        BeginDrawing();
        ClearBackground(background);

         // ----- Caméra activée -----

        // Dessin de la grille
        for (auto &[key, val] : mapGrille)
        {
            DrawRectangle(val.Coordoner.x, val.Coordoner.y,
                          CELL_SIZE, CELL_SIZE,
                          val.EstActiver ? BLACK : WHITE);
        }

         // ----- Caméra désactivée -----

        

        // Clic souris
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // récupération coordonnées souris 
            Vector2 mousePos = GetMousePosition();
            Coords caseToucher = FindCaseWithCoo(mousePos);

            // trouve la case toucher par le clique de la souris puis update sa valeur
            auto it = mapGrille.find(caseToucher);
            if (it != mapGrille.end()){
                it->second.EstActiver = !it->second.EstActiver;
            }
                
        }
        
        // affiche text d'information pour l'interaction avec le boucle d'éxécution des rêgle
        std::string timerStatus = timerActive ? "ACTIF" : "INACTIF";
        std::string fullText = "[S] : Marche/Arret de la boucle | Etat de la boucle : " + timerStatus;
        DrawText(fullText.c_str(), 10, 10, 30, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Conversion coord. pixels -> indices de grille
Coords FindCaseWithCoo(Vector2 mousePos)
{
    int caseX = (int)floor(mousePos.x / (CELL_SIZE + CELL_SPACING));
    int caseY = (int)floor(mousePos.y / (CELL_SIZE + CELL_SPACING));
    return {caseX, caseY};
}

// Voisins en coordonnées grille
std::vector<Coords> getVoisins(Coords c)
{
    return {
        {c.first, c.second - 1},
        {c.first + 1, c.second - 1},
        {c.first + 1, c.second},
        {c.first + 1, c.second + 1},
        {c.first, c.second + 1},
        {c.first - 1, c.second + 1},
        {c.first - 1, c.second},
        {c.first - 1, c.second - 1}
    };
}

void verifVoisins(float dt)
{
    std::map<Coords, int> nouveauxVoisins;

    // Comptage des voisins
    for (auto &[key, val] : mapGrille)
    {
        int nbrVoisin = 0;
        for (auto &v : getVoisins(key))
        {
            auto it = mapGrille.find(v);
            if (it != mapGrille.end() && it->second.EstActiver)
                nbrVoisin++;
        }
        nouveauxVoisins[key] = nbrVoisin;
    }

    //  Mise à jour des cases
    for (auto &[key, val] : mapGrille)
    {
        val.nombreVoisin = nouveauxVoisins[key];
        if (val.nombreVoisin == 3)
            val.EstActiver = true;
        else if (val.EstActiver && val.nombreVoisin == 2)
            val.EstActiver = true;
        else
            val.EstActiver = false;
    }
}
