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
constexpr int CELL_NUMBER_HORIZONTAL = 75 * 2;
constexpr int CELL_NUMBER_VERTICAL = 45 * 2;
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

std::map<Coords, Case> mapGrille;
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

            // mapGrille[{gx, gy}] = Case{{gx, gy},
            //                            {(float)gx * (CELL_SIZE + CELL_SPACING), (float)gy * (CELL_SIZE + CELL_SPACING)},
            //                            false,
            //                            0};
        }
    }
    // Variables de gestion du OnTick : permettent d’exécuter du code à intervalle régulier
    float timer = 0.0f;
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

        BeginDrawing();
        ClearBackground(background);

        BeginMode2D(camera); // ----- Caméra activée -----

        // Dessin de la grille
        for (auto &val : grille)
        {
            DrawRectangle(val.Coordoner.x, val.Coordoner.y,
                          CELL_SIZE, CELL_SIZE,
                          val.EstActiver ? BLACK : WHITE);
        }

        // Dessin de la grille
        // for (auto &[key, val] : mapGrille)
        // {
        //     DrawRectangle(val.Coordoner.x, val.Coordoner.y,
        //                   CELL_SIZE, CELL_SIZE,
        //                   val.EstActiver ? BLACK : WHITE);
        // }

        EndMode2D(); // ----- Caméra désactivée -----

        // Clic souris
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Conversion coordonnées souris -> coordonnées caméra
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
            Coords caseToucher = FindCaseWithCoo(mousePos);

            // trouve la case toucher par le clique de la souris puis update sa valeur
            // auto it = mapGrille.find(caseToucher);
            // Case c = grille[caseToucher.second * CELL_NUMBER_HORIZONTAL + caseToucher.first];
            int index = caseToucher.second * CELL_NUMBER_HORIZONTAL + caseToucher.first;
            //int index = caseToucher.first * CELL_NUMBER_VERTICAL + caseToucher.second;

            if (index >= 0 && index < grille.size())
            {
                grille[index].EstActiver = !grille[index].EstActiver;
                int c = index;
                std::cout
                << " click sur : " << " " << c << " "
    << c + 1 << " "
    << c - 1 << " "
    << c + CELL_NUMBER_HORIZONTAL << " "
    << c + CELL_NUMBER_HORIZONTAL + 1 << " "
    << c + CELL_NUMBER_HORIZONTAL - 1 << " "
    << c - CELL_NUMBER_HORIZONTAL << " "
    << c - CELL_NUMBER_HORIZONTAL + 1 << " "
    << c - CELL_NUMBER_HORIZONTAL - 1
    << "\n";
            }
            // if (it != mapGrille.end()){
            //     it->second.EstActiver = !it->second.EstActiver;
            // }
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
// std::vector<Coords> getVoisins(Coords c)
// {
//     return {
//         {c.first, c.second - 1},
//         {c.first + 1, c.second - 1},
//         {c.first + 1, c.second},
//         {c.first + 1, c.second + 1},
//         {c.first, c.second + 1},
//         {c.first - 1, c.second + 1},
//         {c.first - 1, c.second},
//         {c.first - 1, c.second - 1}};
// }

std::vector<int> getVoisins(int c)
{
    
    return {
        {c + 1},
        {c - 1},
        {c + CELL_NUMBER_HORIZONTAL},
        {c + CELL_NUMBER_HORIZONTAL + 1},
        {c + CELL_NUMBER_HORIZONTAL -1},
        {c - CELL_NUMBER_HORIZONTAL},
        {c - CELL_NUMBER_HORIZONTAL + 1},
        {c - CELL_NUMBER_HORIZONTAL -1}};
}

int nbrVoisin = 0;
std::map<Coords, int> nouveauxVoisins; /// pour chauq co on a un int
std::map<int, int> nouveauxNbrVoisins; /// pour chauq index  on a un int --- A voir si je peux pas le passer en un array
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
    



    // Comptage des voisins
    // for (auto const &[key, val] : mapGrille) // le & veut dire que tu accéde directement a la valeur sans faire de copie ( si tu le met pas sa fais un copie )
    // {
    //     nbrVoisin = 0;
    //     for (auto const &voisins : getVoisins(key))
    //     {
    //         // auto it = mapGrille.find(voisins);
    //         auto it = mapGrille.find(voisins);
    //         if (it != mapGrille.end() /* .end donne l'élément aprés le dernier de la liste donc la fin de la lsite*/ && it->second.EstActiver) // si le voisin est activer cela ajoute 1 au nbr de voisin. second = la 2éme valeur dans le duo qui cosntiture un entre de map donc valeur plutot que item
    //             nbrVoisin++;
    //     }
    //     nouveauxVoisins[key] = nbrVoisin;
    // }
    //  Mise à jour des cases
    // for (auto &[key, val] : mapGrille)
    // {
    //     val.nombreVoisin = nouveauxVoisins[key];
    //     if (val.nombreVoisin == 3)
    //         val.EstActiver = true;
    //     else if (val.EstActiver && val.nombreVoisin == 2)
    //         val.EstActiver = true;
    //     else
    //         val.EstActiver = false;
    // }
}

// note profilleur : premiére ereur fais de linitiation de variable dans verif voisin ensuite c'est find dans la map qui est un peu long