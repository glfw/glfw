/*========================================================================
 * This is a small test application for GLFW.
 * joystick input test.
 *========================================================================*/

#include <GL/glfw.h>

#include <stdio.h>
#include <math.h>

#define MAX_AXES     10
#define MAX_BUTTONS  30

struct JoystickState
{
    int present;
    int num_axes;
    int num_buttons;
    float axes[MAX_AXES];
    unsigned char buttons[MAX_BUTTONS];
};

static struct JoystickState states[GLFW_JOYSTICK_LAST + 1];

int running;
int keyrepeat  = 0;
int systemkeys = 1;


/*========================================================================
 * Retrieve joystick states
 *========================================================================*/
static void updateJoysticksState(void)
{
    int joy;

    for (joy = GLFW_JOYSTICK_1;  joy < GLFW_JOYSTICK_LAST + 1;  joy++)
    {
        printf("Updating information for joystick %d\n", joy);
        states[joy].present = glfwGetJoystickParam(joy, GLFW_PRESENT);
        if (states[joy].present == GL_TRUE)
        {
            states[joy].num_axes = glfwGetJoystickPos(joy, states[joy].axes, MAX_AXES);
            states[joy].num_buttons = glfwGetJoystickButtons(joy, states[joy].buttons, MAX_BUTTONS);
        }
    }
}

/*========================================================================
 * Print out the state of all joysticks on the standard output
 *========================================================================*/
static void displayJoysticksState(void)
{
    int joy;
    int i;

    for (joy = GLFW_JOYSTICK_1;  joy < GLFW_JOYSTICK_LAST + 1;  joy++)
    {
        printf("Joystick %d: %s\n", joy, (states[joy].present == GL_TRUE ? "present" : "not connected"));

        if (states[joy].present == GL_TRUE)
        {
            if (states[joy].num_axes > 0)
            {
                printf("  axes: %.3f", states[joy].axes[0]);
                for (i = 1;  i < states[joy].num_axes;  i++)
                    printf(", %.3f", states[joy].axes[i]);

                printf("\n");
            }
            else
                printf("  axes: none\n");

            if (states[joy].num_buttons > 0)
            {
                printf("  buttons: 00 => %c", ((states[joy].buttons[0] == GLFW_PRESS) ? 'P' : 'R'));

                for (i = 1;  i < states[joy].num_buttons;  i++)
                    printf(", %02d => %c", i, ((states[joy].buttons[i] == GLFW_PRESS) ? 'P' : 'R'));

                printf("\n");
            }
            else
                printf("  buttons: none\n");
        }
    }
}

int main(void)
{
    double start;
    double t;
    double update;

    /* Initialise GLFW */
    glfwInit();
    printf("The program will work for 20 seconds and display every seconds the state of the joysticks\n");
    printf("Your computer is going to be very slow as the program is doing an active loop .....\n");

    start = glfwGetTime();
    update = start;

    /* print the initial state of all joysticks */
    updateJoysticksState();
    printf("\n");
    displayJoysticksState();

    running = GL_TRUE;

    /* Main loop */
    while (running)
    {
        /* Get time */
        t = glfwGetTime();

        /* Display the state of all connected joysticks every secons */
        if ((t - update) > 1.0)
        {
            update = t;
            printf("\n");
            updateJoysticksState();
            printf("\n");
            displayJoysticksState();
        }

        /* Check if the window was closed */
        if ((t - start) > 20.0)
            running = GL_FALSE;
    }

    /* Close OpenGL window and terminate GLFW */
    glfwTerminate();

    return 0;
}

