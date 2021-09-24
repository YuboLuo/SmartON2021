
#define LED 12
#define button 3
#define buzzer 5

int count = 0;
int ButtonPressed = 0;
int flag_1stPress = 0;

int countdown;
int buzzerFreq = 2000;
unsigned long Time=0;


/* eventtype-0 */
//uint8_t events[120] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
//uint8_t event[10][120] = {
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,  0,0,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,  0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,  0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,  1,0,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,1,1,  1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,1,1,0,0,1,1,1,0,1,0,0,1,  1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,  1,0,1,1,1,0,1,1,0,1,0,0,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,0,1,1,0,0,1,0,1,  1,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,  1,0,1,1,1,1,0,0,1,0,0,1,0,1,1,1,0,1,0,1,1,1,1,0,1,1,1,1,1,1,  1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,  0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,  1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,  1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,1,  1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,  0,0,1,0,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,  1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,0,0,1,1,0,1,  1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,  1,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,  1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,0,1,0,1,1,1,0,0,1,0,  0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,1,1,  0,0,1,0,1,1,0,1,1,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,0,1,1,1,0,0,1,  1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1,0,0,  0,1,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,1,  1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,  0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,  0,1,0,0,1,1,0,0,1,1,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,  1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,1,0,  1,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
//};

///* eventtype-1 */
uint8_t event[10][120] = {
{0,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,0,1,0,0,0,1,1,1,0,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,1,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
{1,1,0,1,0,0,0,0,1,1,0,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,1,1,1,0,1,1,1,1,0,0,1,0,0,1,0,1,1,1,1,1,1,1,0,1,1,0,1,0,1,0,0,1,1,1,1,1,0,0,0,1,1,0,1,0,1,1,0,0,1,1,1,1,1,0,1,0,0,1,0,1,0,0,1,1,0,1,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0},
{1,1,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,0,1,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,1,0,1,1,1,0,1,1,0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,0,1,1,1,0,0,1,1,1,0,0},
{1,1,1,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,1,1,1,0,0,1,1,1,0,0,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,0,0,1,1,1,0,0,1,0,1,1,0,0,1,0,0,0,1,1,0,1,0,0,1,1,1,1,0,1,1,1,1,0,1,1,0,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,0,1,0,0,0,0,0,1,0,0,1,1},
{0,1,1,1,0,0,1,1,1,0,1,0,1,1,0,1,0,0,0,0,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1,1},
{1,1,1,1,0,1,1,0,1,0,1,0,0,0,1,0,1,0,1,1,0,1,0,1,1,0,0,0,1,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,0,0,1,1,0,1,0,0,1,1,1,1,1,1,1,0,1,0,0,1,1,0,1,0,1,1,1,0,0,0,1,1,0,1,0,1,0,0,0,0,0,1,1,0,1,0},
{0,0,1,1,1,0,1,1,0,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1,0,0,1,1,1,1,0,0,0,0,1,0,0,1,1,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,1,1,0,0,1},
{1,1,0,1,0,1,1,1,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,1,1,1,1,0,1,1,1,1,0,0,1,1,0,1,1,1,0,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,0,1,1,0,1,0,1,0,0,1,0,1,1,1,0,1,1,0,1,1,1,0,0,1,1,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,1,0},
{1,0,0,0,1,1,0,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,0,0,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,1,0,1,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0},
{1,1,0,1,1,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,1,0,1,1,0,1,0,0,1,0,0,0,1,1,1,1,1}
};


/* eventtype-2 */
//uint8_t event[10][120] = {
//{0,0,0,0,1,0,0,1,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,0,   0,0,1,0,0,1,0,1,1,0,0,0,1,0,0,1,0,0,0, 1,0,0,1,0,0,0,1,1,1,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
//{0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,1,0,1,1,0,0,1,0,0,0,0,0,   1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0, 0,1,0,1,0,1,0,0,0,0,0,   0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,0,1,0,0,1,1,1,0,1,1,0,0,0,1,1,0,0,1,0,0,1,   0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,0,0,0,   0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,0,1,1,1,0,0,1,0,1,0,1,1,1,1,1,1,1,0,0,0,1,0,0,   0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0, 0,0,1,0,0,0,0,0,1,0,0,   0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
//{0,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,0,   1,0,1,0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0, 1,0,0,0,0,1,0,0,0,0,0,   0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,   1,1,0,0,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0, 0,0,0,0,0,0,1,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,0,0,1,0,1,1,0,1,0,1,1,0,1,0,0,1,0,0,1,0,   1,1,0,1,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,1,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,1,0,   0,1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,   1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,1,1,0,1,1,0,0,1,0,0,   1,1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//{0,0,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,0,0,1,0,0,1,1,0,1,0,1,1,0,1,   1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
//};

/* eventtype-3 */ 
//uint8_t event[10][120] = {
//{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,1,1,0,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,0,1,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1},
//{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,1,1,0,0,1},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,1,0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,1},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,1,0,1,1,1,1,1,0,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1,0,0,0,1,0,0},
//{0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,0,0},
//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,1,0,1,0,1,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,1,0,0},
//{0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0},
//{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,0,0,0},
//{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,0,0,0,0},
//};

int n_event = 120;
int cnt = -1;


void setup() {
  // put your setup code here, to run once:
//  pinMode(buzzer,OUTPUT);
  pinMode(LED,OUTPUT);
  pinMode(button,INPUT);
  attachInterrupt(digitalPinToInterrupt(button), pin_ISR, RISING );

  Serial.begin(9600);
  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\nEvent Simulator");

}

void loop() {
  // put your main code here, to run repeatedly:
  int i,j;



  while(ButtonPressed){

    /* we prepare 10 distributions for testing */
    cnt ++;
    cnt = cnt % 10;
    
    Serial.println("cnt is :");
    Serial.println(cnt);
    
    
    /* blilnk LED very fast to indicate the event-play starts */
    count = 3; 
    while(count--){
      digitalWrite(LED,HIGH);
      delay(30);
      digitalWrite(LED,LOW);
      delay(30);
    }
    flag_1stPress = 1;  // in case that the first press of button triggers the ISR multiple times and thus makes ButtonPressed = 2 

    /* play the events */
    Serial.println("\n-----------");
    Serial.println("start the event-play loop");
    Serial.println("-----------\n");

    for(i = 0; i < n_event; i++){

      Time = millis();
      if(event[cnt][i] == 1){
          countdown = 975;
          while(countdown--){  // countdown 0.5s
            digitalWrite(buzzer,HIGH);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            digitalWrite(buzzer,LOW);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            }
        }
      else{
          delay(500);      
      }
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        
        break;
      }

      if(event[cnt][i] == 1){
        countdown = 975;  // countdown another 0.5s
          while(countdown--){
            digitalWrite(buzzer,HIGH);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
            digitalWrite(buzzer,LOW);
            delayMicroseconds(500000/buzzerFreq);  // 0.25ms
          }
      }
      else{
          delay(500);      
      }
      if( ButtonPressed == 2 ){
        ButtonPressed = 1;
        break;
      }
      Serial.println(millis()-Time);



      

      
    } // end of for loop
    
    digitalWrite(LED, LOW);
 

    /* after one iteration is finished without break, we stop in this loop for a while */
    if( i == n_event ){ 

      Serial.println("\n-----------");
      Serial.println("You entered the stop for a while loop ");
      Serial.println("-----------\n");
      
      while(1){
        
          delay(100);
          
          if( ButtonPressed == 2 ){

            Serial.println("\n-----------");
            Serial.println("We now break the stop for a while loop");
            Serial.println("-----------\n");
            
            ButtonPressed = 1;
            break;
           }
           
       }
    }
    

    
  }// end of while loop
}


void pin_ISR(){

  /* for debug */
  Serial.println("\n-----------");
  Serial.println("Button is pressed and the original button status is ");
  Serial.println(ButtonPressed);

  
  if(ButtonPressed == 0)
    ButtonPressed = 1;
    
  if(ButtonPressed == 1 && flag_1stPress == 1)
    ButtonPressed = 2;

  /* for debug */
  Serial.println("the new button status is ");
  Serial.println(ButtonPressed);
  Serial.println("-----------\n");
}
