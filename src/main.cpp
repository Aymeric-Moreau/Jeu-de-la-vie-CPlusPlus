#include <raylib.h>
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <utility>
#include <array>

#include <algorithm>
#include <unordered_set>


// --- Paramètres globaux ---
constexpr int CELL_SIZE = 10;                   // Taille d'une case
constexpr int CELL_SPACING = 1;                 // Espace entre les cases
constexpr int CELL_NUMBER_HORIZONTAL = 75 * 128; // dabors sans multiplicateur puis 2 puis quand 2 valide alors 4 ensuite 8 et 16 puis 32 pui 64 puis 128 puis 256
constexpr int CELL_NUMBER_VERTICAL = 45 * 128;
constexpr Color BACKGROUND = BLUE; //
constexpr int SCREENWIDTH = 1540;
constexpr int SCREENHEIGHT = 990;

using Coords = std::pair<int, int>;

// struct perso pour les infos de chaque Case de la grille
struct Case
{
    Coords index;
    Vector2 Coordoner; // Coordonnées en pixels
    bool EstActiver{false};
    int nombreVoisin{0};
};

// coordonée de chaque coin de l'écran
Vector2 topLeftWorld;
Vector2 topRightWorld;
Vector2 bottomLeftWorld;
Vector2 bottomRightWorld;

// Variable servant a faire l'affichage et les vérification dynamique
std::vector<Case> caseVisible;
std::unordered_set<int> caseAverifier;
std::unordered_set<int> caseActiver;


// array stockant la grille compléte
std::array<Case, CELL_NUMBER_HORIZONTAL * CELL_NUMBER_VERTICAL> grille;

// pré-déclarations de mes fonctions
void UpdateViewportCoord();
std::vector<Case> GetAllCaseBetweenMinMaxCo(Vector2 CoMin, Vector2 CoMax);
std::vector<Coords> GetVoisins(Coords c);
int FindCaseWithCoo(Vector2 mousePos);
void ActiverCase(int index);
void VerifVoisins(float dt);
void DesactiverCase(int index);
void ActiverCase(int index);
void AjouterAuCaseAVerif(int c);
void SupprimerAuCaseAVerif(int c);
void ConstructionCaseAVerif();


Camera2D camera;

int main()
{

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Raylib jeu de la vie");
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

    constexpr float interval = 1.0f;
    float showTextTimer = 0.0f; // Compteur pour l’affichage temporaire de texte
    bool timerActive{true};

    // spawn de la ball qui nous servira de "character" invisible pour qu'on puisse déplacer la caméra
    float gridWidth = CELL_NUMBER_HORIZONTAL * (CELL_SIZE + CELL_SPACING);
    float gridHeight = CELL_NUMBER_VERTICAL * (CELL_SIZE + CELL_SPACING);
    Vector2 gridCenter = {gridWidth / 2.0f, gridHeight / 2.0f};
    Vector2 ballPosition = gridCenter;

    // Définition de la caméra
    camera = {0};
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
        {
            ballPosition.x += speed * dt;
        }

        if (IsKeyDown(KEY_LEFT))
        {
            ballPosition.x -= speed * dt;
        }

        if (IsKeyDown(KEY_UP))
        {
            ballPosition.y -= speed * dt;
        }

        if (IsKeyDown(KEY_DOWN))
        {
            ballPosition.y += speed * dt;
        }

        // Mise à jour du timer

        timer += dt;

        // while (!WindowShouldClose()) / event tick mais avec un delay entre chaque execution pour optimiser ce qui n'a pas besoin d'être fais a chaque frame
        if (timer >= interval && timerActive)
        {
            timer = 0.0f;
            VerifVoisins(dt);
            showTextTimer = 1.0f;
        }

        if (showTextTimer > 0.0f)
            showTextTimer -= dt;

        // Mise à jour de la caméra pour qu’elle suive la boule
        camera.target = ballPosition;
        camera.offset = {(float)SCREENWIDTH / 2, (float)SCREENHEIGHT / 2};

        UpdateViewportCoord();
        GetAllCaseBetweenMinMaxCo(topLeftWorld, bottomRightWorld);



        BeginDrawing();
        ClearBackground(BACKGROUND);

        BeginMode2D(camera); // ----- Caméra activée -----

        for (auto &val : caseVisible)
        {
            DrawRectangle(val.Coordoner.x, val.Coordoner.y,
                          CELL_SIZE, CELL_SIZE,
                          val.EstActiver ? BLACK : WHITE);
        }
        EndMode2D(); // ----- Caméra désactivée -----

        // Clic souris  IsMouseButtonDown(int button)

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            // Conversion coordonnées souris -> coordonnées caméra
            Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
            // Coords caseToucher = FindCaseWithCoo(mousePos);

            // trouve la case toucher par le clique de la souris puis update sa valeur

            int index = FindCaseWithCoo(mousePos);

            if (index >= 0 && index < grille.size())
            {
                
                if (grille[index].EstActiver)
                {
                    DesactiverCase(index);
                }
                else
                {
                    ActiverCase(index);
                }
                
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
int FindCaseWithCoo(Vector2 mousePos)
{
    int caseX = (int)floor(mousePos.x / (CELL_SIZE + CELL_SPACING));
    int caseY = (int)floor(mousePos.y / (CELL_SIZE + CELL_SPACING));
    // int index = caseY * CELL_NUMBER_HORIZONTAL + caseX;
    return caseY * CELL_NUMBER_HORIZONTAL + caseX;
}

void UpdateViewportCoord()
{
    topLeftWorld = GetScreenToWorld2D((Vector2){0, 0}, camera);
    topRightWorld = GetScreenToWorld2D((Vector2){SCREENWIDTH, 0}, camera);
    bottomLeftWorld = GetScreenToWorld2D((Vector2){0, SCREENHEIGHT}, camera);
    bottomRightWorld = GetScreenToWorld2D((Vector2){SCREENWIDTH, SCREENHEIGHT}, camera);
}


std::vector<Case> GetAllCaseBetweenMinMaxCo(Vector2 CoMin, Vector2 CoMax)
{
    caseVisible.clear();

    // Convertir pixels monde -> coordonnées grille
    int minX = (int)floor(CoMin.x / (CELL_SIZE + CELL_SPACING));
    int minY = (int)floor(CoMin.y / (CELL_SIZE + CELL_SPACING));

    int maxX = (int)floor(CoMax.x / (CELL_SIZE + CELL_SPACING));
    int maxY = (int)floor(CoMax.y / (CELL_SIZE + CELL_SPACING));

    // Clamp pour éviter débordement
    minX = std::max(0, minX);
    minY = std::max(0, minY);
    maxX = std::min(CELL_NUMBER_HORIZONTAL - 1, maxX);
    maxY = std::min(CELL_NUMBER_VERTICAL - 1, maxY);

    // Boucle propre en coordonnées
    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            int index = x + y * CELL_NUMBER_HORIZONTAL;
            caseVisible.push_back(grille[index]);
        }
    }

    return caseVisible;
}


