//soglie velocità
static const int velocita_soglia_min=5;  //sotto questa velotià invio dati raro
static const int velocita_soglia_max=40; //sotto questa velotià invio dati medio - sopra questa velocità invio lento

//frequenze invio
static unsigned long FREQMIN = 7200000; //freq invio sotto soglia min
static unsigned long FREQMED = 20000; //freq invio sopra soglia max
static unsigned long FREQMAX = 5000; //freq invio tra soglia min e soglia max

static const int minDegToSendDataToServer = 30; //se la direzione cambia di oltre 30 gradi allora viene forzato l'invio dei dati al server

#define TRACCAR_HOST "40.112.128.183"
#define TRACCAR_OSMAND_PORT 5055

#define TRACCAR_GPS103_PORT 5001
#define TRACCAR_GPS103_CELLPHONENUMBER "33312345678"
//#define TRACCAR_DEV_ID "tonz"  //comment for automatic ID
