#include <iostream>

using namespace std;

#include <gui.h>
#include <vector>

void desenha() {
    GUI::displayInit();

    GUI::setLight(1, 0, 3, 0, true, false);
    //GUI::setLight(2, 5, 3, 5, true, false);
    GUI::setLight(3, -5, 3, 5, true, false);

    GUI::drawOrigin(1);

    GUI::setColor(1, 0, 0);
    GUI::drawFloor();

    //Original
    GUI::drawOrigin(1);

    GUI::setColor(1, 1, 0);

    GUI::drawScaledBox(0.99, 0, 0, -1, 1, 1, 0);
    //GUI::drawScaledBox(0.99, 1, 1, -1, 2, 2, 0);

    //transformado
    //glPushMatrix();
        glTranslatef(glutGUI::tx, glutGUI::ty, glutGUI::tz);
        glRotatef(glutGUI::az, 0, 0, 1);
        glRotatef(glutGUI::ay, 0, 1, 0);
        glRotatef(glutGUI::ax, 1, 0, 0);
        glScalef(glutGUI::sx, glutGUI::sy, glutGUI::sz);
        //GUI::drawOrigin(1);
        GUI::setColor(0, 0, 1, 0.5);
        GUI::drawBox(0, 0, -1, 1, 1, 0);
        //GUI::drawBox(1, 1, -1, 2, 2, 0);
    //glPopMatrix();

    GUI::displayEnd();
}

void teclado(unsigned char key, int x, int y) {
    GUI::keyInit(key, x, y);

    switch (key) {
    case 't':
        glutGUI::trans_obj = !glutGUI::trans_obj;
        break;
    case 'l':
        glutGUI::trans_luz = !glutGUI::trans_luz;
        break;
    case 'i':
        glutGUI::tx = 0.0;
        glutGUI::ty = 0.0;
        glutGUI::tz = 0.0;
        glutGUI::ax = 0.0;
        glutGUI::ay = 0.0;
        glutGUI::az = 0.0;
        glutGUI::sx = 1.0;
        glutGUI::sy = 1.0;
        glutGUI::sz = 1.0;
        break;
    default:
        break;
    }
}

int main()
{
    GUI gui = GUI(800, 600, desenha, teclado);
}
