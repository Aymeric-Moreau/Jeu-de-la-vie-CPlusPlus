#include <raylib.h>
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <utility>
#include <array>
// #include <unordered_map>

// --- Paramètres globaux ---
constexpr int CELL_SIZE = 25;   // Taille d'une case
constexpr int CELL_SPACING = 1; // Espace entre les cases
constexpr int CELL_NUMBER_HORIZONTAL = 75 * 16; // dabors sans multiplicateur puis 2 puis quand 2 valide alors 4 ensuite 8 et 16
constexpr int CELL_NUMBER_VERTICAL = 45 * 16;
using Coords = std::pair<int, int>;
// struct perso pour les infos de chaque Case de la grille
struct Case
{
    Coords index;
    Vector2 Coordoner; // Coordonnées en pixels
    bool EstActiver{false};
    int nombreVoisin{0};
};

// On utilise une clé grille (x, y) entière

// std::map<Coords, Case> mapGrille;
std::array<Case, CELL_NUMBER_HORIZONTAL * CELL_NUMBER_VERTICAL> grille;

// pré-déclarations de mes fonctions
std::vector<Coords> getVoisins(Coords c);
Coords FindCaseWithCoo(Vector2 mousePos);
void verifVoisins(float dt);

int main()
{

    constexpr Color background = BLUE;
    constexpr int screenWidth = 1540;
    constexpr int screenHeight = 990;

    InitWindow(screenWidth, screenHeight, "Raylib jeu de la vie");
    SetTargetFPS(60);

    int ActualIndexArray = 0;

    // Initialisation de la grille
    for (int gy = 0; gy < CELL_NUMBER_VERTICAL; gy++)
        {
    for (int gx = 0; gx < CELL_NUMBER_HORIZONTAL; gx++)
    {
        
            grille[ActualIndexArray] = Case{{gx, gy},
                                            {(float)gx * (CELL_SIZE + CELL_SPACING), (float)gy * (CELL_SIZE + CELL_SPACING)},
                                            false,
                                            0};
            ActualIndexArray++;


        }
    }
    // Variables de gestion du OnTick : permettent d’exécuter du code à intervalle régulier
    float timer = 0.0f;
    float timer2 = 0.0f;
    constexpr float interval = 1.0f;
    float showTextTimer = 0.0f; // Compteur pour l’affichage temporaire de texte
    bool timerActive{true};

    // spawn de la ball qui nous servira de "character" invisible pour qu'on puisse déplacer la caméra
    Vector2 ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

    // Définition de la caméra
    Camera2D camera = {0};
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        // Toggle pause
        if (IsKeyPressed(KEY_S))
        {
            timerActive = !timerActive;
        }

        float dt = GetFrameTime();

        constexpr float speed = 300.0f; // pixels/seconde

        if (IsKeyDown(KEY_RIGHT))
            ballPosition.x += speed * dt;
        if (IsKeyDown(KEY_LEFT))
            ballPosition.x -= speed * dt;
        if (IsKeyDown(KEY_UP))
            ballPosition.y -= speed * dt;
        if (IsKeyDown(KEY_DOWN))
            ballPosition.y += speed * dt;

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

        // Mise à jour de la caméra pour qu’elle suive la boule
        camera.target = ballPosition;
        camera.offset = {(float)screenWidth / 2, (float)screenHeight / 2};

        
        timer2 += dt;
        // Dessin de la grille
        // for (auto &val : grille)
        // {
        //     DrawRectangle(val.Coordoner.x, val.Coordoner.y,
        //                   CELL_SIZE, CELL_SIZE,
        //                   val.EstActiver ? BLACK : WHITE);
        // }

        if (timer2 >= 1.0f / 30.0f )
        {
            BeginDrawing();
        ClearBackground(background);

        BeginMode2D(camera); // ----- Caméra activée -----
            timer2 = 0.0f;
            for (auto &val : grille)
        {
            DrawRectangle(val.Coordoner.x, val.Coordoner.y,
                          CELL_SIZE, CELL_SIZE,
                          val.EstActiver ? BLACK : WHITE);
        }
        EndMode2D(); // ----- Caméra désactivée -----
        }



        

        // Clic souris
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Conversion coordonnées souris -> coordonnées caméra
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
            Coords caseToucher = FindCaseWithCoo(mousePos);

            // trouve la case toucher par le clique de la souris puis update sa valeur
            
            int index = caseToucher.second * CELL_NUMBER_HORIZONTAL + caseToucher.first;
            

            if (index >= 0 && index < grille.size())
            {
                grille[index].EstActiver = !grille[index].EstActiver;
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


std::array<int,8> voisins;
std::array<int,8> getVoisins(int c)
{
    voisins = {
        c + 1,
        c - 1,
        c + CELL_NUMBER_HORIZONTAL,
        c + CELL_NUMBER_HORIZONTAL + 1,
        c + CELL_NUMBER_HORIZONTAL -1,
        c - CELL_NUMBER_HORIZONTAL,
        c - CELL_NUMBER_HORIZONTAL + 1,
        c - CELL_NUMBER_HORIZONTAL -1};
    return voisins;
}

int nbrVoisin = 0;

std::array<int, CELL_NUMBER_HORIZONTAL * CELL_NUMBER_VERTICAL > nouveauxNbrVoisins; /// pour chauq index  on a un int --- A voir si je peux pas le passer en un array
void verifVoisins(float dt)
{

    for (size_t i = 0; i < grille.size(); i++)
    {
        nbrVoisin = 0;
        for (auto const &voisins : getVoisins(i))
        {

            if (voisins >= 0 && voisins < grille.size())
            {
                if ( grille[voisins].EstActiver) // si le voisin est activer cela ajoute 1 au nbr de voisin. 
                nbrVoisin++;
            }
            
        }
        nouveauxNbrVoisins[i] = nbrVoisin;
    }

    for (size_t i = 0; i < grille.size(); i++)
    {

        grille[i].nombreVoisin = nouveauxNbrVoisins[i];
        if (grille[i].nombreVoisin == 3)
            grille[i].EstActiver = true;
        else if (grille[i].EstActiver && grille[i].nombreVoisin == 2)
            grille[i].EstActiver = true;
        else
            grille[i].EstActiver = false;
    }
    
}

// note profilleur : premiére ereur fais de linitiation de variable dans verif voisin ensuite c'est find dans la map qui est un peu long
// une fois la map corriger on passe en x4 et la map nouveauxNbrVoisins prend trop de temp
// aprés la map nouveauxNbrVoisins j'ai opti get voisin pour pas que sa allocator un vector a chaque fois je l'ai passer sous array avec instanciation avant
// avec un fois 8 on est a 30 fps alors que avec le x 2 on est a 60 on sent que c'est lent mais il y a pas de ralentissement et verifvoisin fais 0.08