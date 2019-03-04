#include <iostream>

using namespace std;

float px = 2.0;
float py = 1.5;

#include <gui.h>
#include <vector>

void desenha() {

    GUI::displayInit();

    GUI::setLight(0, 0, 3, 0, true, false);

    GUI::drawOrigin(1);

    GUI::setColor(1, 1, 1);
    //GUI::drawFloor();
    GUI::drawQuadBox(-3, 0, -3, 3, 3, 3, 0.03, 0.03, true);

    GUI::setColor(3, 1, 2);
    GUI::drawSphere(px, py, 0, 0.2);

    GUI::setColor(1, 2, 1);
    GUI::drawSphere(glutGUI::tx, glutGUI::ty, glutGUI::tz, 0.2);

    GUI::displayEnd();
}

void teclado(unsigned char key, int x, int y) {

    GUI::keyInit(key,x,y);

    switch (key) {
    case 'J':
        px += 0.1;
        glutGUI::tx -= 0.1;
        break;
    case 'j':
        px -= 0.1;
        glutGUI::tx += 0.1;
        break;
    case 'K':
        py += 0.1;
        glutGUI::ty -= 0.1;
        break;
    case 'k':
        py -= 0.1;
        glutGUI::ty += 0.1;
        break;
    case 't':
        glutGUI::trans_obj = !glutGUI::trans_obj;
        break;
    case 'l':
        glutGUI::trans_luz = !glutGUI::trans_luz;
        break;
    default:
        break;
    }
}

int main()
{
    GUI gui = GUI(800, 600, desenha, teclado);
}
