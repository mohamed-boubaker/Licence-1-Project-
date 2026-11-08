
#include <Grapic.h>
using namespace grapic;
#include <iostream>
#include <math.h>
using namespace std;

const int DIMW = 600;
const int MAX_ROCKS = 12;
const float G = 9.81;
const float dt = 0.1f;

struct Cplx
{
    float x,y;
};
struct Vec
{
    float x, y;
    float l()
    {
        return sqrt(x*x + y*y);
    }
    float mx;
    float my;
};
Vec m_v(float x, float y)
{
    Vec v;
    v.x = x;
    v.y = y;
    return v;
}
Vec operator+(const Vec& a, const Vec& b)
{
    Vec r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}
Vec operator-(const Vec& a, const Vec& b)
{
    Vec r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

Vec operator+=(Vec& a, const Vec& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}
Vec operator*(float a, const Vec& b)
{
    Vec r;
    r.x = a * b.x;
    r.y = a * b.y;
    return r;
}
Vec operator/(const Vec& a, float b)
{
    Vec r;
    r.x = a.x / b;
    r.y = a.y / b;
    return r;
}
Vec operator*(const Vec& a, const Vec& b)
{
    Vec r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    return r;
}
Vec operator*(const Vec& v, float s)
{
    Vec r;
    r.x= v.x * s;
    r.y = v.y * s;
    return r;
}

Cplx m_c(float r, float i)
{
    Cplx c;
    c.x = r;
    c.y = i;
    return c;
}

Cplx m_ce(float r, float theta)
{
    Cplx c;
    c.x = r*cos(theta);
    c.y = r*sin(theta);
    return c;
}
Cplx operator-(Cplx a, Cplx b)
{
    Cplx c = m_c( a.x-b.x, a.y-b.y );
    return c;
}
Cplx operator*(float a, Cplx b)
{
    Cplx c = m_c( a*b.x, a*b.y );
    return c;
}
Cplx operator*(Cplx b, float a)
{
    Cplx c = m_c( a*b.x, a*b.y );
    return c;
}


Cplx operator/(Cplx b, float d)
{
    Cplx c = m_c( b.x/d, b.y/d );
    return c;
}
Cplx operator*(Cplx a, Cplx b)
{
    Cplx c = m_c( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x );
    return c;
}
Cplx operator+(Cplx a, Cplx b)
{
    return m_c( a.x+b.x, a.y+b.y );
}
float to_r(float deg)
{
    return M_PI*deg/180.f;
}
Cplx ro(Cplx p, float cx, float cy, float theta_deg)
{
    Cplx rot = m_ce( 1, to_r(theta_deg));
    Cplx tr = m_c( cx, cy);
    return ((p-tr)*rot)+tr;
}

float nm(const Vec v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

struct Part
{
    Vec p; // La position de p est (p.x, p.y)
    Vec v; // vitesse en m/s (v.x,v.y)
    Vec f; // force en N (f.x, f.y)
    float m; // masse en kg
    float radius;


};


struct Rock
{
    Vec pos;
    float radius;
    float speed; // vitesse horizontale des rochers
};

struct World
{
    Part alien;
    int nb_rocks;
    Rock moon;
    Rock rocks[MAX_ROCKS];
     Image img_background;
    Image img_alien;
    Image img_rocks;
    Image img_rock1;
    Image img_rock2;
    Image img_moon;
    int scor;
    Cplx cata;
    Cplx moon1;
};

Vec forceLauncher(World& w)
{
    Vec f = m_v(0, 0); // Vecteur nul

    if (isMousePressed(SDL_BUTTON_LEFT))
    {
        int x, y;
        mousePos(x, y);
        Vec mousePos = m_v(x, y);
        Vec launcherPos = w.alien.p;
        f = mousePos - launcherPos;
    }

    return f;
}

void initWorld(World& w)
{
    // Ini de l'alien
    w.alien.p.x = DIMW / 2;
    w.alien.p.y = 30;
    w.alien.v.x = 0;
    w.alien.v.y = 0;
    w.alien.f.x = 0;
    w.alien.f.y = 0;
    w.alien.m = 10;
    w.alien.radius = 5;

    // Ini de la lune
    w.moon.radius = 30;
    w.moon.pos.x = rand() % (DIMW - int(w.moon.radius * 2));
    w.moon.pos.y = DIMW - 50;

    // ini de cool
    w.moon1 = m_c(w.moon.pos.x, w.moon.pos.y);
    w.cata = w.moon1 + m_c(40, 0);

    // Définir les zones à éviter autour de l'alien et de la lune
    float alien_radius_avoid = 100.0f; // Rayon de la zone à éviter autour de l'alien
    float moon_radius_avoid = 150.0f;   // Rayon de la zone à éviter autour de la lune

    // Initialisation des rochers
    w.nb_rocks = 12;
    for (int i = 0; i < w.nb_rocks; i++)
    {
        bool valid_position = false;
        while (!valid_position)
        {
            // Générer une position aléatoire pour le rocher
            w.rocks[i].pos.x = rand() % int(DIMW - w.rocks[i].radius * 2);
            w.rocks[i].pos.y = rand() % int(DIMW - w.rocks[i].radius * 2);

            // Vérifier si la position est valide (pas dans les zones à éviter)
            Vec distance_to_alien = w.rocks[i].pos - w.alien.p;
            Vec distance_to_moon = w.rocks[i].pos - w.moon.pos;

            if (distance_to_alien.l() > alien_radius_avoid && distance_to_moon.l() > moon_radius_avoid)
            {
                valid_position = true;
            }
        }

        // Ini le rayon / vitesse du rocher
        w.rocks[i].radius = (rand() % 50) + 5;
        w.rocks[i].speed = static_cast<float>(rand() % 20 + 5) / 500.0f; // vitesse aléatoire entre 1 et 5
    }
}

// Vitesse max alien
void maxVitesseAlien(World& w)
{
    int maxvitesse = 7;
    if (w.alien.v.y > maxvitesse)
        w.alien.v.y = maxvitesse;
    if (w.alien.v.x > maxvitesse)
        w.alien.v.x = maxvitesse;

    int maxforce = 7;
    if (w.alien.f.y > maxforce)
        w.alien.f.y = maxforce;
    if (w.alien.f.x > maxforce)
        w.alien.f.x = maxforce;
}

void draw(World& w)
{
    image_draw(w.img_background, 0, 0, DIMW, DIMW);

    // Récupération de la position de l'alien et du curseur
    Vec alien_pos = w.alien.p;
    Vec f = forceLauncher(w);


    // Affichage de l'alien en tenant compte de l'angle calculé
    image_draw(w.img_alien, w.alien.p.x - 30, w.alien.p.y - 30, 50, 50);

    // Affichage rochers
    for (int i = 0; i < w.nb_rocks; i++)
    {
        if (i <= 4)
        {
            image_draw(w.img_rocks, w.rocks[i].pos.x - w.rocks[i].radius,
                       w.rocks[i].pos.y - w.rocks[i].radius, w.rocks[i].radius * 2,
                       w.rocks[i].radius * 2);
        }
        else if (i <= 8)
        {
            image_draw(w.img_rock1, w.rocks[i].pos.x - w.rocks[i].radius,
                       w.rocks[i].pos.y - w.rocks[i].radius, w.rocks[i].radius * 2,
                       w.rocks[i].radius * 2);
        }
        else if (i <= 12)
        {
            image_draw(w.img_rock2, w.rocks[i].pos.x - w.rocks[i].radius,
                       w.rocks[i].pos.y - w.rocks[i].radius, w.rocks[i].radius * 2,
                       w.rocks[i].radius * 2);
        }
    }

    // Affichage lune
    color(255, 255, 255);
    circleFill(w.moon.pos.x, w.moon.pos.y, w.moon.radius);
    image_draw(w.img_moon, w.moon.pos.x - 37, w.moon.pos.y - 35, w.moon.radius * 2.5, w.moon.radius * 2.5);

    // Affichage lanceur
    Vec force = forceLauncher(w);

    // couleur du trait entre l'alien et la souris

    color(255, 255, 255);
    line(w.alien.p.x, w.alien.p.y, w.alien.p.x + f.x - w.alien.radius / 2, w.alien.p.y + f.y - w.alien.radius / 2);
}



bool alienWin(const World& w)
{
    //  si l'alien a atteint la lune
    if ((w.alien.p - w.moon.pos).l() < w.moon.radius + w.alien.radius * 0.2)
    {
        return true;
    }
    return false; // Sinon, retourne false
}

bool collision(World& w)
{
    for (int i = 0; i < w.nb_rocks; i++)
    {
        if ((w.alien.p - w.rocks[i].pos).l() < w.rocks[i].radius + w.alien.radius * 2)
        {
            return true; // S'il y a collision : arrête la boucle
        }
    }
    return false; // Si aucune collision : retourne false
}

bool nonwin(const World& w)
{
    // Vérifie si l'alien est sorti de l'écran
    if (w.alien.p.x < -2.0f || w.alien.p.x > DIMW + 2.0f || w.alien.p.y < -2.0f || w.alien.p.y > DIMW + 1.0f)
    {
        return true;
    }
    return false;
}

void update(World& w, float delta_t)
{
    w.cata = ro(w.cata, w.moon1.x, w.moon1.y, 0.5);
//cool
for (int i = 0; i < 2; i++) {
    color(255, 255, 255);
    Cplx pos = ro(w.cata, w.moon1.x, w.moon1.y, i*-1);
    circleFill(pos.x, pos.y, 5);
}

    // Calcul de la force appliquée sur l'alien
    Vec f = forceLauncher(w);
    if (isMousePressed(SDL_BUTTON_LEFT))
    {
        w.alien.f = f / 500;
    }

    for (int i = 0; i < w.nb_rocks; i++)
    {
       for (int i = 0; i < w.nb_rocks; i++)
    {
        // Calcul des forces gravitationnelles  par les rochers sur l'alien
        Vec dir = w.rocks[i].pos - w.alien.p;
        float distance = dir.l();
        float force_magnitude = G * w.alien.m * w.rocks[i].radius / (distance * distance) * 0.025;
        Vec force = (force_magnitude / distance) * dir;
        w.alien.f += force;
    }

        // Mise à jour de la position
        w.rocks[i].pos.x += w.rocks[i].speed;

        // Vérification et ajustement de la position si le rocher dépasse les limites de l'écran
        if (w.rocks[i].pos.x - 34 > DIMW)
        {
            w.rocks[i].pos.x = -w.rocks[i].radius * 2; // Réapparaît à gauche de l'écran
        }
        else if (w.rocks[i].pos.x < -w.rocks[i].radius * 2)
        {
            w.rocks[i].pos.x = DIMW; // Réapparaît à droite de l'écran
        }
    }

    // Mise à jour de la position et de la vitesse de l'alien
    w.alien.v += (delta_t / w.alien.m) * w.alien.f / 9;
    w.alien.p += delta_t * w.alien.v;

    // Réinitialisation de la force appliquée sur l'alien
    w.alien.f = m_v(0, 0);

    // Test de fin de partie
    static float last_event = 0.0; // temps écoulé depuis la dernière collision ou victoire
    if (alienWin(w))
    {
        cout << "+10!"
         << endl;
        last_event = 0.0; // réinitialisation du temps
         w.scor += 10;
        initWorld(w);
    }
    else if (collision(w))
    {
        cout << "-5 !"
        << endl;
        last_event = 0.0; // réinitialisation du temps
         w.scor -= 5;
        initWorld(w);
    }
    else if (nonwin(w))
    {
        cout << "-5 !"
         << endl;
        last_event = 0.0; // réinitialisation du temps
         w.scor -= 5;
        initWorld(w);
    }
    else
    {
        last_event += delta_t;
        if (last_event > 1000.0)
        {
            if (isKeyPressed(SDLK_r))
            {
                initWorld(w);
                last_event = 0.0; // réinitialisation du temps écoulé
            }
        }
    }


}
 void dScor(int Scor)
{
    color(255, 255, 255);
    fontSize(30);
    print(DIMW / 2, DIMW - 40, Scor);
}
int main(int, char**)
{
    srand(time(NULL)); // Pour que l'aléatoire change à chaque fois.
    bool stop = false;
    winInit("Rocks", DIMW, DIMW);
    setKeyRepeatMode(true);
    World w;
    initWorld(w);

    // Chargement des images
    w.img_alien = image("data/alien.png");
    w.img_rocks = image("data/rocks.png");
    w.img_rock1 = image("data/rock1.png");
    w.img_rock2 = image("data/rock2.png");
    w.img_moon = image("data/moon.png");
    w.img_background = image("data/space_background.png");

    backgroundColor(100, 50, 200);
   w.scor = 0;
    while (!stop)
    {
        winClear();
        draw(w);

        update(w, dt);
           dScor(w.scor);
        maxVitesseAlien(w);
        stop = winDisplay();
    }

    pressSpace();
    winQuit();
    return 0;
}


