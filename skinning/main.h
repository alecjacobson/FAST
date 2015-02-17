// GLUT version of Skinning.app

// Forward declaration of skinning class
class Skinning;

// Skinning object
Skinning * skinning;

////////////////////////////////////////////////////////////////////////////
// GLUT callbacks
////////////////////////////////////////////////////////////////////////////
void Display(void);
void Reshape(int width, int height);
void Terminate(void);
void key(unsigned char key,int mouse_x, int mouse_y);
void mouse(int glutButton, int glutState, int mouse_x, int mouse_y);
void mouse_move(int mouse_x, int mouse_y);
// Main Program
int main(int argc, char *argv[]);