std::array<int, 8> voisins;
std::array<int, 8> GetVoisins(int c)
{
    voisins = {
        c + 1,
        c - 1,
        c + CELL_NUMBER_HORIZONTAL,
        c + CELL_NUMBER_HORIZONTAL + 1,
        c + CELL_NUMBER_HORIZONTAL - 1,
        c - CELL_NUMBER_HORIZONTAL,
        c - CELL_NUMBER_HORIZONTAL + 1,
        c - CELL_NUMBER_HORIZONTAL - 1};
    return voisins;
}
std::array<int, 8> voisinCaseActiver;
std::array<int, 8> voisinCaseActiverVoisin;
void ActiverCase(int index)
{

    if (index >= 0 && index < grille.size())
    {
        grille[index].EstActiver = true;
        AjouterAuCaseAVerif(index);
        // voisinCaseActiver = GetVoisins(index);
        // for (int i = 0; i < voisinCaseActiver.size(); i++)
        // {
        //     AjouterAuCaseAVerif(voisinCaseActiver[i]);
        //     voisinCaseActiverVoisin = GetVoisins(voisinCaseActiver[i]);
        //     for (int y = 0; y < voisinCaseActiverVoisin.size(); y++)
        // {
        //     AjouterAuCaseAVerif(voisinCaseActiverVoisin[y]);
        // }

        // }
        
    }
}

void DesactiverCase(int index)
{

    if (index >= 0 && index < grille.size())
    {
        grille[index].EstActiver = false;
        SupprimerAuCaseAVerif(index);
    }
}
// caseAverifier
void AjouterAuCaseAVerif(int c){

    caseActiver.insert(c);

}

void SupprimerAuCaseAVerif(int c){
    caseActiver.erase(c);
}

void ConstructionCaseAVerif(){
    caseAverifier = caseActiver;
    for (auto &&i : caseActiver)
    {
        for (auto &&i : GetVoisins(i))
        {
            caseAverifier.insert(i);
        }
        

    }
    
}

int nbrVoisin = 0;

std::array<int, CELL_NUMBER_HORIZONTAL * CELL_NUMBER_VERTICAL> nouveauxNbrVoisins; 
void VerifVoisins(float dt)
{
    ConstructionCaseAVerif();
for (auto &&i : caseAverifier)
{
     nbrVoisin = 0;
        for (auto const &voisins : GetVoisins(i))
        {

            if (voisins >= 0 && voisins < grille.size())
            {
                if (grille[voisins].EstActiver) // si le voisin est activer cela ajoute 1 au nbr de voisin.
                    nbrVoisin++;
            }
        }
        nouveauxNbrVoisins[i] = nbrVoisin;
}


    for (auto &&i : caseAverifier)
{
grille[i].nombreVoisin = nouveauxNbrVoisins[i];

                if (grille[i].nombreVoisin == 3)
            ActiverCase(i);
        else if (grille[i].EstActiver && grille[i].nombreVoisin == 2)
            ActiverCase(i);
        else
            DesactiverCase(i);
}


}


// note profilleur : premiére ereur fais de linitiation de variable dans verif voisin ensuite c'est find dans la map qui est un peu long
// une fois la map corriger on passe en x4 et la map nouveauxNbrVoisins prend trop de temp
// aprés la map nouveauxNbrVoisins j'ai opti get voisin pour pas que sa allocator un vector a chaque fois je l'ai passer sous array avec instanciation avant
// avec un fois 8 on est a 30 fps alors que avec le x 2 on est a 60 on sent que c'est lent mais il y a pas de ralentissement et verifvoisin fais 0.08
// le probléme est que l'on déssine tous les cas 3 milion de case alors qu'il faudrait déssiner que ceux visible a la caméra
// j'ai donc limiter les dessin au case présent a la caméra
// ensuite ne passant a x32 j'ai eu des probléme avec VérifVoisin car sa fesais trop de case
// donc j'ai limiter les nombre de case a vérif uniquement avec ceux activer et leur voisin
// une foi le x256 test la grille est trop grand pour etre stocker dans l'array grille

// int GetScreenWidth(void);                                   // Get current screen width
// int GetScreenHeight(void);                                  // Get current screen height
